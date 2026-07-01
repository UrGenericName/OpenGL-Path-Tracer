#pragma once

#include "Animation.h"

#include <glm/gtc/matrix_transform.hpp>

void Animation::cameraSpin(Camera& cam, unsigned int currentFrame) {

	static glm::vec3 initialPosition;
	if (currentFrame == 0) {
		initialPosition = glm::vec3(0.0f, -10.0f, 0.0f);
	}
	float angleDegrees = static_cast<float>(currentFrame) * (360 / 60);
	float angleRadians = glm::radians(angleDegrees);

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::rotate(modelMatrix, angleRadians, glm::vec3(0.0f, 0.0f, 1.0f));

	cam.Position = glm::vec3(modelMatrix * glm::vec4(initialPosition, 1.0f));

	cam.Orientation = -cam.Position;

}

void Animation::ballMove(Mesh& mesh, unsigned int currentFrame) {

	float offset = (mesh.position.x > 0) ? 0.0f : 60.0f;
	float time = (currentFrame + offset) / 120.0f;

	mesh.position.z = sin(time * 2.0f * 3.14159265f) * 3.5f + 2.0f;

}

void Animation::meshSpin(Mesh& mesh, unsigned int currentFrame) {

	mesh.rotation.z += (3.14159265 * 2) / 120;

}


void Animation::meshLightHue(Mesh& mesh, unsigned int currentFrame) {

	static std::unordered_map<Mesh*, float> initialHues;

	if (currentFrame == 0) {
		glm::vec3 hsvBase = glm::hsvColor(mesh.tint);
		initialHues[&mesh] = hsvBase.x;
	}

	float initialHue = initialHues[&mesh];

	float hue = fmodf((static_cast<float>(currentFrame) * 6.0f) + initialHue, 360.0f);

	glm::vec3 hsv(hue, 1.0f, 1.0f);
	mesh.tint = glm::rgbColor(hsv);

	if (currentFrame == 0 && initialHues.size() > 100) {
		initialHues.clear();
	}

}

namespace Animation {

	vector<AnimationCameraFunction*> animationCameraFunctions{

		new AnimationCameraFunction{ "Disabled", nullptr },
		new AnimationCameraFunction{ "camSpin", cameraSpin }

	};

	vector<AnimationMeshFunction*> animationMeshFunctions{

		new AnimationMeshFunction{ "Disabled", nullptr },
		new AnimationMeshFunction{ "ballMove", ballMove },
		new AnimationMeshFunction{ "meshSpin", meshSpin },
		new AnimationMeshFunction{ "meshLightHue", meshLightHue }

	};

}