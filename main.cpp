//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>
#include <time.h>
#include <stdlib.h>

int glWindowWidth = 1800;
int glWindowHeight = 1000;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
GLuint modelLoc2;
glm::mat4 view;
GLuint viewLoc;
GLuint viewLoc2;
glm::mat4 projection;
GLuint projectionLoc;
GLuint projectionLoc2;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
GLuint normalMatrixLoc2;

glm::vec3 lightDir;
GLuint lightDirLoc;
GLuint lightDirLoc2;
glm::vec3 lightColor;
GLuint lightColorLoc;
GLuint lightColorLoc2;
glm::mat4 lightRotation;

glm::vec3 pointLight = glm::vec3(61.61f, 6.525f, 103.9f);
GLuint pointLightLoc;
glm::vec3 pointLightColor = glm::vec3(238.0f / 255.0f, 232.0f / 255.0f, 170.0f / 255.0f);
GLuint pointLightColorLoc;

glm::vec3 pointLight2 = glm::vec3(27.329f, 0.25541f, 78.758f);
GLuint pointLightLoc2;
glm::vec3 pointLightColor2 = glm::vec3(1.0f, 127.0f / 255.0f, 36.0f / 255.0f);
GLuint pointLightColorLoc2;
float incLinear;
GLfloat incLinearLoc;
float incQuad;
GLfloat incQuadLoc;

gps::Camera myCamera(
				glm::vec3(0.0f, 5.0f, 25.0f), 
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 2.5f;

bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

float fogDensity = 0.0f;
GLfloat fogDensityLoc;

gps::Model3D nanosuit;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D leaf;
gps::Model3D winterScene;
gps::Model3D leftWing;
gps::Model3D rightWing;
gps::Model3D lake;
gps::Model3D lamppost;
gps::Model3D snowflake;
gps::Model3D fire;

gps::Shader myCustomShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader myCustomShader2;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;


GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);
	myCustomShader.useShaderProgram();
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

double mouseX = -1.0;
double mouseY = -1.0;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (mouseX < 0) {
		mouseX = xpos;
		mouseY = ypos;
	}
	else {
		double dx = xpos - mouseX;
		double dy = ypos - mouseY;
		myCamera.rotate(-dy * 0.007, dx * 0.007);
		mouseX = xpos;
		mouseY = ypos;
	}
}

bool wireframeMode = false;
bool pointMode = false;
bool snowfall = false;
bool fog = false;
bool night = false;
bool tour = false;

void processMovement()
{

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_Z]) {
		myCamera.rotate(-0.007f, 0.0f);
	}

	if (pressedKeys[GLFW_KEY_X]) {
		myCamera.rotate(0.007f, 0.0f);
	}

	if (pressedKeys[GLFW_KEY_C]) {
		myCamera.rotate(0.0f, -0.007f);
	}

	if (pressedKeys[GLFW_KEY_V]) {
		myCamera.rotate(0.0f, 0.007f);
	}

	if (pressedKeys[GLFW_KEY_R]) {
		wireframeMode = !wireframeMode;
	}

	if (pressedKeys[GLFW_KEY_T]) {
		pointMode = !pointMode;
	}

	if (pressedKeys[GLFW_KEY_Y]) {
		snowfall = !snowfall;
	}

	if (pressedKeys[GLFW_KEY_U]) {
		fog = !fog;
	}

	if (pressedKeys[GLFW_KEY_I]) {
		night = !night;
	}

	if (pressedKeys[GLFW_KEY_O]) {
		tour = !tour;
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_BLEND);
}

FILE* camCoordsFile = NULL;

void openFileForWriting() {
	camCoordsFile = fopen("tour.txt", "w");
}

void openFileForReading() {
	camCoordsFile = fopen("tour.txt", "r");
}

void closeFile() {
	fclose(camCoordsFile);
	camCoordsFile = NULL;
}

void writeCoords() {
		fprintf(camCoordsFile, "%f %f %f %f %f %f %f %f %f\n", myCamera.getCameraPosition().x, myCamera.getCameraPosition().y, myCamera.getCameraPosition().z, myCamera.getCameraTarget().x, myCamera.getCameraTarget().y, myCamera.getCameraTarget().z, myCamera.getCameraUpDirection().x, myCamera.getCameraUpDirection().y, myCamera.getCameraUpDirection().z);
}

void readCoordsAndMoveCam() {
	//openFileForReading();
	float posX, posY, posZ, tarX, tarY, tarZ, upX, upY, upZ;
	if (tour) {
		if (camCoordsFile == NULL) {
			openFileForReading();
		}
		if (fscanf(camCoordsFile, "%f%f%f%f%f%f%f%f%f", &posX, &posY, &posZ, &tarX, &tarY, &tarZ, &upX, &upY, &upZ) != 9) {
			tour = false;
			closeFile();
		}
		else {
			glm::vec3 pos = glm::vec3(posX, posY, posZ);
			glm::vec3 tar = glm::vec3(tarX, tarY, tarZ);
			glm::vec3 up = glm::vec3(upX, upY, upZ);
			myCamera.setCamera(pos, tar, up);
			//printf("%f %f %f %f %f %f %f %f %f\n", myCamera.getCameraPosition().x, myCamera.getCameraPosition().y, myCamera.getCameraPosition().z, myCamera.getCameraTarget().x, myCamera.getCameraTarget().y, myCamera.getCameraTarget().z, myCamera.getCameraUpDirection().x, myCamera.getCameraUpDirection().y, myCamera.getCameraUpDirection().z);

		}

	}
}

float** snowflakesCoordinates = (float**) malloc(400 * sizeof(float*));

void generateSnowflakes() {
	for (int i = 0; i < 400; i++)
		snowflakesCoordinates[i] = (float*)malloc(5 * sizeof(float));


	for (int i = 0; i < 400; i++) {
		snowflakesCoordinates[i][0] = rand() % 455 - 275;
		snowflakesCoordinates[i][1] = 100;
		snowflakesCoordinates[i][2] = rand() % 455 - 275;
		snowflakesCoordinates[i][3] = (rand() % 45 + 5) / 100.0f + 0.01f;
		snowflakesCoordinates[i][4] = rand() % 75;
	}
}

void initObjects() {
	screenQuad.LoadModel("objects/quad/quad.obj");
	winterScene.LoadModel("objects/winterscene/winterSceneFinalObj.obj");
	leftWing.LoadModel("objects/winterscene/leftWing.obj");
	rightWing.LoadModel("objects/winterscene/rightWing.obj");
	lake.LoadModel("objects/winterscene/lake.obj");
	lamppost.LoadModel("objects/winterscene/lamppost.obj");
	snowflake.LoadModel("objects/winterscene/snowflake1.obj");
	fire.LoadModel("objects/winterscene/fire2.obj");

	generateSnowflakes();
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depth.vert", "shaders/depth.frag");
	depthMapShader.useShaderProgram();
	myCustomShader2.loadShader("shaders/shaderReflect.vert", "shaders/shaderReflect.frag");
	myCustomShader2.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(-23.0f, 68.0f, 15.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");	
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	if(night)
		lightColor = glm::vec3(0.1f, 0.1f, 0.1f); //dark light
	else
		lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	//point light setting
	pointLightLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLight");
	glUniform3fv(pointLightLoc, 1, glm::value_ptr(pointLight));

	//set point light color
	pointLightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightColor");
	glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));

	//fire light setting
	pointLightLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "pointLight2");
	glUniform3fv(pointLightLoc2, 1, glm::value_ptr(pointLight2));

	//set fire light color
	pointLightColorLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightColor2");
	glUniform3fv(pointLightColorLoc2, 1, glm::value_ptr(pointLightColor2));

	incLinearLoc = glGetUniformLocation(myCustomShader.shaderProgram, "incLinear");
	glUniform1f(incLinearLoc, incLinear);

	incQuadLoc = glGetUniformLocation(myCustomShader.shaderProgram, "incQuad");
	glUniform1f(incQuadLoc, incQuad);


	myCustomShader2.useShaderProgram();

	modelLoc2 = glGetUniformLocation(myCustomShader2.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model));

	viewLoc2 = glGetUniformLocation(myCustomShader2.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrixLoc2 = glGetUniformLocation(myCustomShader2.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc2, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projectionLoc2 = glGetUniformLocation(myCustomShader2.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc2, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction
	lightDirLoc2 = glGetUniformLocation(myCustomShader2.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc2, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));

	//set light color
	lightColorLoc2 = glGetUniformLocation(myCustomShader2.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc2, 1, glm::value_ptr(lightColor));
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	const GLfloat near_plane = 0.1f, far_plane = 300.0f;
	glm::mat4 lightProjection = glm::ortho(-270.0f, 180.0f, -200.0f, 200.0f, near_plane, far_plane);

	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}

void generateSnowfall(gps::Shader shader) {
	for (int i = 0; i < 400; i++) {

		model = glm::translate(glm::mat4(1.0f), glm::vec3(snowflakesCoordinates[i][0], snowflakesCoordinates[i][1], snowflakesCoordinates[i][2]));
		model = glm::scale(model, glm::vec3(snowflakesCoordinates[i][4], snowflakesCoordinates[i][4], snowflakesCoordinates[i][4]));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		snowflake.Draw(shader);

		if (snowflakesCoordinates[i][1] > -5)
			snowflakesCoordinates[i][1] -= snowflakesCoordinates[i][3];
		else
			snowflakesCoordinates[i][1] = 100;
	}
}

glm::vec3 axRotatieLeftWing = glm::vec3(-118.4f, 14.42f, 36.34f);
glm::vec3 axRotatieRightWing = glm::vec3(-117.7f, 14.38f, 37.99f);

float amount = 0;
bool dir = false;	//false = down, true = up

void drawObjects(gps::Shader shader, bool depthPass) {

	if (wireframeMode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		if (pointMode)
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
	shader.useShaderProgram();
	
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 0.1f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	//nanosuit.Draw(shader);
	leaf.Draw(shader);
	winterScene.Draw(shader);
	lamppost.Draw(shader);
	fire.Draw(shader);

	if(snowfall)
		generateSnowfall(shader);

	if (fog)
		fogDensity = 0.00315f;
	else
		fogDensity = 0.0f;

	fogDensityLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity");
	glUniform1f(fogDensityLoc, fogDensity);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	//ground.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), axRotatieLeftWing);
	model = glm::rotate(model, glm::radians(360.0f) * amount, glm::vec3(0.8f, 0.54f, -0.55f));
	model = glm::translate(model, -axRotatieLeftWing);
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	leftWing.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), axRotatieRightWing);
	model = glm::rotate(model, glm::radians(360.0f) * -amount, glm::vec3(0.9f, 0.70f, -0.33f));
	model = glm::translate(model, -axRotatieRightWing);
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	rightWing.Draw(shader);
	
	if (!dir)
		amount -= 0.01f;
	else
		amount += 0.01f;

	if (glm::radians(360.0f) * amount > 0.5) {
		amount = 0.00f;
		dir = false;
	}
	if (glm::radians(360.0f) * amount < -0.5) {
		amount = 0.00f;
		dir = true;
	}


	if (incLinear < 0.655f)
		incLinear += 0.05f;
	else
		incLinear = 0.0f;

	if (incQuad < 1.7925f)
		incQuad += 0.1f;
	else
		incQuad = 0.0f;

	incLinearLoc = glGetUniformLocation(myCustomShader.shaderProgram, "incLinear");
	glUniform1f(incLinearLoc, incLinear);

	incQuadLoc = glGetUniformLocation(myCustomShader.shaderProgram, "incQuad");
	glUniform1f(incQuadLoc, incQuad);
}

void initSkyBox() {

	faces.push_back("textures/skybox/right.bmp");
	faces.push_back("textures/skybox/left.bmp");
	faces.push_back("textures/skybox/top.bmp");
	faces.push_back("textures/skybox/bottom.bmp");
	faces.push_back("textures/skybox/back.bmp");
	faces.push_back("textures/skybox/front.bmp");

	mySkyBox.Load(faces);
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));
}

void renderScene() {

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, true);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		myCustomShader.useShaderProgram();

		if (tour) {
			readCoordsAndMoveCam();
		}

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

		if (night)
			lightColor = glm::vec3(0.1f, 0.1f, 0.1f); //dark light
		else
			lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light

		lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

		drawObjects(myCustomShader, false);



		myCustomShader2.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view));

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc2, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		lake.Draw(myCustomShader2);

		

		//draw a white cube around the light

		/*lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = lightRotation;
		model = glm::translate(model, 1.0f * lightDir);
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		//lightCube.Draw(lightShader);*/
		mySkyBox.Draw(skyboxShader, view, projection);
	}
}

void cleanup() {
	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);

	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initSkyBox();

	glCheckError();

	//openFileForWriting();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();		

		//writeCoords();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//closeFile();
	cleanup();

	return 0;
}
