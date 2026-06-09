#pragma once

#include "ImguiWindow.h"

using namespace ImGui;

ImguiWindow::ImguiWindow() {}

void ImguiWindow::initImgui(GLFWwindow* window) {

	IMGUI_CHECKVERSION();
	CreateContext();
	ImGuiIO& io = GetIO();
	(void)io;
	StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");

}

void ImguiWindow::drawImgui() {

	if (!drawWindow) return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    NewFrame();

    Begin("Debug");

    Text("Render Visualization Mode:");
    Separator();

    if (BeginTable("ShaderLayoutTable", 2)) {

        TableNextColumn();
        RadioButton("Disabled", &debugMode, static_cast<int>(DebugTypes::DISABLED));
        RadioButton("Albedo", &debugMode, static_cast<int>(DebugTypes::ALBEDO));
        RadioButton("Normal", &debugMode, static_cast<int>(DebugTypes::NORMAL));
        RadioButton("Roughness", &debugMode, static_cast<int>(DebugTypes::ROUGHNESS));
        RadioButton("Metallic", &debugMode, static_cast<int>(DebugTypes::METALLIC));

        TableNextColumn();
        Checkbox("Lambertian Shading", &debugLambertian);
        SliderInt("Bounces", &debugBounces, 1, 10);
        SliderInt("Samples", &debugSamples, 1, 10);

        EndTable();
    }

    End();

    Render();
    ImGui_ImplOpenGL3_RenderDrawData( GetDrawData() );
}