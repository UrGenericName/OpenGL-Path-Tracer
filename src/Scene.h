#pragma once

#include <vector>
#include <string>
#include "Mesh.h"
#include "Camera.h"
#include "SSBOcomponent.h"
#include "Rendercomponent.h"
#include "ShaderPipelineComponent.h"
#include "DebugSettings.h"

#define SCENE_FILE_EXTENSION ".txt"

class Scene {
public:

	DebugSettings debugSettings;

	Camera& camera;

	unsigned int textureWidth;
	unsigned int textureHeight;

	vector<Mesh*> meshCollection;
	glm::vec3 backgroundColor = { 0.17f, 0.17f, 0.17f };

	SSBOcomponent SSBOcomponent;
	RenderComponent renderComponent;
	ShaderPipelineComponent shaderPipelineComponent;

	Scene(Camera& i_camera, unsigned int width = 256, unsigned int height = 256);
	Scene(Camera& i_camera, string fileName, unsigned int width = 256, unsigned int height = 256);
	~Scene();

	double getFrameTime();
	void importScene(string fileName);
	void exportScene(string fileName);
	void Draw(GLFWwindow* window);
	void link();

private:

	double frameTime;

	void Inputs(GLFWwindow* window);
	void setWindowTitle(GLFWwindow* window, double frameTime);

};