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
    Separator();

    Text("Frame: %d", frame);

    if (BeginTable("ShaderLayoutTable", 2)) {

        TableSetupColumn("Render Visualization");
        TableSetupColumn("Settings");
        TableHeadersRow();
        
        TableNextRow();
        
        // RENDER VISUALIZATION
        TableNextColumn();
        RadioButton("Disabled", &debugMode, static_cast<int>(DebugTypes::DISABLED));
        RadioButton("Albedo", &debugMode, static_cast<int>(DebugTypes::ALBEDO));
        RadioButton("Normal", &debugMode, static_cast<int>(DebugTypes::NORMAL));
        RadioButton("Roughness", &debugMode, static_cast<int>(DebugTypes::ROUGHNESS));
        RadioButton("Metallic", &debugMode, static_cast<int>(DebugTypes::METALLIC));

        // SETTINGS
        TableNextColumn();
        Checkbox("Lambertian Shading", &debugLambertian);
        SliderInt("Bounces", &maxBounces, 1, MAX_BOUNCES);
        SliderInt("Samples", &maxSamples, 1, MAX_SAMPLES);

        EndTable();
    }

    if (BeginTable("ShaderLayoutTable", 2)) {

        TableSetupColumn("Misc");
        TableSetupColumn("");
        TableHeadersRow();

        TableNextRow();

        TableNextColumn();
        BeginDisabled(!debugForceRoughness);
        SliderFloat("Roughness", &debugForceRoughnessAmount, 0, 1);
        EndDisabled();

        TableNextColumn();
        Checkbox("Force Roughness", &debugForceRoughness);
        EndTable();

    }

    if (Button("Clear Frames")) frame = 0;

    End();

    Render();
    ImGui_ImplOpenGL3_RenderDrawData( GetDrawData() );
}