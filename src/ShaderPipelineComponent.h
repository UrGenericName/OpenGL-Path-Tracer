#pragma once

#include "ShaderClass.h"
#include "Camera.h"
#include "FBO.h"
#include "Gizmo.h"
#include "DebugSettings.h"
#include <random>

#define COLOR_NOISE "textures/core/color_noise.png"

class ShaderPipelineComponent {
public:

	Shader* depthPrepassShader;
	Shader* pathTracingShader;
	Shader* postProcessingShader;

	FBO* frameBuffer;
	FBO* accumulationBuffer;

	GLuint highlightedMeshBuffer;

	Texture* colorNoise;

	Gizmo gizmo;

	~ShaderPipelineComponent();

	void link(DebugSettings& debugSettings, Camera& camera, glm::vec3& backgroundColor);

	void Draw_DepthPrepass(Camera& camera, vector<Mesh*>& meshCollection, vector<MeshTextures>& meshTextures, DebugSettings& debugSettings);
	void Draw_PathTracingPass(Camera& camera, vector<Mesh*>& meshCollection, vector<MeshTextures>& meshTextures, DebugSettings& debugSettings);
	void Draw_PostProcessingPass(Camera& camera, vector<Mesh*>& meshCollection, vector<MeshTextures>& meshTextures, GLFWwindow* window, DebugSettings& debugSettings);

private:

	void generateDepthUniforms(Shader& shader, Camera& camera, DebugSettings& debugSettings);
	void generatePathTracingUniforms(Shader& shader, Camera& camera, DebugSettings& debugSettings);
	void generatePostProcessingUniforms(Shader& shader, Camera& camera, GLFWwindow* window, DebugSettings& debugSettings);

	// Random generators
	inline static random_device m_rd;
	inline static minstd_rand0 m_gen{ m_rd() };
	inline static uniform_int_distribution<uint32_t> m_distrib{ 0, (numeric_limits<uint32_t>::max)() };

};