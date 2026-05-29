#pragma once

#include <vector>
#include "Mesh.h"

class Scene {
public:

	std::vector<Mesh*> meshCollection;
	Scene();
	~Scene();

	void Draw(Shader& shader, Camera& camera);
	void generateSSBOs(GLuint& vertexSSBO, GLuint& indicesSSBO, GLuint& meshHeaderSSBO);
};