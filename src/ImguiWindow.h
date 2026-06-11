#pragma once

#include "imgui.h"
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

#define MAX_BOUNCES 256
#define MAX_SAMPLES 5096

class ImguiWindow {
public:

	ImguiWindow();

	static void initImgui(GLFWwindow* window);
	void drawImgui();

	enum class DebugTypes {

		DISABLED,
		ALBEDO,
		NORMAL,
		ROUGHNESS,
		METALLIC

	};

	int debugMode = static_cast<int>(DebugTypes::DISABLED);
	bool debugLambertian = true;

	bool debugForceRoughness = false;
	float debugForceRoughnessAmount = 1.0f;

	int maxBounces = 8;
	int maxSamples = 32;

	unsigned int currentSample = 0;

	bool drawWindow = true;

private:

	ImVec2 windowSize;
	ImVec2 windowPosition;

};