#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/color_space.hpp>

#include "Camera.h"
#include "Mesh.h"

namespace Animation {

	void cameraSpin(Camera& cam, unsigned int currentFrame);
	void meshSpin(Mesh& mesh, unsigned int currentFrame);
	void meshLightHue(Mesh& mesh, unsigned int currentFrame);

	// STRUCTS
	struct AnimationCameraFunction {

		string name;
		std::function<void(Camera&, unsigned int)> function;

	};

	struct AnimationMeshFunction {

		string name;
		std::function<void(Mesh&, unsigned int)> function;

	};

	// VECTORS
	extern vector<AnimationCameraFunction*> animationCameraFunctions;

	extern vector<AnimationMeshFunction*> animationMeshFunctions;

}