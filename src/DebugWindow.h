#pragma once

#include "imgui.h"
#include "Mesh.h"
#include <vector>
#include <glm/glm.hpp>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include "Scene.h"
#include <chrono>
#include <fstream>
#include <filesystem>
#include <format>
#include "Animation.h"

#include "DebugSettings.h"

#define MAX_BOUNCES 16
#define MAX_SAMPLES 5096

class DebugWindow {
public:

	DebugWindow(GLFWwindow* window);
	~DebugWindow();
	void drawImgui(Scene& scene);

private:

	void initImgui(GLFWwindow* window);

	void drawRenderTab(Scene& scene);
	void drawRenderVisualizationTab(Scene& scene);
	void drawSceneSettingsTab(Scene& scene);
	void drawCameraSettingsTab(Scene& scene);
	void drawMeshSettingsTab(Scene& scene);
	void drawMeshMaterialsTab(Scene& scene);
	void drawMiscTab(Scene& scene);

	ImVec2 windowSize;
	ImVec2 windowPosition;

};