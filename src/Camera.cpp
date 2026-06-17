#include "Camera.h"

Camera::Camera(unsigned int i_width, unsigned int i_height, glm::vec3 i_position) : width(i_width), height(i_height), Position(i_position) {};

void Camera::updateMatrix(float FOVdeg, float nearPlane, float farPlane) {

	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	view = glm::lookAt(Position, Position + Orientation, Up);
	projection = glm::perspective(glm::radians(45.0f), (float)(width / height), 0.1f, 100.0f);

	cameraMatrix = projection * view;

}

void Camera::Matrix(Shader& shader, const char* uniform) {

	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));

}

void Camera::Inputs(GLFWwindow* window, ImguiWindow& imguiWindow) {

	// HIGHLIGHT MESH
	if (!imguiWindow.drawWindow && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

		imguiWindow.mouseLeftClick = true;
		imguiWindow.highlightedMesh = -1;
		glfwGetCursorPos(window, &imguiWindow.mouseX, &imguiWindow.mouseY);

	} else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		imguiWindow.mouseLeftClick = false;
	}

	// MOVEMENT (w, a, s, d)
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		Position += speed * Orientation;	// move position foward from orientation
		imguiWindow.currentSample = 0;
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		Position += speed * -glm::normalize(glm::cross(Orientation, Up)); // find the left vector from orientation and add to position
		imguiWindow.currentSample = 0;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		Position += speed * -Orientation;	// move position backward from orientation
		imguiWindow.currentSample = 0;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		Position += speed * glm::normalize(glm::cross(Orientation, Up));	// find the right vector from orientation and add to position
		imguiWindow.currentSample = 0;
	}



	// UP & DOWN (space, ctrl)
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		Position += speed * Up;
		imguiWindow.currentSample = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		Position += speed * -Up;
		imguiWindow.currentSample = 0;
	}

	// MOUSE MOVEMENT
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		if (firstClick) {
			glfwSetCursorPos(window, (width / 2), (height / 2));
			firstClick = false;
		}

		double mouseX;
		double mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
		float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

		glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));

		if (!((glm::angle(newOrientation, Up) <= glm::radians(5.0f)) || (glm::angle(newOrientation, -Up) <= glm::radians(5.0f)))) {

			Orientation = newOrientation;

		}

		Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

		glfwSetCursorPos(window, (width / 2), (height / 2));
		imguiWindow.currentSample = 0;

	} else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;
	}



	// SPEED UP (SHIFT)
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {

		if (!speedUp) {		// speedUp check prevents the speed from continuously increasing every frame
			speed *= 2;
			speedUp = true;
		}

	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {

		if (speedUp) {		// speedUp check prevents the speed from continuously decreasing every frame
			speed /= 2;
			speedUp = false;
		}

	}

}