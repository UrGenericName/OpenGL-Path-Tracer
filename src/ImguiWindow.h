#pragma once

#include "imgui.h"
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

#define MAX_BOUNCES 64
#define MAX_SAMPLES 64

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
	bool debugLambertian = false;

	bool debugForceRoughness = false;
	float debugForceRoughnessAmount = 1.0f;

	int maxBounces = 1;
	int maxSamples = 1;

	unsigned int frame = 0;

	bool drawWindow = true;
};