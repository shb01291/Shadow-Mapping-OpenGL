#define GLEW_STATIC
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "j3a.hpp"


#pragma comment (lib, "glew32s")
#pragma comment (lib, "opengl32")
#pragma commnet (lib, "glfw3")


#include <vector>
#define GLM_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/projection.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "toys.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace glm;

void render(GLFWwindow* window);
void init();
void mouseButtonCallback(GLFWwindow*, int, int, int);
void cursorMotionCallback(GLFWwindow*, double, double);

GLuint triangleVBO = 0;
GLuint normalVBO = 0;
GLuint vertexArrayID = 0;
GLuint indexVBOID = 0;
GLuint texCoordVBO = 0;


GLuint diffTexID = 0;
GLuint bumpTexID = 0;

GLuint shadowTex=0;
GLuint shadowDepth=0;
GLuint shadowFBO = 0;

Program program;
Program shadowProgram;

float cameraDistance = 3;
glm::vec3 sceneCenter = glm::vec3(0, 0, 0);
float cameraYaw = 0.f;
float cameraPitch = 0.f;
float cameraFov = 30.f;

vec3 lightPos = vec3(3, 3, 3);
vec3 lightColor = vec3(1);

double lastX, lastY;



int main(int argc, const char* argv[])
{
	if (!glfwInit()) exit(EXIT_FAILURE);


	glfwWindowHint(GLFW_SAMPLES, 8);
	GLFWwindow* window = glfwCreateWindow(640, 480, "201621034", 0, 0);
	glfwMakeContextCurrent(window);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorMotionCallback);

	glewInit();


	init();
	glfwSwapInterval(1);
	while (!glfwWindowShouldClose(window)) {
		render(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();


}



void init() {

	loadJ3A("dwarf.j3a");
	int texWidth, texHeight, texChannels;
	void* buffer = stbi_load(diffuseMap[0].c_str(), &texWidth, &texHeight, &texChannels, 4);

	glGenTextures(1, &diffTexID);
	glBindTexture(GL_TEXTURE_2D, diffTexID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(buffer);

	buffer = stbi_load(bumpMap[0].c_str(), &texWidth, &texHeight, &texChannels, 4);

	glGenTextures(1, &bumpTexID);
	glBindTexture(GL_TEXTURE_2D, bumpTexID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(buffer);
	program.loadShaders("shader.vert", "shader.frag");
	shadowProgram.loadShaders("shadow.vert", "shadow.frag");
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	glGenBuffers(1, &triangleVBO);
	glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
	glBufferData(GL_ARRAY_BUFFER, nVertices[0] * sizeof(glm::vec3), vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glBufferData(GL_ARRAY_BUFFER, nVertices[0] * sizeof(glm::vec3), normals[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &texCoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
	glBufferData(GL_ARRAY_BUFFER, nVertices[0] * sizeof(glm::vec2), texCoords[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);


	glGenBuffers(1, &indexVBOID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBOID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, nTriangles[0] * sizeof(glm::u32vec3), triangles[0], GL_STATIC_DRAW);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FRAMEBUFFER_SRGB);

	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 1024, 1024, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &shadowDepth);
	glBindTexture(GL_TEXTURE_2D, shadowDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, shadowTex, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowDepth, 0);
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) printf("FBO Error\n");
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);



}

float rotAngle = 0;

void render(GLFWwindow* window) {

	int w, h;
	GLuint loc; 
	mat4 modelMat = mat4(1);
	

	//shadow map rendering
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glViewport(0, 0, 1024, 1024);
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shadowProgram.programID);
	mat4 shadowProjMat = ortho(-2.f, 2.f, -2.f, 2.f, 0.01f, 10.f);
	mat4 shadowViewMat = lookAt(lightPos, vec3(0, 0, 0), vec3(0, 1, 0));
	mat4 shadowMVP = shadowProjMat * shadowViewMat * modelMat;
	loc = glGetUniformLocation(shadowProgram.programID, "shadowMVP");
	glUniformMatrix4fv(loc, 1, 0, value_ptr(shadowMVP));


	glBindVertexArray(vertexArrayID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBOID);
	glDrawElements(GL_TRIANGLES, nTriangles[0] * 3, GL_UNSIGNED_INT, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

	//End of shadow rendering


	glfwGetFramebufferSize(window, &w, &h);
	glViewport(0, 0, w, h);
	glClearColor(0, 0, .5, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glUseProgram(program.programID);
	loc = glGetUniformLocation(program.programID, "projMat");
	mat4 projMat = perspective(cameraFov / 180.f * 3.1415926535f, w / (float)h, 0.01f, 100.f);
	glUniformMatrix4fv(loc, 1, 0, value_ptr(projMat));

	loc = glGetUniformLocation(program.programID, "viewMat");
	vec3 cameraPosition = vec3(rotate(cameraYaw, vec3(0, 1, 0))
		* rotate(cameraPitch, vec3(1, 0, 0)) * vec4(0, 0, cameraDistance, 0));

	mat4 viewMat = lookAt(cameraPosition, vec3(0, 0, 0), vec3(0, 1, 0));
	glUniformMatrix4fv(loc, 1, 0, value_ptr(viewMat));

	loc = glGetUniformLocation(program.programID, "cameraPos");
	glUniform3fv(loc, 1, value_ptr(cameraPosition));

	loc = glGetUniformLocation(program.programID, "lightColor");
	glUniform3fv(loc, 1, value_ptr(lightColor));
	
	loc = glGetUniformLocation(program.programID, "lightPos");
	glUniform3fv(loc, 1, value_ptr(lightPos));

	mat4 shadowBias = translate(vec3(0.5)) * scale(vec3(0.5));
	mat4 shadowBiasMVP = shadowBias * shadowMVP;
	loc = glGetUniformLocation(program.programID, "shadowBiasMVP");
	glUniformMatrix4fv(loc, 1, 0, value_ptr(shadowBiasMVP));

	glActiveTexture(GL_TEXTURE0+2);
	glBindTexture(GL_TEXTURE_2D, diffTexID);
	loc = glGetUniformLocation(program.programID, "diffTex");
	glUniform1i(loc, 2);

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, bumpTexID);
	loc = glGetUniformLocation(program.programID, "bumpTex");
	glUniform1i(loc, 1);


	glBindVertexArray(vertexArrayID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBOID);
	glDrawElements(GL_TRIANGLES, nTriangles[0] * 3, GL_UNSIGNED_INT, 0);

	

	glfwSwapBuffers(window);

}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		lastX = int(xpos);
		lastY = int(ypos);

	}
}

void cursorMotionCallback(GLFWwindow* window, double xpos, double ypos) {
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
			cameraFov += (ypos - lastY) / 30;
		}
		else {
			cameraPitch -= (ypos - lastY) / 300;
			cameraPitch = glm::clamp(cameraPitch, -1.f, 1.f);
			cameraYaw -= (xpos - lastX) / 300;
		}
		lastX = int(xpos);
		lastY = int(ypos);
	}


}