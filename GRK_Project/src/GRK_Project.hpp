#include <GLFW/glfw3.h>
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

//window variables
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
int WIDTH = 800, HEIGHT = 800;

//models ------------------------------------------------------------------------------------------------------------------------------------------------------------- models
namespace models 
{
	// structures
	Core::RenderContext roomContext;
	Core::RenderContext roofContext;
	Core::RenderContext ceilingContext;
	Core::RenderContext floorContext;
	Core::RenderContext sphereContext;
	Core::RenderContext groundContext;
	Core::RenderContext skyboxContext;

	//furnitures
	Core::RenderContext hugeWindowContext;
	Core::RenderContext smallWindow1Context;
	Core::RenderContext smallWindow2Context;
	Core::RenderContext bedContext;
	Core::RenderContext chairContext;
	Core::RenderContext deskContext;
	Core::RenderContext doorContext;
	Core::RenderContext drawerContext;
	Core::RenderContext marbleBustContext;
	Core::RenderContext materaceContext;
	Core::RenderContext pencilsContext;
	Core::RenderContext painting;


	//player
	Core::RenderContext fly0Context;
	Core::RenderContext fly1Context;
	Core::RenderContext fly2Context;
	Core::RenderContext fly3Context;
	Core::RenderContext fly4Context;
	Core::RenderContext fly5Context;

}

namespace textures 
{
	//structures
	GLuint skybox;
	GLuint ground;

	//furnitures


	//player
}

//variables -------------------------------------------------------------------------------------------------------------------------------------------------------- variables
GLuint depthMapFBO;
GLuint depthMap;

GLuint program;
GLuint programSun;
GLuint programTest;
GLuint programTex;
GLuint programSkybox;

Core::Shader_Loader shaderLoader;


//sun
glm::vec3 sunDir = glm::vec3(-0.93633f, 0.351106, 0.003226f);
glm::vec3 sunColor = glm::vec3(0.9f, 0.9f, 0.7f);
float sunForce = 5;

//camera
glm::vec3 cameraPos = glm::vec3(0.479490f, 1.250000f, -2.124680f);
glm::vec3 cameraDir = glm::vec3(-0.354510f, 0.000000f, 0.935054f);

//player
glm::vec3 playerPos = glm::vec3(0, 1.250000f, 0);
glm::vec3 playerDir = glm::vec3(-0.0f, 0.000000f, 1.0f);

//apsect and exposition
float aspectRatio = 1.f;
float exposition = 1.f;

//pointlight (sun)
glm::vec3 pointlightPos = glm::vec3(0, 100.0f, 0);
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
void drawObjectPBR(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color, float roughness, float metallic) 
{
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glUniform1f(glGetUniformLocation(program, "exposition"), exposition);

	glUniform1f(glGetUniformLocation(program, "roughness"), roughness);
	glUniform1f(glGetUniformLocation(program, "metallic"), metallic);

	glUniform3f(glGetUniformLocation(program, "color"), color.x, color.y, color.z);

	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glUniform3f(glGetUniformLocation(program, "sunDir"), sunDir.x, sunDir.y, sunDir.z);
	glUniform3f(glGetUniformLocation(program, "sunColor"), sunColor.x * sunForce/100, sunColor.y * sunForce/100, sunColor.z * sunForce/100);

	glUniform3f(glGetUniformLocation(program, "lightPos"), pointlightPos.x, pointlightPos.y, pointlightPos.z);
	glUniform3f(glGetUniformLocation(program, "lightColor"), pointlightColor.x, pointlightColor.y, pointlightColor.z);

	glUniform3f(glGetUniformLocation(program, "spotlightConeDir"), spotlightConeDir.x, spotlightConeDir.y, spotlightConeDir.z);
	glUniform3f(glGetUniformLocation(program, "spotlightPos"), spotlightPos.x, spotlightPos.y, spotlightPos.z);
	glUniform3f(glGetUniformLocation(program, "spotlightColor"), spotlightColor.x, spotlightColor.y, spotlightColor.z);
	glUniform1f(glGetUniformLocation(program, "spotlightPhi"), spotlightPhi);
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

	if (animationState == 0)
	{
		drawObjectPBR(models::fly0Context,
			glm::translate(playerPos) * playerCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.1f)),
			glm::vec3(3, 3, 3),
			0.2, 1.0
		);
	}
	if (animationState == 1)
	{
		drawObjectPBR(models::fly1Context,
			glm::translate(playerPos) * playerCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.1f)),
			glm::vec3(3, 3, 3),
			0.2, 1.0
		);
	}
	if (animationState == 2)
	{
		drawObjectPBR(models::fly2Context,
			glm::translate(playerPos) * playerCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.1f)),
			glm::vec3(3, 3, 3),
			0.2, 1.0
		);
	}
	if (animationState == 3)
	{
		drawObjectPBR(models::fly3Context,
			glm::translate(playerPos) * playerCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.1f)),
			glm::vec3(3, 3, 3),
			0.2, 1.0
		);
	}
	if (animationState == 4)
	{
		drawObjectPBR(models::fly4Context,
			glm::translate(playerPos) * playerCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.1f)),
			glm::vec3(3, 3, 3),
			0.2, 1.0
		);
	}
	if (animationState == 5)
	{
		drawObjectPBR(models::fly5Context,
			glm::translate(playerPos) * playerCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.1f)),
			glm::vec3(3, 3, 3),
			0.2, 1.0
		);
	}
	
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

	float sunSpeedFast = 0.1f;
	float sunSpeedSlow = 0.001f;
	float sunSpeed;

	

	if (pointlightPos.y + sunPositionTranslateModifier < 4.0f)
	{
		sunSpeed = sunSpeedFast;
	}
	else
	{
		sunSpeed = sunSpeedSlow;
	}

	if (pointlightPos.y + sunPositionTranslateModifier <= 1.5f)
	{
		sunForce = 0.5f;
	}
	else
	{
		sunForce = (pointlightPos.y + sunPositionTranslateModifier) / 3;
	}

	pointlightPos = glm::vec3(glm::eulerAngleZ(sunSpeed) * glm::vec4(pointlightPos, 0));
	sunDir = pointlightPos;

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * glm::translate(pointlightPos + glm::vec3(0.0f,sunPositionTranslateModifier,0.0f)) * glm::scale(glm::vec3(1.0f));
	glUniformMatrix4fv(glGetUniformLocation(programSun, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniform3f(glGetUniformLocation(programSun, "color"), sunColor.x * 2.5f, sunColor.y * 2.5f, sunColor.z * 2.5f);
	glUniform1f(glGetUniformLocation(programSun, "exposition"), exposition);
	Core::DrawContext(models::sphereContext);
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
	renderSkybox(models::skyboxContext, glm::translate(glm::scale(glm::mat4(), glm::vec3(4.0f)), glm::vec3(0.0f,2.0f,0.0f)), textures::skybox);

	//time and delta time
	float time = glfwGetTime();
	updateDeltaTime(time);

	//shadow
	renderShadowapSun();

	//sun
	renderSun();
	

	glUseProgram(program);
	
	//render structures
	drawObjectPBR(models::ceilingContext, glm::translate(glm::mat4(), glm::vec3(0.0f, -0.01f, 0.0f)), glm::vec3(10.0f, 10.0f, 10.0f), 0.8f, 0.0f);
	drawObjectPBR(models::roofContext, glm::mat4(), glm::vec3(40.0f, 0.0f, 0.0f), 0.8f, 0.0f);
	drawObjectPBR(models::floorContext, glm::mat4(), glm::vec3(5.0f, 0.0f, 0.0f), 1.0f, 1.0f);
	drawObjectPBR(models::roomContext, glm::mat4(), glm::vec3(10.0f, 0.1f, 3.0f), 0.8f, 0.0f);
	drawObjectPBR(models::groundContext, glm::mat4(), glm::vec3(2.0f, 3.5f, 2.0f), 1.0f, 1.0f);

	//render furnitures
	drawObjectPBR(models::bedContext, glm::mat4(), glm::vec3(0.03f, 0.03f, 0.03f), 0.2f, 0.0f);
	drawObjectPBR(models::chairContext, glm::mat4(), glm::vec3(0.2f, 0.4f, 0.8f), 0.4f, 0.0f);
	drawObjectPBR(models::deskContext, glm::mat4(), glm::vec3(0.4f, 0.1f, 0.0f), 0.2f, 0.0f);
	drawObjectPBR(models::doorContext, glm::mat4(), glm::vec3(0.4f, 0.1f, 0.05f), 0.2f, 0.0f);
	drawObjectPBR(models::drawerContext, glm::mat4(), glm::vec3(0.4f, 0.08f, 0.03f), 0.2f, 0.0f);
	drawObjectPBR(models::marbleBustContext, glm::mat4(), glm::vec3(1.0f, 1.0f, 1.0f), 0.5f, 1.0f);
	drawObjectPBR(models::materaceContext, glm::mat4(), glm::vec3(0.9f, 0.9f, 0.9f), 0.8f, 0.0f);
	drawObjectPBR(models::pencilsContext, glm::mat4(), glm::vec3(0.1f, 0.02f, 0.0f), 0.1f, 0.0f);
	drawObjectPBR(models::hugeWindowContext, glm::mat4(), glm::vec3(0.4f, 0.1f, 0.05f), 0.2f, 0.0f);
	drawObjectPBR(models::smallWindow1Context, glm::mat4(), glm::vec3(0.4f, 0.1f, 0.05f), 0.2f, 0.0f);
	drawObjectPBR(models::smallWindow2Context, glm::mat4(), glm::vec3(0.4f, 0.1f, 0.05f), 0.2f, 0.0f);
	drawObjectPBR(models::painting, glm::mat4(), glm::vec3(10.0f, 10.0f, 10.0f), 0.0f, 0.0f);

	//render and animate player
	animatePlayer();

	//update player light cone
	spotlightPos = playerPos + 0.2 * playerDir;
	spotlightConeDir = playerDir;

	glUseProgram(0);
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


void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
}

void loadSkyboxTextures() {
	int w, h;

	glGenTextures(1, &textures::skybox);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures::skybox);

	const char* filepaths[6] = {
		"models/skybox/textures/px.png",
		"models/skybox/textures/nx.png",
		"models/skybox/textures/py.png",
		"models/skybox/textures/ny.png",
		"models/skybox/textures/pz.png",
		"models/skybox/textures/nz.png"
	};
	for (unsigned int i = 0; i < 6; i++)
	{
		unsigned char* image = SOIL_load_image(filepaths[i], &w, &h, 0, SOIL_LOAD_RGBA);
		if (image) {
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image
			);
		}
		else {
			std::cout << "Failed to load texture: " << filepaths[i] << std::endl;
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

// init ------------------------------------------------------------------------------------------------------- init
void init(GLFWwindow* window)
{
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);
	program = shaderLoader.CreateProgram("shaders/shader_9_1.vert", "shaders/shader_9_1.frag");
	programTest = shaderLoader.CreateProgram("shaders/test.vert", "shaders/test.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_8_sun.vert", "shaders/shader_8_sun.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");

	//load structures
	loadModelToContext("./models/structures/sphere.obj", models::sphereContext);
	loadModelToContext("./models/structures/room.obj", models::roomContext);
	loadModelToContext("./models/structures/roof.obj", models::roofContext);
	loadModelToContext("./models/structures/ceiling.obj", models::ceilingContext);
	loadModelToContext("./models/structures/floor.obj", models::floorContext);
	loadModelToContext("./models/structures/ground/ground.obj", models::groundContext);

	//load furnitures
	loadModelToContext("./models/furnitures/bed.obj", models::bedContext);
	loadModelToContext("./models/furnitures/chair.obj", models::chairContext);
	loadModelToContext("./models/furnitures/desk.obj", models::deskContext);
	loadModelToContext("./models/furnitures/door.obj", models::doorContext);
	loadModelToContext("./models/furnitures/drawer.obj", models::drawerContext);
	loadModelToContext("./models/furnitures/marable_bust.obj", models::marbleBustContext);
	loadModelToContext("./models/furnitures/materace.obj", models::materaceContext);
	loadModelToContext("./models/furnitures/pencils.obj", models::pencilsContext);
	loadModelToContext("./models/furnitures/huge_window.obj", models::hugeWindowContext);
	loadModelToContext("./models/furnitures/small_window_1.obj", models::smallWindow1Context);
	loadModelToContext("./models/furnitures/small_window_2.obj", models::smallWindow2Context);
	loadModelToContext("./models/furnitures/painting/painting.obj", models::painting);

	//load player
	loadModelToContext("./models/flyModels/fly0.obj", models::fly0Context);
	loadModelToContext("./models/flyModels/fly1.obj", models::fly1Context);
	loadModelToContext("./models/flyModels/fly2.obj", models::fly2Context);
	loadModelToContext("./models/flyModels/fly3.obj", models::fly3Context);
	loadModelToContext("./models/flyModels/fly4.obj", models::fly4Context);
	loadModelToContext("./models/flyModels/fly5.obj", models::fly5Context);

	//load skybox and it's textures
	loadModelToContext("./models/skybox/cube.obj", models::skyboxContext);
	loadSkyboxTextures();
}


//shutdown ---------------------------------------------------------------------------------------------------------------------------------------------------------- shutdown
void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(program);
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
