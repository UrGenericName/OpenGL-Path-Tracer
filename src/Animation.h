#pragma once

#include <glm/gtx/color_space.hpp>

#include "Camera.h"
#include "Mesh.h"

namespace Animation {

	// STRUCTS
	struct AnimationCameraFunction {

		string name;
		std::function<void(Camera&, unsigned int)> function;

	};

	struct AnimationMeshFunction {

		string name;
		std::function<void(Mesh&, unsigned int)> function;

	};

	// FUNCTIONS
	void cameraSpin(Camera& cam, unsigned int currentFrame) {

		cam.Position.x += 0.05;

	}

	void meshSpin(Mesh& mesh, unsigned int currentFrame) {

		mesh.rotation.z += 0.05;

	}


	void meshLightHue(Mesh& mesh, unsigned int currentFrame) {
		// 1. Calculate hue shift (0.0 to 360.0 degrees)
		// Adjust the multiplier (1.0f) to speed up or slow down the shift
		float hue = fmod(static_cast<float>(currentFrame) * 6.0f, 360.0f);

		// 2. Define Saturation and Value (Brightness) at maximum (0.0 to 1.0)
		glm::vec3 hsv(hue, 1.0f, 1.0f);

		// 3. Convert HSV to RGB and assign to mesh tint
		mesh.tint = glm::rgbColor(hsv);

	}

	// VECTORS
	vector<AnimationCameraFunction*> animationCameraFunctions {

		new AnimationCameraFunction{ "Disabled", nullptr },
		new AnimationCameraFunction{ "camSpin", cameraSpin }

	};

	vector<AnimationMeshFunction*> animationMeshFunctions {

		new AnimationMeshFunction{ "Disabled", nullptr },
		new AnimationMeshFunction{ "meshSpin", meshSpin },
		new AnimationMeshFunction{ "meshLightHue", meshLightHue }

	};

}