﻿#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>

#include "glew.h"
#include "glm.hpp"
#include "ext.hpp"
#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Texture.h"
#include "Box.cpp"
#include "SOIL/SOIL.h"
#include "Models.hpp"

//window variables
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
int WIDTH = 800, HEIGHT = 800;

//variables -------------------------------------------------------------------------------------------------------------------------------------------------------- variables

//depht
GLuint depthMapFBO;
GLuint depthMap;

//shaders
GLuint programPBR;
GLuint programTex;
GLuint programSun;
GLuint programSkybox;
Core::Shader_Loader shaderLoader;

//sun
glm::vec3 sunDir = glm::vec3(-0.93f, 0.35f, 0.00f);
glm::vec3 sunColor = glm::vec3(0.9f, 0.9f, 0.7f);
float sunForce = 5;

//camera
glm::vec3 cameraPos = glm::vec3(0.48f, 1.25f, -2.12f);
glm::vec3 cameraDir = glm::vec3(-0.35f, 0.00f, 0.93f);

//player
glm::vec3 playerPos = glm::vec3(0.0f, 1.25f, -7.0f);
glm::vec3 playerDir = glm::vec3(-0.0f, 0.00f, 1.0f);

//apsect and exposition
float aspectRatio = 1.f;
float exposition = 1.f;

//pointlight (sun)
glm::vec3 basePointlightPos = glm::vec3(0, 100.0f, 0);
glm::vec3 pointlightPos = basePointlightPos;
glm::vec3 pointlightColor = glm::vec3(0.9, 0.6, 0.6);
float sunPositionTranslateModifier = -85.0f;

//spotlight (player)
glm::vec3 spotlightPos = glm::vec3(0, 0, 0);
glm::vec3 spotlightConeDir = glm::vec3(0, 0, 0);
glm::vec3 spotlightColor = glm::vec3(0.4, 0.4, 0.9)*3;
float spotlightPhi = 3.14 / 4;

//player animation
int animationState = 0;
bool animationStateRising = true;


//delta time ------------------------------------------------------------------------------------------------------------------------------------------------------- delta time
float lastTime = -1.f;
float deltaTime = 0.f;
void updateDeltaTime(float time) {
	if (lastTime < 0) {
		lastTime = time;
		return;
	}

	deltaTime = time - lastTime;
	if (deltaTime > 0.1) deltaTime = 0.1;
	lastTime = time;
}


//camera and perspective matrix ------------------------------------------------------------------------------------------------------------------ camera and perspective matrix
glm::mat4 createCameraMatrix()
{
	glm::vec3 cameraSide = glm::normalize(glm::cross(cameraDir,glm::vec3(0.f,1.f,0.f)));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraSide,cameraDir));
	glm::mat4 cameraRotrationMatrix = glm::mat4({
		cameraSide.x,cameraSide.y,cameraSide.z,0,
		cameraUp.x,cameraUp.y,cameraUp.z ,0,
		-cameraDir.x,-cameraDir.y,-cameraDir.z,0,
		0.,0.,0.,1.,
		});
	cameraRotrationMatrix = glm::transpose(cameraRotrationMatrix);
	glm::mat4 cameraMatrix = cameraRotrationMatrix * glm::translate(-cameraPos);

	return cameraMatrix;
}

glm::mat4 createPerspectiveMatrix()
{
	
	glm::mat4 perspectiveMatrix;
	float n = 0.05;
	float f = 20.;
	float a1 = glm::min(aspectRatio, 1.f);
	float a2 = glm::min(1 / aspectRatio, 1.f);
	perspectiveMatrix = glm::mat4({
		1,0.,0.,0.,
		0.,aspectRatio,0.,0.,
		0.,0.,(f+n) / (n - f),2*f * n / (n - f),
		0.,0.,-1.,0.,
		});

	
	perspectiveMatrix=glm::transpose(perspectiveMatrix);

	return perspectiveMatrix;
}



//drawPBR ----------------------------------------------------------------------------------------------------------------------------------------------------------- drawPBR
void drawObjectPBR(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color, GLuint textureId, float roughness, float metallic, float brightness) 
{
	GLuint program;
	if (textureId == NULL)
	{
		program = programPBR;
	}
	else
	{
		program = programTex;
	}

	glUseProgram(program);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glUniform1f(glGetUniformLocation(program, "exposition"), exposition);

	glUniform1f(glGetUniformLocation(program, "roughness"), roughness);
	glUniform1f(glGetUniformLocation(program, "metallic"), metallic);
	glUniform1f(glGetUniformLocation(program, "brightness"), brightness);

	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glUniform3f(glGetUniformLocation(program, "sunDir"), sunDir.x, sunDir.y, sunDir.z);
	glUniform3f(glGetUniformLocation(program, "sunColor"), sunColor.x * sunForce/100, sunColor.y * sunForce/100, sunColor.z * sunForce/100);

	glUniform3f(glGetUniformLocation(program, "lightPos"), pointlightPos.x, pointlightPos.y, pointlightPos.z);
	glUniform3f(glGetUniformLocation(program, "lightColor"), pointlightColor.x, pointlightColor.y, pointlightColor.z);

	glUniform3f(glGetUniformLocation(program, "spotlightConeDir"), spotlightConeDir.x, spotlightConeDir.y, spotlightConeDir.z);
	glUniform3f(glGetUniformLocation(program, "spotlightPos"), spotlightPos.x, spotlightPos.y, spotlightPos.z);
	glUniform3f(glGetUniformLocation(program, "spotlightColor"), spotlightColor.x, spotlightColor.y, spotlightColor.z);
	glUniform1f(glGetUniformLocation(program, "spotlightPhi"), spotlightPhi);

	if (textureId == NULL)
	{
		glUniform3f(glGetUniformLocation(program, "color"), color.x, color.y, color.z);
	}
	else
	{
		Core::SetActiveTexture(textureId, "colorTexture", programTex, 0);
	}
	
	Core::DrawContext(context);
}

//player animation ------------------------------------------------------------------------------------------------------------------------------------------- player animation
void animatePlayer()
{
	glm::vec3 playerSide = glm::normalize(glm::cross(playerDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 playerUp = glm::normalize(glm::cross(playerSide, playerDir));
	glm::mat4 playerCameraRotrationMatrix = glm::mat4({
		playerSide.x,playerSide.y,playerSide.z,0,
		playerUp.x,playerUp.y,playerUp.z ,0,
		-playerDir.x,-playerDir.y,-playerDir.z,0,
		0.,0.,0.,1.,
		});


	if (animationStateRising)
	{
		if (animationState < 5)
		{
			animationState++;
		}
		else
		{
			animationState--;
			animationStateRising = false;
		}
	}
	else
	{
		if (animationState > 0)
		{
			animationState--;
		}
		else
		{
			animationState++;
			animationStateRising = true;
		}
	}

	Core::RenderContext player;

	switch (animationState)
	{
	case 0:
		player = models::fly0; break;
	case 1:
		player = models::fly1; break;
	case 2:
		player = models::fly2; break;
	case 3:
		player = models::fly3; break;
	case 4:
		player = models::fly4; break;
	case 5:
		player = models::fly5; break;
	}

	drawObjectPBR(player,
		glm::translate(playerPos) * playerCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.1f)),
		glm::vec3(3, 3, 3), NULL,
		0.2f, 1.0, 1.0f);
	
}


//render shadow ----------------------------------------------------------------------------------------------------------------------------------------------- render shadow
void renderShadowapSun()
{
	float time = glfwGetTime();
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	//uzupelnij o renderowanie glebokosci do tekstury
	// 
	//		/\
	//		||
	// 
	//coś z ostatnich zajęć, przy odrobinie szczęścia ktoś to ogarnie zanim zauważy to prowadzący xd

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WIDTH, HEIGHT);
}


//render scene objects ----------------------------------------------------------------------------------------------------------------------------------- render scene objects
void renderSun(float rotation)
{
	glUseProgram(programSun);

	glm::vec3 pointlightPos = glm::vec3(glm::eulerAngleZ((rotation * 3.142)/180) * glm::vec4(basePointlightPos, 0));
	sunDir = pointlightPos;

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * glm::translate(pointlightPos + glm::vec3(0.0f,sunPositionTranslateModifier,0.0f)) * glm::scale(glm::vec3(1.0f));
	glUniformMatrix4fv(glGetUniformLocation(programSun, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniform3f(glGetUniformLocation(programSun, "color"), sunColor.x * 2.5f, sunColor.y * 2.5f, sunColor.z * 2.5f);
	glUniform1f(glGetUniformLocation(programSun, "exposition"), exposition);
	Core::DrawContext(models::sphere);
}

void renderSkybox(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID)
{
	glDepthFunc(GL_LEQUAL);
	glUseProgram(programSkybox);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programSkybox, "transformation"), 1, GL_FALSE, (float*)&transformation);
	Core::SetActiveTexture(textureID, "skybox", programSkybox, 0);
	Core::DrawContext(context);
	glDepthFunc(GL_LESS);
}

//render scene --------------------------------------------------------------------------------- render scene
void renderScene(GLFWwindow* window)
{
	//skybox
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderSkybox(models::skybox, glm::translate(glm::scale(glm::mat4(), glm::vec3(4.0f)), glm::vec3(0.0f,2.0f,0.0f)), textures::skybox);

	//time and delta time
	float time = glfwGetTime();
	updateDeltaTime(time);

	//shadow
	renderShadowapSun();

	//sun
	renderSun(13);
	
	
	//render structures
	drawObjectPBR(models::ceiling, glm::translate(glm::mat4(), glm::vec3(0.0f, -0.01f, 0.0f)), glm::vec3(10.0f, 10.0f, 10.0f), NULL, 0.8f, 0.0f, 1.0f);
	drawObjectPBR(models::roof, glm::mat4(), glm::vec3(), textures::roof, 0.0f, 1.0f, 20.0f);
	drawObjectPBR(models::floor, glm::mat4(), glm::vec3(5.0f, 0.0f, 0.0f), NULL, 1.0f, 1.0f, 1.0f);
	drawObjectPBR(models::room, glm::mat4(), glm::vec3(10.0f, 0.1f, 3.0f), NULL, 0.8f, 0.0f, 1.0f);
	drawObjectPBR(models::ground, glm::mat4(), glm::vec3(), textures::ground, 1.0f, 1.0f, 30.0f);

	//render furnitures
	drawObjectPBR(models::bed, glm::mat4(), glm::vec3(0.03f, 0.03f, 0.03f), NULL, 0.2f, 0.0f, 1.0f);
	drawObjectPBR(models::chair, glm::mat4(), glm::vec3(0.2f, 0.4f, 0.8f), NULL, 0.4f, 0.0f, 1.0f);
	drawObjectPBR(models::desk, glm::mat4(), glm::vec3(0.4f, 0.1f, 0.0f), NULL, 0.2f, 0.0f, 1.0f);
	drawObjectPBR(models::door, glm::mat4(), glm::vec3(0.4f, 0.1f, 0.05f), NULL, 0.2f, 0.0f, 1.0f);
	drawObjectPBR(models::drawer, glm::mat4(), glm::vec3(0.4f, 0.08f, 0.03f), NULL, 0.2f, 0.0f, 1.0f);
	drawObjectPBR(models::marbleBust, glm::mat4(), glm::vec3(1.0f, 1.0f, 1.0f), NULL, 0.5f, 1.0f, 1.0f);
	drawObjectPBR(models::materace, glm::mat4(), glm::vec3(0.9f, 0.9f, 0.9f), NULL, 0.8f, 0.0f, 1.0f);
	drawObjectPBR(models::pencils, glm::mat4(), glm::vec3(0.1f, 0.02f, 0.0f), NULL, 0.1f, 0.0f, 1.0f);
	drawObjectPBR(models::hugeWindow, glm::mat4(), glm::vec3(0.4f, 0.1f, 0.05f), NULL, 0.2f, 0.0f, 1.0f);
	drawObjectPBR(models::smallWindow1, glm::mat4(), glm::vec3(0.4f, 0.1f, 0.05f), NULL, 0.2f, 0.0f, 1.0f);
	drawObjectPBR(models::smallWindow2, glm::mat4(), glm::vec3(0.4f, 0.1f, 0.05f), NULL, 0.2f, 0.0f, 1.0f);
	drawObjectPBR(models::painting, glm::mat4(), glm::vec3(), textures::painting, 0.0f, 0.0f, 3.0f);

	//render and animate player
	animatePlayer();

	//update player light cone
	spotlightPos = playerPos + 0.2 * playerDir;
	spotlightConeDir = playerDir;

	glfwSwapBuffers(window);
}



// initialization ---------------------------------------------------------------------------------------------------------------------------------------------- initialization
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	aspectRatio = width / float(height);
	glViewport(0, 0, width, height);
	WIDTH = width;
	HEIGHT = height;
}


// init ------------------------------------------------------------------------------------------------------- init
void init(GLFWwindow* window)
{
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);
	programPBR = shaderLoader.CreateProgram("shaders/shader_pbr.vert", "shaders/shader_pbr.frag");
	programTex = shaderLoader.CreateProgram("shaders/shader_pbr_tex.vert", "shaders/shader_pbr_tex.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_sun.vert", "shaders/shader_sun.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");
	
	loadAllModels();
}


//shutdown ---------------------------------------------------------------------------------------------------------------------------------------------------------- shutdown
void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(programPBR);
	shaderLoader.DeleteProgram(programTex);
	shaderLoader.DeleteProgram(programSun);
	shaderLoader.DeleteProgram(programSkybox);
}



//input processing ------------------------------------------------------------------------------------------------------------------------------------------- input processing
void processInput(GLFWwindow* window)
{
	glm::vec3 spaceshipSide = glm::normalize(glm::cross(playerDir, glm::vec3(0.f,1.f,0.f)));
	glm::vec3 spaceshipUp = glm::vec3(0.f, 1.f, 0.f);
	float angleSpeed = 0.05f * deltaTime * 60;
	float moveSpeed = 0.05f * deltaTime * 60;


	//exit
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}


	//motion
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		playerPos += playerDir * moveSpeed;
	}	
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		playerPos -= playerDir * moveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		playerPos += spaceshipSide * moveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		playerPos -= spaceshipSide * moveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		playerPos += spaceshipUp * moveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		playerPos -= spaceshipUp * moveSpeed;
	}
	

	//rotation
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		playerDir = glm::vec3(glm::eulerAngleY(angleSpeed) * glm::vec4(playerDir, 0));
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		playerDir = glm::vec3(glm::eulerAngleY(-angleSpeed) * glm::vec4(playerDir, 0));
	}
		

	//update camera
	cameraPos = playerPos - 0.5 * playerDir + glm::vec3(0, 2, 0) * 0.2f;
	cameraDir = playerDir;


	//exposition
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		exposition -= 0.05;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
		exposition += 0.05;
	}
		

	// debug info
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		printf("spaceshipPos = glm::vec3(%ff, %ff, %ff);\n", playerPos.x, playerPos.y, playerPos.z);
		printf("spaceshipDir = glm::vec3(%ff, %ff, %ff);\n", playerDir.x, playerDir.y, playerDir.z);
	}
}



//main loop -------------------------------------------------------------------------------------------------------------------------------------------------------- main loop
void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		renderScene(window);
		glfwPollEvents();
	}
}
