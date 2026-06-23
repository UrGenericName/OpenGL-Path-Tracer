#pragma once

#include "Gizmo.h"
#include <iostream>
#include <algorithm>

Gizmo::Gizmo() {}

void Gizmo::Draw(Shader& shader) {

	gizmoX->DrawGizmo(shader, 1);
	gizmoY->DrawGizmo(shader, 2);
	gizmoZ->DrawGizmo(shader, 3);

}

void Gizmo::setPos(glm::vec3 position) {

	gizmoX->position = position;
	gizmoY->position = position;
	gizmoZ->position = position;

}

Gizmo::Selection Gizmo::getSelection(GLFWwindow* window, Camera& camera) {

	glm::vec3 position = gizmoX->position;

	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	glm::vec2 mouseCoords(mouseX / camera.width, mouseY / camera.height);

	const float axisLength = 2.0f;
	glm::vec2 gizmoXCoords = screenCoordinatesFromPos(camera, glm::vec3(position.x + axisLength, position.y, position.z));
	glm::vec2 gizmoYCoords = screenCoordinatesFromPos(camera, glm::vec3(position.x, position.y + axisLength, position.z));
	glm::vec2 gizmoZCoords = screenCoordinatesFromPos(camera, glm::vec3(position.x, position.y, position.z + axisLength));

	glm::vec2 gizmoOriginCoords = screenCoordinatesFromPos(camera, glm::vec3(position.x, position.y, position.z));

	// Exit if not within selection threshold
	float selectionThreshold = max({ distance2D(gizmoOriginCoords, gizmoXCoords), distance2D(gizmoOriginCoords, gizmoYCoords) , distance2D(gizmoOriginCoords, gizmoZCoords) });
	if (distance2D(mouseCoords, gizmoOriginCoords) > selectionThreshold) return Selection::NONE;

	float distanceSelectionX = distanceToLine2D(mouseCoords, gizmoOriginCoords, gizmoXCoords, true);
	float distanceSelectionY = distanceToLine2D(mouseCoords, gizmoOriginCoords, gizmoYCoords, true);
	float distanceSelectionZ = distanceToLine2D(mouseCoords, gizmoOriginCoords, gizmoZCoords, true);

	if (distanceSelectionX < distanceSelectionY && distanceSelectionX < distanceSelectionZ) {

		// X AXIS IS CLOSEST
		return Selection::X;

	}
	else if (distanceSelectionY < distanceSelectionX && distanceSelectionY < distanceSelectionZ) {

		// Y AXIS IS CLOSEST
		return Selection::Y;

	}
	else {

		// Z AXIS IS CLOSEST
		return Selection::Z;

	}

}

float Gizmo::distanceToLine2D(glm::vec2 point, glm::vec2 A, glm::vec2 B, bool finite, float* returnT) {

	glm::vec2 lineDir = glm::normalize(B - A);
	glm::vec2 toPoint = point - A;

	float t = glm::dot(toPoint, lineDir) / glm::dot(lineDir, lineDir);

	if (finite) t = clamp(t, 0.0f, 1.0f);

	glm::vec2 closestPoint = lineDir * t + A;

	if (returnT != nullptr) *returnT = t;

	return distance2D(closestPoint, point);

}

float Gizmo::distance2D(glm::vec2 A, glm::vec2 B) {
	return sqrt(pow(A.x - B.x, 2) + pow(A.y - B.y, 2));
}

glm::vec2 Gizmo::screenCoordinatesFromPos(Camera& camera, glm::vec3 position) {

	glm::vec4 screenCoordinates = camera.getMatrix() * glm::vec4(position, 1.0f);

	// screen coordinates that range from 0 to 1
	float screenX = ((screenCoordinates.x / screenCoordinates.w) + 1.0f) / 2.0f;
	float screenY = (-(screenCoordinates.y / screenCoordinates.w) + 1.0f) / 2.0f;

	return glm::vec2(screenX, screenY);

}