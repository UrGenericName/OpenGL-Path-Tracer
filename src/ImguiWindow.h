#pragma once

#include "imgui.h"
#include "Mesh.h"
#include <vector>
#include <glm/glm.hpp>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <filesystem>

#include "DebugSettings.h"

#define MAX_BOUNCES 16
#define MAX_SAMPLES 5096

class Scene;	// Foward Deceleration

class ImguiWindow {
public:

	ImguiWindow();

	static void initImgui(GLFWwindow* window);
	void drawImgui(double frameTime, unsigned int animationFrame, Scene* scene, Mesh* highlightedMesh);	// defined in Scene.cpp

private:

	ImVec2 windowSize;
	ImVec2 windowPosition;

};