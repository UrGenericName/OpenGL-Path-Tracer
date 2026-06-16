#pragma once

#include <vector>
#include <set>
#include <random>
#include "ImguiWindow.h"
#include "Mesh.h"
#include "FBO.h"

#define COLOR_NOISE "textures/core/color_noise.png"

class Scene {
public:

	Camera& camera;

	std::vector<Mesh*> meshCollection;
	glm::vec3 backgroundColor = { 0.07f, 0.13f, 0.17f };

	unsigned int textureWidth;
	unsigned int textureHeight;

	Scene(Camera& i_camera, unsigned int width = 256, unsigned int height = 256);
	~Scene();

	void Draw(GLFWwindow* window);
	void link();

private:

	void generateSSBOs(unsigned int width, unsigned int height, GLuint& vertexSSBO, GLuint& indicesSSBO, GLuint& meshTextureSSBO, GLuint& meshHeaderSSBO, GLuint& textureArray, std::vector<glm::vec4>& meshTexturesOutput);
	void generateUniforms(Shader& shader, Camera& camera);

	Shader* pathTracer;
	Shader* accumulationPass;

	FBO* frameBuffer;
	FBO* accumulationBuffer;

	Texture* colorNoise;
	ImguiWindow imguiWindow;

	// Cached data
	std::vector<glm::vec4> meshTexturesOutput;
	GLuint vertexSSBO, indicesSSBO, textureMeshSSBO, meshHeaderSSBO, textureArray;

	// Random generators
	inline static std::random_device m_rd;
	inline static std::minstd_rand0 m_gen{ m_rd() };
	inline static std::uniform_int_distribution<uint32_t> m_distrib{ 0, (std::numeric_limits<uint32_t>::max)() };

};