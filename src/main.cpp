#pragma once
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL

#include "DebugWindow.h"
#include "Scene.h"

#define WIDTH 1200
#define HEIGHT 800

GLFWwindow* initGLFW();
void configOpenGL(GLFWwindow* window);

int main() {

	GLFWwindow* window{ initGLFW() };
	DebugWindow debugWindow{ window };
	configOpenGL(window);


	Camera camera(WIDTH, HEIGHT, glm::vec3(0.0f, -12.0f, 4.5f));
	Scene scene(camera, 256, 256);
	scene.link();

	while (!glfwWindowShouldClose(window))
	{

		scene.Draw(window);
		debugWindow.drawImgui(scene);

		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

GLFWwindow* initGLFW() {

	glfwInit();

	// Pass info to glfw about the version and profile of OpenGL to use
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Creates the actual glfw window
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}

	// Sets our created window to active
	glfwMakeContextCurrent(window);

	// loads OpenGL
	gladLoadGL();

	return window;

}

void configOpenGL(GLFWwindow* window) {

	glViewport(0, 0, WIDTH, HEIGHT);
	glfwSwapBuffers(window);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
}