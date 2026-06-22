#pragma once

#include "Camera.h"
#include "Mesh.h"

namespace Animation {

	void meshSpin(Mesh& mesh, unsigned int currentFrame) {

		double t = static_cast<double>(currentFrame) / 100;

		mesh.position = glm::vec3( (1 + cos(t)), (sin(t)), (2 * sin(t / 2)) );

	}

}