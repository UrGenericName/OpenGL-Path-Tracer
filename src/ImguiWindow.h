#pragma once

#include "imgui.h"
#include "Mesh.h"
#include <vector>
#include <glm/glm.hpp>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <filesystem>

#define MAX_BOUNCES 256
#define MAX_SAMPLES 5096

class Scene;	// Foward Deceleration

class ImguiWindow {
public:

	ImguiWindow();

	static void initImgui(GLFWwindow* window);
	void drawImgui(double frameTime, Scene* scene, Mesh* highlightedMesh);	// defined in Scene.cpp

	enum class DebugTypes {

		DISABLED,
		ALBEDO,
		NORMAL,
		ROUGHNESS,
		METALLIC

	};

	// DEBUG
	int debugMode = static_cast<int>(DebugTypes::DISABLED);
	bool debugLambertian = true;
	bool debugUniversalRoughness = false;
	float debugUniversalRoughnessAmount = 1.0f;

	// PATH-TRACING
	int maxBounces = 8;
	int maxSamples = 32;
	float minBrightness = 0.0f;
	float maxBrightness = 0.5f;
	unsigned int currentSample = 0;

	// HIGHLIGHTED MESH
	int highlightedMesh = -1;
	bool mouseLeftClick = false;
	double mouseX;
	double mouseY;

	// IMPORT / EXPORT
	inline static char importName[128] = "scene.txt";
	inline static char exportName[128] = "";
	inline static char importOBJname[128] = "";

	// WINDOW
	bool pause = false;
	bool drawWindow = true;

private:

	ImVec2 windowSize;
	ImVec2 windowPosition;

};