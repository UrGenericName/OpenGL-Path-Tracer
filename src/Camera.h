#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "shaderClass.h"

#define SENSITIVITY_DEFAULT 100.0f;
#define SPEED_DEFAULT 0.1f;

class Scene;

class Camera {
public:

	friend Scene;

	glm::vec3 Position;
	glm::vec3 Orientation = glm::vec3(0.0f, 1.0f, 0.0f);

	std::function<void(Camera&, unsigned int)> animation = nullptr;

	unsigned int width;
	unsigned int height;

	float FOVdeg = 45.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;

	float speed = SPEED_DEFAULT;
	float sensitivity = SENSITIVITY_DEFAULT;

	Camera(unsigned int width, unsigned int height, glm::vec3 position);
	
	Camera& operator=(Camera other);

	void updateMatrix();
	void Matrix(Shader& shader, const char* uniform);

private:

	glm::mat4 cameraMatrix = glm::mat4(1.0f);
	const glm::vec3 Up = glm::vec3(0.0f, 0.0f, 1.0f);	// a const vector used to define the up vector
};