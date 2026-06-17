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

void ImguiWindow::drawImgui(double frameTime) {

    // DEBUG WINDOW
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    NewFrame();

    if (IsKeyPressed(ImGuiKey_F)) { 
        drawWindow = !drawWindow; 
        currentSample = 0;
    }

    if (!drawWindow) { EndFrame();  return; }

    Begin("Debug");
    Separator();

    Text("Samples: %d/%d\t\tFT(ms): %.3f\t\tFPS: %d", currentSample, maxSamples, frameTime, static_cast<int>(1000 / frameTime));

    if (BeginTable("ShaderLayoutTable", 2)) {

        TableSetupColumn("Render Visualization");
        TableSetupColumn("Settings");
        TableHeadersRow();
        
        TableNextRow();
        
        // RENDER VISUALIZATION
        TableNextColumn();
        if (RadioButton("Disabled", &debugMode, static_cast<int>(DebugTypes::DISABLED))) currentSample = 0;
        if (RadioButton("Albedo", &debugMode, static_cast<int>(DebugTypes::ALBEDO))) currentSample = 0;
        if (RadioButton("Normal", &debugMode, static_cast<int>(DebugTypes::NORMAL))) currentSample = 0;
        if (RadioButton("Roughness", &debugMode, static_cast<int>(DebugTypes::ROUGHNESS))) currentSample = 0;
        if (RadioButton("Metallic", &debugMode, static_cast<int>(DebugTypes::METALLIC))) currentSample = 0;

        // SETTINGS
        TableNextColumn();
        Checkbox("Lambertian Shading", &debugLambertian);
        if (SliderInt("Bounces", &maxBounces, 1, MAX_BOUNCES)) currentSample = 0;
        if (SliderInt("Samples", &maxSamples, 1, MAX_SAMPLES)) currentSample = 0;
        SliderFloat("Min Brightness", &minBrightness, 0.0f, maxBrightness - 0.001f);
        SliderFloat("Max Brightness", &maxBrightness, minBrightness + 0.001f, 1.0f);

        EndTable();
    }

    if (BeginTable("ShaderLayoutTable", 2)) {

        TableSetupColumn("Misc");
        TableSetupColumn("");
        TableHeadersRow();

        TableNextRow();

        TableNextColumn();
        BeginDisabled(!debugUniversalRoughness);
        if (SliderFloat("Roughness", &debugUniversalRoughnessAmount, 0, 1)) currentSample = 0;
        EndDisabled();

        TableNextColumn();
        if (Checkbox("Universal Roughness", &debugUniversalRoughness)) currentSample = 0;
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