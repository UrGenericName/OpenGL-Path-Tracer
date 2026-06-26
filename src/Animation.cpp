#pragma once

#include "Animation.h"

void Animation::cameraSpin(Camera& cam, unsigned int currentFrame) {

	cam.Position.x += 0.05;

}

void Animation::meshSpin(Mesh& mesh, unsigned int currentFrame) {

	mesh.rotation.z += (3.14159265 * 2) / 60;

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
		new AnimationMeshFunction{ "meshSpin", meshSpin },
		new AnimationMeshFunction{ "meshLightHue", meshLightHue }

	};

}