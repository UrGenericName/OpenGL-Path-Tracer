#pragma once

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

	// VECTORS
	vector<AnimationCameraFunction*> animationCameraFunctions {

		new AnimationCameraFunction{ "Disabled", nullptr },
		new AnimationCameraFunction{ "camSpin", cameraSpin }

	};

	vector<AnimationMeshFunction*> animationMeshFunctions {

		new AnimationMeshFunction{ "Disabled", nullptr },
		new AnimationMeshFunction{ "meshSpin", meshSpin }

	};

}