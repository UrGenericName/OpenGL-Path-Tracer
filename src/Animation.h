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

		mesh.rotation.z += (3.14159265 * 2) / 60;

	}


	void meshLightHue(Mesh& mesh, unsigned int currentFrame) {
		// 1. Store the starting hue exactly once on the very first frame.
		// We use a static variable, but we key it by the mesh address so it works with multiple meshes.
		static std::unordered_map<Mesh*, float> initialHues;

		if (currentFrame == 0) {
			// Convert the starting tint to HSV and grab the initial hue angle
			glm::vec3 hsvBase = glm::hsvColor(mesh.tint);
			initialHues[&mesh] = hsvBase.x;
		}

		// 2. Get our base hue for this specific mesh (defaults to 0 if not found)
		float initialHue = initialHues[&mesh];

		// 3. Run your exact formula, adding the initial tint's hue as the starting offset
		float hue = fmodf((static_cast<float>(currentFrame) * 6.0f) + initialHue, 360.0f);

		// 4. Convert back to RGB and update the mesh
		glm::vec3 hsv(hue, 1.0f, 1.0f);
		mesh.tint = glm::rgbColor(hsv);

		// Cleanup map when the animation loop completely resets
		if (currentFrame == 0 && initialHues.size() > 100) {
			initialHues.clear(); // Prevents memory leaks in long sessions with deleted meshes
		}
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