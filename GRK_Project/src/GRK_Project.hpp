#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <Windows.h>

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
int WIDTH = 1000, HEIGHT = 1000;

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
float sunx = 9.0f, suny = 4.0f, sunz = 2.0f;
glm::vec3 sunDir = glm::vec3(sunx, suny, sunz);
glm::vec3 sunPos = glm::vec3(5.0f, 20.0f, 8.0f);
glm::vec3 sunColor = glm::vec3(0.9f, 0.9f, 0.7f);
float sunForce = 5;

//player
glm::vec3 playerPos = glm::vec3(0.0f, 1.25f, -10.0f);
glm::vec3 playerDir = glm::vec3(-0.0f, 0.00f, 1.0f);

//camera
glm::vec3 cameraStartPos = playerPos - 0.5 * playerDir + glm::vec3(0, 2, 0) * 0.2f;;
glm::vec3 cameraPos = glm::vec3(cameraStartPos.x, cameraStartPos.y, cameraStartPos.z);
glm::vec3 cameraDir = playerDir;

//apsect and exposition
float aspectRatio = 1.f;
float exposition = 1.f;

//table lamp light
glm::vec3 pointlightPos = glm::vec3(0.0f, 2.0f, 0.0f);
glm::vec3 pointlightColorON = glm::vec3(0.9, 0.6, 0.6) * 4;
glm::vec3 pointlightColor = pointlightColorON*0;

//main lamp light
glm::vec3 spotlightPos = glm::vec3(-3.21f, 1.3941f, 1.6343f);
glm::vec3 spotlightConeDir = glm::vec3(0.35f, -0.7f, -0.95f);
glm::vec3 spotlightColorON = glm::vec3(0.4, 0.4, 0.7)*3;
glm::vec3 spotlightColor = spotlightColorON;
float spotlightPhi = 3.14 / 3;

//player animation
int animationState = 0;
bool animationStateRising = true;

//door animation
float doorRotation=0.0f;
bool animationStarted = false;

//switch listener
int switchDelay = 50;
float mainLampRotation = 0;


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

glm::mat4 rotateAroundPivot(float degrees, glm::vec3 axis, glm::vec3 pivot)
{
	glm::mat4 to_pivot = glm::translate(glm::mat4(), -pivot);
	glm::mat4 from_pivot = glm::translate(glm::mat4(), pivot);
	glm::mat4 rotate = to_pivot * glm::rotate(glm::mat4(), glm::radians(degrees), axis) * from_pivot;
	return rotate;
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
	float a1 = min(aspectRatio, 1.f);
	float a2 = min(1 / aspectRatio, 1.f);
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
	glUniform3f(glGetUniformLocation(program, "cameraStartPos"), cameraStartPos.x, cameraStartPos.y, cameraStartPos.z);

	glUniform3f(glGetUniformLocation(program, "sunDir"),  sunDir.x, sunDir.y, sunDir.z);
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

//animations ----------------------------------------------------------------------------------------------------------------------------------------------------- animations
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
	GLuint texture;

	switch (animationState)
	{
	case 0:
		player = models::fly0; texture = textures::fly0; break;
	case 1:
		player = models::fly1; texture = textures::fly1; break;
	case 2:
		player = models::fly2; texture = textures::fly2; break;
	case 3:
		player = models::fly3; texture = textures::fly3; break;
	case 4:
		player = models::fly4; texture = textures::fly4; break;
	case 5:
		player = models::fly5; texture = textures::fly5; break;
	}

	drawObjectPBR(player,
		glm::translate(playerPos) * playerCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.1f)),
		glm::vec3(), texture,
		0.2f, 1.0, 10.0f);
	
}

void animateDoor()
{
	if (doorRotation > -130 && animationStarted)
	{
		doorRotation--;
	}
	glm::vec3 pivot = glm::vec3(0.411f, 0.957f, 4.628f);
	glm::mat4 rotate = rotateAroundPivot(doorRotation, glm::vec3(0, 1, 0), pivot);
	drawObjectPBR(models::door, glm::mat4()*rotate, glm::vec3(), textures::door, 0.9f, 0.0f, 3.0f);
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
void renderSun()
{
	glUseProgram(programSun);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * glm::translate(sunPos + glm::vec3(0.0f,0.0f,0.0f)) * glm::scale(glm::vec3(1.0f));
	glUniformMatrix4fv(glGetUniformLocation(programSun, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniform3f(glGetUniformLocation(programSun, "color"), sunColor.x * 7.5f, sunColor.y * 7.5f, sunColor.z * 7.5f);
	glUniform1f(glGetUniformLocation(programSun, "exposition"), exposition);

	sunDir = glm::vec3(sunx, suny, sunz);

	Core::DrawContext(models::sphere);
}

void renderSkybox(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID)
{
	glDepthFunc(GL_LEQUAL);
	glUseProgram(programSkybox);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * glm::mat4(glm::mat3(createCameraMatrix()));
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programSkybox, "transformation"), 1, GL_FALSE, (float*)&transformation);
	Core::SetActiveTexture(textureID, "skybox", programSkybox, 0);
	Core::DrawContext(context);
	glDepthFunc(GL_LESS);
	glUseProgram(0);
}

//render scene --------------------------------------------------------------------------------- render scene
void renderScene(GLFWwindow* window)
{
	//skybox
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderSkybox(models::skybox, glm::translate(glm::scale(glm::mat4(), glm::vec3(2.5f, 2.0f, 2.0f)), glm::vec3(3.0f, 1.0f, 0.0f)), textures::skybox);


	//time and delta time
	float time = glfwGetTime();
	updateDeltaTime(time);

	//shadow
	renderShadowapSun();

	//sun
	renderSun();
	
	
	//render structures
	drawObjectPBR(models::ceiling, glm::mat4(), glm::vec3(), textures::ceiling, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::roof, glm::mat4(), glm::vec3(), textures::roof, 0.8f, 0.0f, 20.0f);
	drawObjectPBR(models::floor, glm::mat4(), glm::vec3(), textures::floor, 0.8f, 0.0f, 15.0f);
	drawObjectPBR(models::room, glm::mat4(), glm::vec3(), textures::room, 0.8f, 0.0f, 7.0f);
	drawObjectPBR(models::ground, glm::mat4(), glm::vec3(), textures::ground, 0.8f, 0.0f, 30.0f);
	drawObjectPBR(models::walls, glm::mat4(), glm::vec3(), textures::walls, 0.8f, 0.0f, 5.0f);

	//render furnitures
	drawObjectPBR(models::bed, glm::mat4(), glm::vec3(), textures::bed, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::chair, glm::mat4(), glm::vec3(), textures::chair, 0.4f, 0.0f, 5.0f);
	drawObjectPBR(models::desk, glm::mat4(), glm::vec3(), textures::desk, 0.2f, 0.0f, 5.0f);
	animateDoor();
	drawObjectPBR(models::jamb, glm::mat4(), glm::vec3(), textures::jamb, 0.2f, 0.0f, 3.0f);
	drawObjectPBR(models::drawer, glm::mat4(), glm::vec3(), textures::drawer, 0.2f, 0.0f, 5.0f);
	drawObjectPBR(models::marbleBust, glm::mat4(), glm::vec3(), textures::marbleBust, 0.9f, 1.0f, 10.0f);
	drawObjectPBR(models::mattress, glm::mat4(), glm::vec3(), textures::mattress, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::pencils, glm::mat4(), glm::vec3(0.1f, 0.02f, 0.0f), textures::pencils, 0.0f, 0.0f, 3.0f);
	drawObjectPBR(models::hugeWindow, glm::mat4(), glm::vec3(5.0f, 5.0f, 5.0f), NULL, 0.2f, 0.0f, 1.0f);
	drawObjectPBR(models::smallWindow1, glm::mat4(), glm::vec3(5.0f, 5.0f, 5.0f), NULL, 0.2f, 0.0f, 1.0f);
	drawObjectPBR(models::smallWindow2, glm::mat4(), glm::vec3(5.0f, 5.0f, 5.0f), NULL, 0.2f, 0.0f, 1.0f);
	drawObjectPBR(models::painting, glm::mat4(), glm::vec3(), textures::painting, 0.0f, 0.0f, 3.0f);
	drawObjectPBR(models::carpet, glm::mat4(), glm::vec3(), textures::carpet, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::stool, glm::mat4(), glm::vec3(), textures::stool, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::stool, glm::translate(glm::mat4(), glm::vec3(-0.5f, 0, -0.5f)), glm::vec3(), textures::stool, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::barbells, glm::mat4(), glm::vec3(), textures::barbells, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::mat, glm::mat4(), glm::vec3(), textures::mat, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::poster, glm::mat4(), glm::vec3(), textures::poster, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::tableLamp, glm::mat4(), glm::vec3(), textures::tableLamp, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::lamp, glm::mat4(), glm::vec3(), textures::lamp, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::switch_, glm::mat4() * rotateAroundPivot(mainLampRotation, glm::vec3(0,0,1), glm::vec3(-0.8f, -1.25f, -4.6f)), glm::vec3(0.95f, 0.95f, 0.85f), NULL, 0.8f, 0.0f, 5.0f);

	//render environment
	drawObjectPBR(models::tree, glm::translate(glm::mat4(), glm::vec3(5.3f,0.0f,7.0f)), glm::vec3(), textures::tree, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::tree, glm::translate(glm::mat4(), glm::vec3(7.5f, -0.3f, 4.0f)), glm::vec3(), textures::tree, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::tree, glm::translate(glm::mat4(), glm::vec3(10.0f, -0.5f, 1.0f)), glm::vec3(), textures::tree, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::tree, glm::translate(glm::mat4(), glm::vec3(7.0f, -0.3f, -3.0f)), glm::vec3(), textures::tree, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::tree, glm::translate(glm::mat4(), glm::vec3(5.3f, 0.0f, -8.0f)), glm::vec3(), textures::tree, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::tree, glm::translate(glm::mat4(), glm::vec3(0.3f, 0.0f, 8.4f)), glm::vec3(), textures::tree, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::tree, glm::translate(glm::mat4(), glm::vec3(1.3f, 0.0f, 13.4f)), glm::vec3(), textures::tree, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::tree, glm::translate(glm::mat4(), glm::vec3(10.6f, -0.5f, 14.0f)), glm::vec3(), textures::tree, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::tree, glm::translate(glm::mat4(), glm::vec3(15.1f, -0.7f, 8.0f)), glm::vec3(), textures::tree, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::tree, glm::translate(glm::mat4(), glm::vec3(20.8f, -1.0f, 2.0f)), glm::vec3(), textures::tree, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::tree, glm::translate(glm::mat4(), glm::vec3(16.3f, -0.7f, -7.0f)), glm::vec3(), textures::tree, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::tree, glm::translate(glm::mat4(), glm::vec3(8.3f, -0.5f, -12.7f)), glm::vec3(), textures::tree, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::bush, glm::translate(glm::mat4(), glm::vec3(8.3f, 0.0f, -10.7f)), glm::vec3(), textures::bush, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::bush, glm::translate(glm::mat4(), glm::vec3(9.3f, 0.0f, -5.7f)), glm::vec3(), textures::bush, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::bush, glm::translate(glm::mat4(), glm::vec3(3.3f, 0.0f, -6.0f)), glm::vec3(), textures::bush, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::bush, glm::translate(glm::mat4(), glm::vec3(5.3f, 0.0f, 0.0f)), glm::vec3(), textures::bush, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::bush, glm::translate(glm::mat4(), glm::vec3(11.3f, 0.0f, 2.7f)), glm::vec3(), textures::bush, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::rock, glm::translate(glm::mat4(), glm::vec3(11.3f, 0.0f, -3.0f)), glm::vec3(), textures::rock, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::rock, glm::translate(glm::mat4(), glm::vec3(11.3f, 0.0f, -8.7f)), glm::vec3(), textures::rock, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::rock, glm::translate(glm::mat4(), glm::vec3(7.5f, 0.0f, -13.0f)), glm::vec3(), textures::rock, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::rock, glm::translate(glm::mat4(), glm::vec3(7.0f, 0.0f, -0.5f)), glm::vec3(), textures::rock, 0.0f, 0.0f, 5.0f);
	drawObjectPBR(models::rock, glm::translate(glm::mat4(), glm::vec3(4.5f, 0.0f, -7.0f)), glm::vec3(), textures::rock, 0.0f, 0.0f, 5.0f);

	//render and animate player
	animatePlayer();


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

	PlaySound(TEXT("./sounds/backgroundSound.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
	
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
		animationStarted = true;
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

	//sunDir
	//x
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
		sunx += 0.01;
	}
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
		sunx -= 0.01;
	}
	//y
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
		suny += 0.01;
	}
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
		suny -= 0.01;
	}
	//z
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
		sunz += 0.01;
	}
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
		sunz -= 0.01;
	}
		

	// debug info
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		printf("spaceshipPos = glm::vec3(%ff, %ff, %ff);\n", playerPos.x, playerPos.y, playerPos.z);
		printf("spaceshipDir = glm::vec3(%ff, %ff, %ff);\n", playerDir.x, playerDir.y, playerDir.z);
		printf("sunDir = glm::vec3(%ff, %ff, %ff);\n\n", spotlightPos.x, spotlightPos.y, spotlightPos.z);
		printf("sunDir = glm::vec3(%ff, %ff, %ff);\n\n", spotlightConeDir.x, spotlightConeDir.y, spotlightConeDir.z);
	}
}

//constrain movement  ------------------------------------------------------------------------------------------------------------------------------------ constrain movement
void constrainMovement()
{
	//x pos
	if (playerPos.x > 15.0f)
	{
		playerPos.x = 15.0f;
	}
	if (playerPos.x < -15.0f)
	{
		playerPos.x = -15.0f;
	}

	//y pos
	if (playerPos.y > 12.0f)
	{
		playerPos.y = 12.0f;
	}
	if (playerPos.y < 0.0f)
	{
		playerPos.y = 0.0f;
	}

	//z pos
	if (playerPos.z > 15.0f)
	{
		playerPos.z = 15.0f;
	}
	if (playerPos.z < -15.0f)
	{
		playerPos.z = -15.0f;
	}

}

void switchListener()
{
	if (switchDelay < 50)
	{
		switchDelay++;
	}


	if (playerPos.y > 1.10f && playerPos.y < 1.40f)
	{
		if (playerPos.x > 0.7f && playerPos.x < 0.9f)
		{
			if (playerPos.z > -4.6f && playerPos.z < -4.4f)
			{
				if (switchDelay == 50)
				{
					switchDelay = 0;
					if (mainLampRotation == 0)
					{
						mainLampRotation = 180;
						pointlightColor = pointlightColorON;
					}
					else
					{
						mainLampRotation = 0;
						pointlightColor *= 0;
					}
				}
			}			
		}
	}


	if (playerPos.y > 1.03f && playerPos.y < 1.18f)
	{
		if (playerPos.x > -3.35f && playerPos.x < -3.15f)
		{
			if (playerPos.z > 1.65f && playerPos.z < 1.85f)
			{
				if (switchDelay == 50)
				{
					switchDelay = 0;
					if (spotlightColor == spotlightColorON)
					{
						spotlightColor *= 0;
					}
					else
					{
						spotlightColor = spotlightColorON;
					}
				}
			}			
		}
	}
	
}

//fps limiter --------------------------------------------------------------------------------------------------------------------------------------------------fps limiter
void setMaxFPS(float fps)
{
	if (1 / deltaTime > fps)
	{
		float timeToDelay = 1 - (deltaTime * fps);
		Sleep((timeToDelay * 1000)/fps);
	}
}


//main loop -------------------------------------------------------------------------------------------------------------------------------------------------------- main loop
void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		constrainMovement();

		switchListener();

		renderScene(window);
		glfwPollEvents();

		setMaxFPS(75);
	}
}
