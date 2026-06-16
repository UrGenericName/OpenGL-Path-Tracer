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

    // DEBUG WINDOW
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    NewFrame();

    if (IsKeyPressed(ImGuiKey_F)) { drawWindow = !drawWindow; }

    if (!drawWindow) { EndFrame();  return; }

    Begin("Debug");
    Separator();

    Text("Samples: %d/%d", currentSample, maxSamples);

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
        if (SliderInt("Bounces", &maxBounces, 1, MAX_BOUNCES)) currentSample = 0;
        if (SliderInt("Samples", &maxSamples, 1, MAX_SAMPLES)) currentSample = 0;

        EndTable();
    }

    if (BeginTable("ShaderLayoutTable", 2)) {

        TableSetupColumn("Misc");
        TableSetupColumn("");
        TableHeadersRow();

        TableNextRow();

        TableNextColumn();
        BeginDisabled(!debugForceRoughness);
        if (SliderFloat("Roughness", &debugForceRoughnessAmount, 0, 1)) currentSample = 0;
        EndDisabled();

        TableNextColumn();
        Checkbox("Force Roughness", &debugForceRoughness);

        EndTable();

    }

    if (Button("Clear Samples")) currentSample = 0;
    if (Button("Pause")) pause = !pause;

    // clear samples if window is moved or resized
    if (windowSize.x != GetWindowSize().x || windowSize.y != GetWindowSize().y) currentSample = 0;
    if (windowPosition.x != GetWindowPos().x || windowPosition.y != GetWindowPos().y) currentSample = 0;

    windowSize = GetWindowSize();
    windowPosition = GetWindowPos();



    End();

    Render();
    ImGui_ImplOpenGL3_RenderDrawData( GetDrawData() );

}