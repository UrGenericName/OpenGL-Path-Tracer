#pragma once

#include "imgui.h"
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

#define MAX_BOUNCES 10
#define MAX_SAMPLES 10

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
	int debugBounces = 1;
	int debugSamples = 1;

	bool drawWindow = false;
};