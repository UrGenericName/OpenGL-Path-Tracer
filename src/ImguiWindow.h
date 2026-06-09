#pragma once

#include "imgui.h"
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

#define MAX_BOUNCES 16
#define MAX_SAMPLES 16

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
	int debugBounces = 1;
	int debugSamples = 1;
	bool debugLambertian = false;

	bool debugForceRoughness = false;
	float debugForceRoughnessAmount = 1.0f;

	bool drawWindow = true;
};