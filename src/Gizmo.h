#pragma once

#include "Camera.h"
#include "Mesh.h"
#include <GLFW/glfw3.h>

class Gizmo {
public:

	enum class Selection {
		NONE,
		X,
		Y,
		Z
	};

	bool currentlyInUse = false;
	float gizmoDragSensitivity = 9.0f;

	Gizmo();

	void Draw(Shader& shader);
	void setPos(glm::vec3 positon);
	Selection getSelection(GLFWwindow* window, Camera& camera, bool visible);
	glm::vec3 newPointFromMouseDrag(glm::vec2 mouseStart, glm::vec2 mouseNew, Selection axis, Camera& camera);

private:

	const float axisLength = 2.0f;	// internal variable used to determine the threshold of when the gizmo should be selectable

	Selection lastSelection;

	Mesh* gizmoX{ new Mesh("models/core/gizmo_x.obj", glm::vec3(1.0f, 0.0f, 0.0f)) };
	Mesh* gizmoY{ new Mesh("models/core/gizmo_y.obj", glm::vec3(0.0f, 1.0f, 0.0f)) };
	Mesh* gizmoZ{ new Mesh("models/core/gizmo_z.obj", glm::vec3(0.0f, 0.0f, 1.0f)) };

	float distanceToLine2D(glm::vec2 point, glm::vec2 A, glm::vec2 B, bool finite, float* returnT = nullptr);
	float distance2D(glm::vec2 A, glm::vec2 B);
	glm::vec2 screenCoordinatesFromPos(Camera& camera, glm::vec3 position);

};