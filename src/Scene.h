#pragma once

#include <chrono>
#include <vector>
#include <set>
#include <random>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <format>
#include "ImguiWindow.h"
#include "Mesh.h"
#include "FBO.h"
#include "Camera.h"

#define COLOR_NOISE "textures/core/color_noise.png"
#define SCENE_FILE_EXTENSION ".txt"

class Scene {
public:

	friend ImguiWindow;

	Camera& camera;

	unsigned int textureWidth;
	unsigned int textureHeight;

	std::vector<Mesh*> meshCollection;
	glm::vec3 backgroundColor = { 0.07f, 0.13f, 0.17f };

	Scene(Camera& i_camera, unsigned int width = 256, unsigned int height = 256);
	Scene(Camera& i_camera, string fileName, unsigned int width = 256, unsigned int height = 256);
	~Scene();

	void importScene(string fileName);
	void exportScene(string fileName);
	void Draw(GLFWwindow* window);
	void link();

private:

	Shader* depthPrepassShader;
	Shader* pathTracingShader;
	Shader* postProcessingShader;

	FBO* frameBuffer;
	FBO* accumulationBuffer;

	GLuint highlightedMeshBuffer;

	Texture* colorNoise;
	ImguiWindow imguiWindow;

	GLuint vertexSSBO, indicesSSBO, meshTextureSSBO, meshHeaderSSBO, textureArray;

	vector<Vertex> globalVertices;
	vector<GLuint> globalIndices;
	vector<glm::vec4> meshTextures; // <albedoIndex, normalIndex, roughnessIndex, metallicIndex>
	vector<glm::vec4> meshHeader;	// <indicesStartPointer, indicesSize, emissiveValue>

	set<string> texturePool;	// all the textures used in the scene

	Mesh* gizmoX { new Mesh("models/core/gizmo_x.obj", glm::vec3(1.0f, 0.0f, 0.0f)) };
	Mesh* gizmoY { new Mesh("models/core/gizmo_y.obj", glm::vec3(0.0f, 1.0f, 0.0f)) };
	Mesh* gizmoZ { new Mesh("models/core/gizmo_z.obj", glm::vec3(0.0f, 0.0f, 1.0f)) };

	void generateGlobalVertices();
	void generateGlobalIndices();
	void generateMeshTextures();
	void generateMeshHeader();

	void updateVertexSSBO();
	void updateIndicesSSBO();
	void updateMeshTexturesSSBO();
	void updateMeshHeaderSSBO();

	void generateSSBOs(unsigned int width, unsigned int height);

	void generateDepthUniforms(Shader& shader, Camera& camera);
	void generatePathTracingUniforms(Shader& shader, Camera& camera);
	void generatePostProcessingUniforms(Shader& shader, Camera& camera);

	void Draw_DepthPrepass(Shader& Depth_shader);
	void Draw_PathTracingPass(Shader& PathTracing_shader);
	void Draw_PostProcessingPass(Shader& PostProcessing_shader);

	// Random generators
	inline static std::random_device m_rd;
	inline static std::minstd_rand0 m_gen{ m_rd() };
	inline static std::uniform_int_distribution<uint32_t> m_distrib{ 0, (std::numeric_limits<uint32_t>::max)() };

};