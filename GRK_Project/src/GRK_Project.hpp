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
#include "Shadows.hpp"

//window variables
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
int WIDTH = 1000, HEIGHT = 1000;

//variables -------------------------------------------------------------------------------------------------------------------------------------------------------- variables

//depht
GLuint depthMapSunFBO;
GLuint depthMapSun;
GLuint depthMapTableLightFBO;
GLuint depthMapTableLight;

//shaders
GLuint programPBR;
GLuint programTex;
GLuint programSun;
GLuint programSkybox;
GLuint programDepth;
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

//main lamp light
glm::vec3 pointlightPos = glm::vec3(0.0f, 2.0f, 0.0f);
glm::vec3 pointlightColorON = glm::vec3(0.9, 0.6, 0.6) * 4;
glm::vec3 pointlightColor = pointlightColorON*0;

//table lamp light
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

//dog animation
float dogTailRotation = -45.0f;
bool doggTailRotationIncreasing = true;

//bird animation
int birdAnimationState = 0;
bool birdAnimationStateRising = true;
float birdRotation = 180.0f;
glm::mat4 birdPos = glm::mat4();

//switch listener
int switchDelay = 50;
float mainLampRotation = 0;

//projections for shadows
glm::mat4 sunVP = glm::ortho(-25.f, 25.f, -25.f, 25.f, 1.0f, 80.0f) * glm::lookAt(sunPos, sunPos - sunDir, glm::vec3(0,1,0));
glm::mat4 tableLightVP = glm::perspective(glm::radians(90.0f), 1.0f, 0.05f, 25.0f) * glm::lookAt(spotlightPos, spotlightPos + spotlightConeDir, glm::vec3(0,1,0));


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
	glm::mat4 rotate = from_pivot * glm::rotate(glm::mat4(), glm::radians(degrees), axis) * to_pivot;
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

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMapTableLight);
	glUniformMatrix4fv(glGetUniformLocation(program, "tableLightVP"), 1, GL_FALSE, (float*)&tableLightVP);

	glActiveTexture(GL_TEXTURE0+1);
	glBindTexture(GL_TEXTURE_2D, depthMapSun);
	glUniformMatrix4fv(glGetUniformLocation(program, "sunVP"), 1, GL_FALSE, (float*)&sunVP);


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
	glm::vec3 pivot = glm::vec3(-0.411f, 0.957f, -4.628f);
	glm::mat4 rotate = rotateAroundPivot(doorRotation, glm::vec3(0, 1, 0), pivot);
	drawObjectPBR(models::door, glm::mat4()*rotate, glm::vec3(), textures::door, 0.9f, 0.0f, 3.0f);
}

void animateTail()
{
	if (doggTailRotationIncreasing)
	{
		if (dogTailRotation >= 35.0f)
		{
			doggTailRotationIncreasing = false;
		}
		else
		{
			dogTailRotation += 4.5;
		}
	}
	else
	{
		if (dogTailRotation <= -35.0f)
		{
			doggTailRotationIncreasing = true;
		}
		else
		{
			dogTailRotation -= 4.5;
		}
	}
	glm::vec3 pivot = glm::vec3(-2.97f, 0.44f, -3.81f);
	glm::mat4 rotate = rotateAroundPivot(dogTailRotation, glm::vec3(1, 0, 1), pivot);
	drawObjectPBR(models::dogTail, glm::mat4()*rotate, glm::vec3(), textures::dogTail, 0.8f, 0.0f, 5.0f);
}

void animateBird(glm::mat4 startingBirdPos)
{
	birdRotation -= 1.0f;
	if (birdRotation == 0.0f)
	{
		birdRotation = 360.0f;
	}
	glm::vec3 pivot = glm::vec3(-3.0f, 5.0f, 0.0f);
	glm::mat4 rotate = rotateAroundPivot(birdRotation, glm::vec3(0, 1, 0), pivot);

	if (birdAnimationStateRising)
	{
		if (birdAnimationState < 6)
		{
			birdAnimationState++;
		}
		else
		{
			birdAnimationState--;
			birdAnimationStateRising = false;
		}
	}
	else
	{
		if (birdAnimationState > 0)
		{
			birdAnimationState--;
		}
		else
		{
			birdAnimationState++;
			birdAnimationStateRising = true;
		}
	}

	Core::RenderContext bird;
	GLuint texture;

	switch (birdAnimationState)
	{
	case 0:
		bird = models::bird0; texture = textures::bird0; break;
	case 1:
		bird = models::bird1; texture = textures::bird1; break;
	case 2:
		bird = models::bird2; texture = textures::bird2; break;
	case 3:
		bird = models::bird3; texture = textures::bird3; break;
	case 4:
		bird = models::bird4; texture = textures::bird4; break;
	case 5:
		bird = models::bird5; texture = textures::bird5; break;
	case 6:
		bird = models::bird6; texture = textures::bird6; break;
	}

	birdPos = startingBirdPos * rotate;
	drawObjectPBR(bird, birdPos, glm::vec3(), texture, 0.8f, 0.0f, 5.0f);
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

void renderShadows(GLuint program, GLuint FBO, glm::mat4 VP) {

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	//render structures
	drawObjectDepth(program, models::ceiling, VP, glm::mat4());
	drawObjectDepth(program, models::roof, VP, glm::mat4());
	drawObjectDepth(program, models::floor, VP, glm::mat4());
	drawObjectDepth(program, models::room, VP, glm::mat4());
	drawObjectDepth(program, models::ground, VP, glm::mat4());
	drawObjectDepth(program, models::walls, VP, glm::mat4());

	//render furnitures
	drawObjectDepth(program, models::bed, VP, glm::mat4());
	drawObjectDepth(program, models::chair, VP, glm::mat4());
	drawObjectDepth(program, models::desk, VP, glm::mat4());
	drawObjectDepth(program, models::jamb, VP, glm::mat4());
	drawObjectDepth(program, models::drawer, VP, glm::mat4());
	drawObjectDepth(program, models::marbleBust, VP, glm::mat4());
	drawObjectDepth(program, models::mattress, VP, glm::mat4());
	drawObjectDepth(program, models::pencils, VP, glm::mat4());
	drawObjectDepth(program, models::hugeWindow, VP, glm::mat4());
	drawObjectDepth(program, models::smallWindow1, VP, glm::mat4());
	drawObjectDepth(program, models::smallWindow2, VP, glm::mat4());
	drawObjectDepth(program, models::painting, VP, glm::mat4());
	drawObjectDepth(program, models::carpet, VP, glm::mat4());
	drawObjectDepth(program, models::stool, VP, glm::mat4());
	drawObjectDepth(program, models::stool, VP, glm::translate(glm::mat4(), glm::vec3(-0.5f, 0, -0.5f)));
	drawObjectDepth(program, models::barbells, VP, glm::mat4());
	drawObjectDepth(program, models::mat, VP, glm::mat4());
	drawObjectDepth(program, models::poster, VP, glm::mat4());
	drawObjectDepth(program, models::tableLamp, VP, glm::mat4());
	drawObjectDepth(program, models::lamp, VP, glm::mat4());
	drawObjectDepth(program, models::switch_, VP, glm::mat4() * rotateAroundPivot(mainLampRotation, glm::vec3(0, 0, 1), glm::vec3(0.8f, 1.25f, -4.6f)));
	drawObjectDepth(program, models::pillow1, VP, glm::mat4());
	drawObjectDepth(program, models::pillow2, VP, glm::mat4());
	drawObjectDepth(program, models::duvet, VP, glm::mat4());
	drawObjectDepth(program, models::shelf, VP, glm::mat4());
	drawObjectDepth(program, models::book, VP, glm::mat4());
	drawObjectDepth(program, models::shelf, VP, glm::translate(glm::mat4(), glm::vec3(-1.2f, -0.3f, 0.0f)));
	drawObjectDepth(program, models::books, VP, glm::translate(glm::mat4(), glm::vec3(-1.2f, -0.3f, 0.0f)));
	drawObjectDepth(program, models::book, VP, glm::translate(glm::mat4(), glm::vec3(-0.8f, -1.40f, -0.55f)));
	drawObjectDepth(program, models::openBook, VP, glm::mat4());
	drawObjectDepth(program, models::dogBed, VP, glm::mat4());
	drawObjectDepth(program, models::waterBowl, VP, glm::mat4());
	drawObjectDepth(program, models::foodBowl, VP, glm::mat4());
	drawObjectDepth(program, models::boneToy, VP, glm::mat4());
	drawObjectDepth(program, models::tennisBall, VP, glm::mat4());
	drawObjectDepth(program, models::umbrella, VP, glm::mat4());
	drawObjectDepth(program, models::umbrellaHolder, VP, glm::mat4());
	drawObjectDepth(program, models::hatstand, VP, glm::mat4());
	drawObjectDepth(program, models::hatShelf, VP, glm::mat4());
	drawObjectDepth(program, models::capHat, VP, glm::mat4());
	drawObjectDepth(program, models::highHat, VP, glm::mat4());
	drawObjectDepth(program, models::beret, VP, glm::mat4());
	drawObjectDepth(program, models::jacket, VP, glm::mat4());
	drawObjectDepth(program, models::wardrobe, VP, glm::mat4());
	drawObjectDepth(program, models::cabinet, VP, glm::mat4());
	drawObjectDepth(program, models::table, VP, glm::mat4());
	drawObjectDepth(program, models::pot, VP, glm::mat4());
	drawObjectDepth(program, models::pot, VP, glm::translate(glm::mat4(), glm::vec3(-6.45f, 0.285f, -1.85f)));
	drawObjectDepth(program, models::pot, VP, glm::translate(glm::mat4(), glm::vec3(-6.35f, 0.285f, -1.45f)));
	drawObjectDepth(program, models::pot, VP, glm::translate(glm::mat4(), glm::vec3(-6.25f, 0.285f, -2.15f)));
	drawObjectDepth(program, models::bench, VP, glm::mat4());

	//render environment
	drawObjectDepth(program, models::tree, VP, glm::translate(glm::mat4(), glm::vec3(5.3f, 0.0f, 7.0f)));
	drawObjectDepth(program, models::tree, VP, glm::translate(glm::mat4(), glm::vec3(7.5f, -0.3f, 4.0f)));
	drawObjectDepth(program, models::tree, VP, glm::translate(glm::mat4(), glm::vec3(10.0f, -0.5f, 1.0f)));
	drawObjectDepth(program, models::tree, VP, glm::translate(glm::mat4(), glm::vec3(7.0f, -0.3f, -3.0f)));
	drawObjectDepth(program, models::tree, VP, glm::translate(glm::mat4(), glm::vec3(5.3f, 0.0f, -8.0f)));
	drawObjectDepth(program, models::tree, VP, glm::translate(glm::mat4(), glm::vec3(0.3f, 0.0f, 8.4f)));
	drawObjectDepth(program, models::tree, VP, glm::translate(glm::mat4(), glm::vec3(1.3f, 0.0f, 13.4f)));
	drawObjectDepth(program, models::tree, VP, glm::translate(glm::mat4(), glm::vec3(10.6f, -0.5f, 14.0f)));
	drawObjectDepth(program, models::tree, VP, glm::translate(glm::mat4(), glm::vec3(15.1f, -0.7f, 8.0f)));
	drawObjectDepth(program, models::tree, VP, glm::translate(glm::mat4(), glm::vec3(20.8f, -1.0f, 2.0f)));
	drawObjectDepth(program, models::tree, VP, glm::translate(glm::mat4(), glm::vec3(16.3f, -0.7f, -7.0f)));
	drawObjectDepth(program, models::tree, VP, glm::translate(glm::mat4(), glm::vec3(8.3f, -0.5f, -12.7f)));
	drawObjectDepth(program, models::bush, VP, glm::translate(glm::mat4(), glm::vec3(8.3f, 0.0f, -10.7f)));
	drawObjectDepth(program, models::bush, VP, glm::translate(glm::mat4(), glm::vec3(9.3f, 0.0f, -5.7f)));
	drawObjectDepth(program, models::bush, VP, glm::translate(glm::mat4(), glm::vec3(3.3f, 0.0f, -6.0f)));
	drawObjectDepth(program, models::bush, VP, glm::translate(glm::mat4(), glm::vec3(5.3f, 0.0f, 0.0f)));
	drawObjectDepth(program, models::bush, VP, glm::translate(glm::mat4(), glm::vec3(11.3f, 0.0f, 2.7f)));
	drawObjectDepth(program, models::rock, VP, glm::translate(glm::mat4(), glm::vec3(11.3f, 0.0f, -3.0f)));
	drawObjectDepth(program, models::rock, VP, glm::translate(glm::mat4(), glm::vec3(11.3f, 0.0f, -8.7f)));
	drawObjectDepth(program, models::rock, VP, glm::translate(glm::mat4(), glm::vec3(7.5f, 0.0f, -13.0f)));
	drawObjectDepth(program, models::rock, VP, glm::translate(glm::mat4(), glm::vec3(7.0f, 0.0f, -0.5f)));
	drawObjectDepth(program, models::rock, VP, glm::translate(glm::mat4(), glm::vec3(4.5f, 0.0f, -7.0f)));
	drawObjectDepth(program, models::dog, VP, glm::mat4());
	drawObjectDepth(program, models::plant, VP, glm::mat4());
	drawObjectDepth(program, models::plant, VP, glm::translate(glm::mat4(), glm::vec3(-6.45f, 0.285f, -1.85f)));
	drawObjectDepth(program, models::plant, VP, glm::translate(glm::mat4(), glm::vec3(-6.35f, 0.285f, -1.45f)));
	drawObjectDepth(program, models::bird2, VP, birdPos);


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WIDTH, HEIGHT);
}

//render scene --------------------------------------------------------------------------------- render scene
void renderScene(GLFWwindow* window)
{


	//shadows
	renderShadows(programDepth, depthMapSunFBO, sunVP);
	renderShadows(programDepth, depthMapTableLightFBO, tableLightVP);

	//skybox
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderSkybox(models::skybox, glm::translate(glm::scale(glm::mat4(), glm::vec3(2.5f, 2.0f, 2.0f)), glm::vec3(3.0f, 1.0f, 0.0f)), textures::skybox);


	//time and delta time
	float time = glfwGetTime();
	updateDeltaTime(time);

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
	drawObjectPBR(models::switch_, glm::mat4() * rotateAroundPivot(mainLampRotation, glm::vec3(0,0,1), glm::vec3(0.8f, 1.25f, -4.6f)), glm::vec3(0.95f, 0.95f, 0.85f), NULL, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::pillow1, glm::mat4(), glm::vec3(), textures::pillow1, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::pillow2, glm::mat4(), glm::vec3(), textures::pillow2, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::duvet, glm::mat4(), glm::vec3(), textures::duvet, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::shelf, glm::mat4(), glm::vec3(), textures::shelf, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::book, glm::mat4(), glm::vec3(), textures::book, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::shelf, glm::translate(glm::mat4(), glm::vec3(-1.2f, -0.3f, 0.0f)), glm::vec3(), textures::shelf, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::books, glm::translate(glm::mat4(), glm::vec3(-1.2f, -0.3f, 0.0f)), glm::vec3(), textures::books, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::book, glm::translate(glm::mat4(), glm::vec3(-0.8f, -1.40f, -0.55f)), glm::vec3(), textures::book, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::openBook, glm::mat4(), glm::vec3(), textures::openBook, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::dogBed, glm::mat4(), glm::vec3(), textures::dogBed, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::waterBowl, glm::mat4(), glm::vec3(), textures::waterBowl, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::foodBowl, glm::mat4(), glm::vec3(), textures::foodBowl, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::boneToy, glm::mat4(), glm::vec3(), textures::boneToy, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::tennisBall, glm::mat4(), glm::vec3(), textures::tennisBall, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::umbrella, glm::mat4(), glm::vec3(), textures::umbrella, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::umbrellaHolder, glm::mat4(), glm::vec3(), textures::umbrellaHolder, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::hatstand, glm::mat4(), glm::vec3(), textures::hatstand, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::hatShelf, glm::mat4(), glm::vec3(), textures::hatShelf, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::capHat, glm::mat4(), glm::vec3(), textures::capHat, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::highHat, glm::mat4(), glm::vec3(), textures::highHat, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::beret, glm::mat4(), glm::vec3(), textures::beret, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::jacket, glm::mat4(), glm::vec3(), textures::jacket, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::wardrobe, glm::mat4(), glm::vec3(), textures::wardrobe, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::cabinet, glm::mat4(), glm::vec3(), textures::cabinet, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::table, glm::mat4(), glm::vec3(), textures::table, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::pot, glm::mat4(), glm::vec3(), textures::pot, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::pot, glm::translate(glm::mat4(), glm::vec3(-6.45f, 0.285f, -1.85f)), glm::vec3(), textures::pot, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::pot, glm::translate(glm::mat4(), glm::vec3(-6.35f, 0.285f, -1.45f)), glm::vec3(), textures::pot, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::pot, glm::translate(glm::mat4(), glm::vec3(-6.25f, 0.285f, -2.15f)), glm::vec3(), textures::pot, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::bench, glm::mat4(), glm::vec3(), textures::bench, 0.8f, 0.0f, 5.0f);

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
	drawObjectPBR(models::dog, glm::mat4(), glm::vec3(), textures::dog, 0.8f, 0.0f, 5.0f);
	animateTail();
	drawObjectPBR(models::plant, glm::mat4(), glm::vec3(), textures::plant, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::plant, glm::translate(glm::mat4(), glm::vec3(-6.45f, 0.285f, -1.85f)), glm::vec3(), textures::plant, 0.8f, 0.0f, 5.0f);
	drawObjectPBR(models::plant, glm::translate(glm::mat4(), glm::vec3(-6.35f, 0.285f, -1.45f)), glm::vec3(), textures::plant, 0.8f, 0.0f, 5.0f);
	animateBird(glm::translate(glm::mat4(), glm::vec3(0, 2, 0)));

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
	programDepth = shaderLoader.CreateProgram("shaders/shader_shadow.vert", "shaders/shader_shadow.frag");

	loadAllModels();

	//init depth maps
	initDepthMap(depthMapSun, depthMapSunFBO);
	initDepthMap(depthMapTableLight, depthMapTableLightFBO);

	PlaySound(TEXT("./sounds/backgroundSound.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
	
}


//shutdown ---------------------------------------------------------------------------------------------------------------------------------------------------------- shutdown
void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(programPBR);
	shaderLoader.DeleteProgram(programTex);
	shaderLoader.DeleteProgram(programSun);
	shaderLoader.DeleteProgram(programSkybox);
	shaderLoader.DeleteProgram(programDepth);
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
		sunx += 0.1;
	}
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
		sunx -= 0.1;
	}
	//y
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
		suny += 0.1;
	}
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
		suny -= 0.1;
	}
	//z
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
		sunz += 0.1;
	}
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
		sunz -= 0.1;
	}
		

	// debug info
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		printf("spaceshipPos = glm::vec3(%ff, %ff, %ff);\n", playerPos.x, playerPos.y, playerPos.z);
		printf("spaceshipDir = glm::vec3(%ff, %ff, %ff);\n", playerDir.x, playerDir.y, playerDir.z);
		printf("sunDir = glm::vec3(%ff, %ff, %ff);\n\n", sunx, suny, suny);
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
