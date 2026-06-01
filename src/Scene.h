#pragma once

#include <vector>
#include <set>
#include "Mesh.h"

class Scene {
public:

	std::vector<Mesh*> meshCollection;
	Scene();
	~Scene();

	void Draw(Shader& shader, Camera& camera);
	void generateSSBOs(GLuint& vertexSSBO, GLuint& indicesSSBO, GLuint& meshTextureSSBO, GLuint& meshHeaderSSBO, GLuint& textureArray, std::vector<glm::vec4>& meshTexturesOutput);

};