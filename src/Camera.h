#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "shaderClass.h"
#include "ImguiWindow.h"

#define SENSITIVITY_DEFAULT 100.0f;
#define SPEED_DEFAULT 0.1f;

class Camera {
public:
	glm::vec3 Position;
	glm::vec3 Orientation = glm::vec3(0.0f, 1.0f, 0.0f);

	unsigned int width;
	unsigned int height;

	float FOVdeg = 45.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;

	float speed = SPEED_DEFAULT;
	float sensitivity = SENSITIVITY_DEFAULT;

	Camera(unsigned int width, unsigned int height, glm::vec3 position);

	void updateMatrix();
	void Matrix(Shader& shader, const char* uniform);
	void Inputs(GLFWwindow* window, ImguiWindow& imguiWindow);

private:

	glm::mat4 cameraMatrix = glm::mat4(1.0f);

	bool speedUp = false;	// used in the speed up section of Inputs(...)
	bool firstClick = true;	// just to avoid camera snapping when initially selecting window
	bool lastWindowInput = false;

	const glm::vec3 Up = glm::vec3(0.0f, 0.0f, 1.0f);	// a const vector used to define the up vector
};