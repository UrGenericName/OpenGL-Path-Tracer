#pragma once

#include <vector>
#include <set>
#include <random>
#include "ImguiWindow.h"
#include "Mesh.h"

#define COLOR_NOISE "textures/core/color_noise.png"

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

	Texture* colorNoise;
	ImguiWindow imguiWindow;

	inline static std::random_device m_rd;
	inline static std::minstd_rand0 m_gen{ m_rd() };
	inline static std::uniform_int_distribution<uint32_t> m_distrib{ 0, (std::numeric_limits<uint32_t>::max)() };

};