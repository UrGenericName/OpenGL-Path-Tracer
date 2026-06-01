#pragma once

#include <vector>
#include <set>
#include "Mesh.h"

class Scene {
public:

	unsigned int textureWidth;
	unsigned int textureHeight;

	std::vector<Mesh*> meshCollection;
	Scene(unsigned int width = 256, unsigned int height = 256);
	~Scene();

	void Draw(Shader& shader, Camera& camera);
	void generateSSBOs(unsigned int width, unsigned int height, GLuint& vertexSSBO, GLuint& indicesSSBO, GLuint& meshTextureSSBO, GLuint& meshHeaderSSBO, GLuint& textureArray, std::vector<glm::vec4>& meshTexturesOutput);

};