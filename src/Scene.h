#pragma once

#include <vector>
#include <set>
#include "Mesh.h"

class Scene {
public:

	std::vector<Mesh*> meshCollection;
	glm::vec3 backgroundColor = { 0.07f, 0.13f, 0.17f };

	unsigned int textureWidth;
	unsigned int textureHeight;

	Scene(unsigned int width = 256, unsigned int height = 256);
	~Scene();

	void Draw(Shader& shader, Camera& camera, GLFWwindow* window);
	void generateSSBOs(unsigned int width, unsigned int height, GLuint& vertexSSBO, GLuint& indicesSSBO, GLuint& meshTextureSSBO, GLuint& meshHeaderSSBO, GLuint& textureArray, std::vector<glm::vec4>& meshTexturesOutput);
	void link(Shader& shader);

private:

	// Cached data
	std::vector<glm::vec4> meshTexturesOutput;
	GLuint vertexSSBO, indicesSSBO, textureMeshSSBO, meshHeaderSSBO, textureArray;

};