#pragma once

#include <vector>
#include "Mesh.h"

class Scene {
public:

	std::vector<Mesh*> meshCollection;
	Scene();
	~Scene();

	void Draw(Shader& shader, Camera& camera);
	GLuint createSSBO();
};