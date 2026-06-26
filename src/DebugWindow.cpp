#pragma once

#include "DebugWindow.h"

using namespace ImGui;

DebugWindow::DebugWindow(GLFWwindow* window) { initImgui(window); }

DebugWindow::~DebugWindow() {

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

}

void DebugWindow::initImgui(GLFWwindow* window) {

	IMGUI_CHECKVERSION();
	CreateContext();
	ImGuiIO& io = GetIO();
	(void)io;
	StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");

}

void DebugWindow::drawImgui(Scene& scene) {

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	NewFrame();

	if (IsKeyPressed(ImGuiKey_Tab)) {
		scene.debugSettings.drawWindow = !scene.debugSettings.drawWindow;
		scene.debugSettings.currentSample = 0;
	}

	if (!scene.debugSettings.drawWindow) { EndFrame();  return; }

	scene.debugSettings.usingDebugWindow = GetIO().WantCaptureMouse;


	// DRAW WINDOW
	Begin("Debug");
	Separator();

	double frameTime = scene.getFrameTime();
	Text("Samples: %d/%d\t\tFT(ms): %.3f\t\tFPS: %d", scene.debugSettings.currentSample, scene.debugSettings.maxSamples, frameTime, static_cast<int>(1000 / frameTime));

	drawRenderTab(scene);
	drawRenderVisualizationTab(scene);
	drawSceneSettingsTab(scene);
	drawCameraSettingsTab(scene);
	drawMeshSettingsTab(scene);
	drawMiscTab(scene);

	End();

	// clear samples if window is moved or resized
	if (windowSize.x != GetWindowSize().x || windowSize.y != GetWindowSize().y) scene.debugSettings.currentSample = 0;
	if (windowPosition.x != GetWindowPos().x || windowPosition.y != GetWindowPos().y) scene.debugSettings.currentSample = 0;

	windowSize = GetWindowSize();
	windowPosition = GetWindowPos();

	Render();
	ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());

}

void DebugWindow::drawRenderTab(Scene& scene) {

	const ImVec4 validPathColor(0.1f, 0.3f, 0.1f, 1.0f);
	const ImVec4 invalidPathColor(0.15f, 0.15f, 0.15f, 1.0f);

	if (BeginTable("ShaderLayoutTable", 2)) {

		TableSetupColumn("Render");
		TableSetupColumn("Render Settings");
		TableHeadersRow();

		TableNextRow();

		BeginDisabled(scene.debugSettings.previewAnimationPhase != DebugSettings::RenderPhase::COMPLETE);

		// RENDER
		TableNextColumn();
		BeginDisabled(scene.debugSettings.imageRenderPhase != DebugSettings::RenderPhase::COMPLETE);
		if (Button("Render Image")) {
			scene.debugSettings.imageRenderPhase = DebugSettings::RenderPhase::WAITING;
		}
		EndDisabled();
		BeginDisabled(scene.debugSettings.videoRenderPhase != DebugSettings::RenderPhase::COMPLETE);
		SameLine();
		if (Button("Render Video")) {
			scene.debugSettings.videoRenderPhase = DebugSettings::RenderPhase::WAITING;
			scene.renderComponent.animationFrame = 0;
		}

		SliderInt("Frames", &scene.debugSettings.totalAnimationFrames, 1, 1000);
		EndDisabled();

		if (Button("Preview")) {
			scene.debugSettings.previewAnimationPhase = DebugSettings::RenderPhase::WAITING;
			scene.debugSettings.debugMode = static_cast<int>(DebugSettings::DebugTypes::ALBEDO);
		}
		SameLine();
		Text("Frame: %d/%d", scene.renderComponent.animationFrame, scene.debugSettings.totalAnimationFrames);

		// RENDER SETTINGS
		TableNextColumn();
		if (SliderInt("Bounces", &scene.debugSettings.maxBounces, 1, MAX_BOUNCES)) scene.debugSettings.currentSample = 0;
		if (SliderInt("Samples", &scene.debugSettings.maxSamples, 1, MAX_SAMPLES)) scene.debugSettings.currentSample = 0;
		if (Button("Clear Samples")) scene.debugSettings.currentSample = 0;
		Spacing();
		if (Button("Pause")) scene.debugSettings.pause = !scene.debugSettings.pause;

		EndDisabled();

		EndTable();
	}


}

void DebugWindow::drawRenderVisualizationTab(Scene& scene) {

	if (BeginTable("ShaderLayoutTable", 2)) {

		TableSetupColumn("Render Visualization");
		TableSetupColumn("Post Processing");
		TableHeadersRow();

		TableNextRow();

		// RENDER VISUALIZATION
		TableNextColumn();
		BeginDisabled(scene.debugSettings.previewAnimationPhase != DebugSettings::RenderPhase::COMPLETE);
		if (RadioButton("Disabled", &scene.debugSettings.debugMode, static_cast<int>(DebugSettings::DebugTypes::DISABLED))) scene.debugSettings.currentSample = 0;
		Spacing();
		if (RadioButton("Albedo", &scene.debugSettings.debugMode, static_cast<int>(DebugSettings::DebugTypes::ALBEDO))) scene.debugSettings.currentSample = 0;
		if (RadioButton("Normal", &scene.debugSettings.debugMode, static_cast<int>(DebugSettings::DebugTypes::NORMAL))) scene.debugSettings.currentSample = 0;
		if (RadioButton("Roughness", &scene.debugSettings.debugMode, static_cast<int>(DebugSettings::DebugTypes::ROUGHNESS))) scene.debugSettings.currentSample = 0;
		if (RadioButton("Metallic", &scene.debugSettings.debugMode, static_cast<int>(DebugSettings::DebugTypes::METALLIC))) scene.debugSettings.currentSample = 0;
		Spacing();
		if (RadioButton("VertexNormal", &scene.debugSettings.debugMode, static_cast<int>(DebugSettings::DebugTypes::VERTEX_NORMAL))) scene.debugSettings.currentSample = 0;
		EndDisabled();
		Spacing();
		Checkbox("Lambertian Shading", &scene.debugSettings.debugLambertian);

		// POST PROCESSING
		TableNextColumn();
		SliderFloat("Min Brightness", &scene.debugSettings.minBrightness, 0.0f, scene.debugSettings.maxBrightness - 0.001f);
		SliderFloat("Max Brightness", &scene.debugSettings.maxBrightness, scene.debugSettings.minBrightness + 0.001f, 1.0f);

		EndTable();
	}

}

void DebugWindow::drawSceneSettingsTab(Scene& scene) {

	const ImVec4 validPathColor(0.1f, 0.3f, 0.1f, 1.0f);
	const ImVec4 invalidPathColor(0.15f, 0.15f, 0.15f, 1.0f);

	if (BeginTable("ShaderLayoutTable", 3)) {

		TableSetupColumn("Scene");
		TableHeadersRow();

		TableNextRow();

		std::filesystem::path filePath;

		TableNextColumn();

		filePath = "scenes/" + string(scene.debugSettings.importName);
		bool isImportValid = (std::filesystem::exists(filePath) && filePath.extension().string() == SCENE_FILE_EXTENSION);
		PushStyleColor(ImGuiCol_FrameBg, (isImportValid ? validPathColor : invalidPathColor));
		InputTextWithHint("##importFileName", "scene.txt", scene.debugSettings.importName, IM_ARRAYSIZE(scene.debugSettings.importName));
		PopStyleColor(1);
		TableNextColumn();

		filePath = "scenes/" + string(scene.debugSettings.exportName);
		bool isExportValid = (filePath.extension().string() == SCENE_FILE_EXTENSION);
		PushStyleColor(ImGuiCol_FrameBg, (isExportValid ? validPathColor : invalidPathColor));
		InputTextWithHint("##exportFileName", "scene.txt", scene.debugSettings.exportName, IM_ARRAYSIZE(scene.debugSettings.exportName));
		PopStyleColor(1);
		TableNextColumn();

		filePath = scene.debugSettings.importOBJname;
		bool isOBJValid = (std::filesystem::exists(filePath) && filePath.extension().string() == ".obj");
		PushStyleColor(ImGuiCol_FrameBg, (isOBJValid ? validPathColor : invalidPathColor));
		InputTextWithHint("##importOBJName", "models/cube.obj", scene.debugSettings.importOBJname, IM_ARRAYSIZE(scene.debugSettings.importOBJname));
		PopStyleColor(1);

		TableNextRow();
		TableNextColumn();
		BeginDisabled(!isImportValid);
		if (Button("Import Scene")) {
			scene.importScene(scene.debugSettings.importName);
			scene.link();
			strcpy(scene.debugSettings.exportName, scene.debugSettings.importName);
			scene.debugSettings.importName[0] = 0x00;
		}
		EndDisabled();

		TableNextColumn();
		BeginDisabled(!isExportValid);
		if (Button("Export Scene")) {
			scene.exportScene(scene.debugSettings.exportName);
			scene.debugSettings.exportName[0] = 0x00;
		}
		EndDisabled();

		TableNextColumn();
		BeginDisabled(!isOBJValid);
		if (Button("Import OBJ")) {

			Mesh* mesh = new Mesh(scene.debugSettings.importOBJname);
			scene.meshCollection.push_back(mesh);
			scene.link();
			scene.debugSettings.importOBJname[0] = 0x00;

		}
		EndDisabled();

		EndTable();
	}

}

void DebugWindow::drawCameraSettingsTab(Scene& scene) {

	if (BeginTable("ShaderLayoutTable", 5)) {

		TableSetupColumn("Camera");
		TableSetupColumn("   X");
		TableSetupColumn("   Y");
		TableSetupColumn("   Z");
		TableHeadersRow();

		TableNextRow();
		TableNextColumn();
		Text("Position");
		TableNextColumn();
		{
			PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
			if (DragFloat("##posX", &(scene.camera.Position.x), 0.5f)) scene.debugSettings.currentSample = 0;
			PopStyleColor(1);
			TableNextColumn();
			PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
			if (DragFloat("##posY", &(scene.camera.Position.y), 0.5f)) scene.debugSettings.currentSample = 0;
			PopStyleColor(1);
			TableNextColumn();
			PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
			if (DragFloat("##posZ", &(scene.camera.Position.z), 0.5f)) scene.debugSettings.currentSample = 0;
			PopStyleColor(1);
			TableNextColumn();
			if (Button("Reset##positon")) scene.camera.Position = glm::vec3(0.0f);
		}

		TableNextRow();
		TableNextColumn();
		Text("Animation");
		TableNextColumn();

		static int current_selection;

		// some fucked up shit to get the current_selection of the camera animation
		current_selection = 0;
		auto highlighedCameraAnimation = (scene.camera.animation.target<void(*)(Camera&, unsigned int)>());
		auto rawHighlightedCameraAnimation = highlighedCameraAnimation ? (void*)*(highlighedCameraAnimation) : nullptr;

		for (int i = 0; i < Animation::animationCameraFunctions.size(); ++i) {
			auto currentCameraAnimation = (Animation::animationCameraFunctions[i]->function.target<void(*)(Camera&, unsigned int)>());
			auto rawCurrentCameraAnimation = currentCameraAnimation ? (void*)*(currentCameraAnimation) : nullptr;

			if (rawHighlightedCameraAnimation == rawCurrentCameraAnimation) {
				current_selection = i;
				break;
			}

		}

		const char* current_selection_name = Animation::animationCameraFunctions[current_selection]->name.c_str();

		// Create drop down component
		if (BeginCombo("", current_selection_name)) {

			for (int i = 0; i < Animation::animationCameraFunctions.size(); ++i) {

				const bool is_selected = (current_selection == i);

				// Assign new selected animation to highlighted mesh
				if (Selectable(Animation::animationCameraFunctions[i]->name.c_str(), is_selected)) {
					current_selection = i;
					scene.camera.animation = Animation::animationCameraFunctions[i]->function;
				}

				if (is_selected) SetItemDefaultFocus();

			}

			EndCombo();
		}

		TableNextRow();
		TableNextColumn();
		Text("Settings");
		TableNextColumn();

		if (SliderFloat("FOV", &(scene.camera.FOVdeg), 0.0f, 90.0f)) scene.debugSettings.currentSample = 0;
		TableNextColumn();
		if (SliderFloat("Near Plane", &(scene.camera.nearPlane), 0.0f, scene.camera.farPlane)) scene.debugSettings.currentSample = 0;
		TableNextColumn();
		if (SliderFloat("Far Plane", &(scene.camera.farPlane), scene.camera.nearPlane, 10000.0f)) scene.debugSettings.currentSample = 0;

		EndTable();
	}

}

void DebugWindow::drawMeshSettingsTab(Scene& scene) {

	Mesh* highlightedMesh = (scene.debugSettings.highlightedMeshIndex == -1) ? nullptr : scene.meshCollection[scene.debugSettings.highlightedMeshIndex];

	if (highlightedMesh != nullptr) {

		if (BeginTable("ShaderLayoutTable", 5)) {

			TableSetupColumn("Edit Mesh");
			TableSetupColumn("   X");
			TableSetupColumn("   Y");
			TableSetupColumn("   Z");
			TableHeadersRow();

			TableNextRow();
			TableNextColumn();
			Text("Position");
			TableNextColumn();
			{
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
				if (DragFloat("##posX", &(highlightedMesh->position.x), 0.5f)) {
					scene.debugSettings.currentSample = 0;
					scene.SSBOcomponent.generateGlobalVertices(scene.meshCollection);
					scene.SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
				if (DragFloat("##posY", &(highlightedMesh->position.y), 0.5f)) {
					scene.debugSettings.currentSample = 0;
					scene.SSBOcomponent.generateGlobalVertices(scene.meshCollection);
					scene.SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
				if (DragFloat("##posZ", &(highlightedMesh->position.z), 0.5f)) {
					scene.debugSettings.currentSample = 0;
					scene.SSBOcomponent.generateGlobalVertices(scene.meshCollection);
					scene.SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				if (Button("Reset##positon")) {
					scene.debugSettings.currentSample = 0;
					highlightedMesh->position = glm::vec3(0.0f);
					scene.SSBOcomponent.generateGlobalVertices(scene.meshCollection);
					scene.SSBOcomponent.updateVertexSSBO();
				}
			}

			TableNextRow();
			TableNextColumn();
			Text("Rotation");
			TableNextColumn();
			{
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
				if (DragFloat("##pitch", &(highlightedMesh->rotation.x), glm::pi<float>() * 0.05f)) {
					scene.debugSettings.currentSample = 0;
					scene.SSBOcomponent.generateGlobalVertices(scene.meshCollection);
					scene.SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
				if (DragFloat("##yaw", &(highlightedMesh->rotation.y), glm::pi<float>() * 0.05f)) {
					scene.debugSettings.currentSample = 0;
					scene.SSBOcomponent.generateGlobalVertices(scene.meshCollection);
					scene.SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
				if (DragFloat("##roll", &(highlightedMesh->rotation.z), glm::pi<float>() * 0.05f)) {
					scene.debugSettings.currentSample = 0;
					scene.SSBOcomponent.generateGlobalVertices(scene.meshCollection);
					scene.SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				if (Button("Reset##rotation")) {
					highlightedMesh->rotation = glm::vec3(0.0f);
					scene.debugSettings.currentSample = 0;
					scene.SSBOcomponent.generateGlobalVertices(scene.meshCollection);
					scene.SSBOcomponent.updateVertexSSBO();
				}
			}

			TableNextRow();
			TableNextColumn();
			Text("Scale");
			TableNextColumn();
			{
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
				if (DragFloat("##scaleX", &(highlightedMesh->scale.x), 0.1f)) {
					scene.debugSettings.currentSample = 0;
					scene.SSBOcomponent.generateGlobalVertices(scene.meshCollection);
					scene.SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
				if (DragFloat("##scaleY", &(highlightedMesh->scale.y), 0.1f)) {
					scene.debugSettings.currentSample = 0;
					scene.SSBOcomponent.generateGlobalVertices(scene.meshCollection);
					scene.SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
				if (DragFloat("##scaleZ", &(highlightedMesh->scale.z), 0.1f)) {
					scene.debugSettings.currentSample = 0;
					scene.SSBOcomponent.generateGlobalVertices(scene.meshCollection);
					scene.SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				if (Button("Reset##scale")) {
					highlightedMesh->scale = glm::vec3(1.0f);
					scene.debugSettings.currentSample = 0;
					scene.SSBOcomponent.generateGlobalVertices(scene.meshCollection);
					scene.SSBOcomponent.updateVertexSSBO();
				}
			}

			TableNextRow();
			TableNextColumn();
			Text("Tint");
			TableNextColumn();
			{
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
				if (SliderFloat("##tintR", &(highlightedMesh->tint.x), 0.0f, 1.0f)) {
					scene.debugSettings.currentSample = 0;
					scene.SSBOcomponent.generateGlobalVertices(scene.meshCollection);
					scene.SSBOcomponent.updateVertexSSBO();
					for (Mesh* mesh : scene.meshCollection) {
						mesh->updateBuffers();
					}
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
				if (SliderFloat("##tintG", &(highlightedMesh->tint.y), 0.0f, 1.0f)) {
					scene.debugSettings.currentSample = 0;
					scene.SSBOcomponent.generateGlobalVertices(scene.meshCollection);
					scene.SSBOcomponent.updateVertexSSBO();
					for (Mesh* mesh : scene.meshCollection) {
						mesh->updateBuffers();
					}
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
				if (SliderFloat("##tintB", &(highlightedMesh->tint.z), 0.0f, 1.0f)) {
					scene.debugSettings.currentSample = 0;
					scene.SSBOcomponent.generateGlobalVertices(scene.meshCollection);
					scene.SSBOcomponent.updateVertexSSBO();
					for (Mesh* mesh : scene.meshCollection) {
						mesh->updateBuffers();
					}
				}
				PopStyleColor(1);
				TableNextColumn();
				if (Button("Reset##tint")) {
					scene.debugSettings.currentSample = 0;
					highlightedMesh->tint = glm::vec3(1.0f);
				}
			}

			TableNextRow();
			TableNextColumn();
			Text("Emission");
			TableNextColumn();
			if (SliderFloat("##emission", &(highlightedMesh->emissive), 0.0f, 500.0f)) {
				scene.debugSettings.currentSample = 0;
				scene.SSBOcomponent.generateMeshHeader(scene.meshCollection);
				scene.SSBOcomponent.updateMeshHeaderSSBO();
			}

			TableNextRow();
			TableNextColumn();
			Text("Animation");
			TableNextColumn();

			static int current_selection;

			// some fucked up shit to get the current_selection of the highlightedMesh animation
			current_selection = 0;
			auto highlighedMeshAnimation = (highlightedMesh->animation.target<void(*)(Mesh&, unsigned int)>());
			auto rawHighlightedMeshAnimation = highlighedMeshAnimation ? (void*)*(highlighedMeshAnimation) : nullptr;

			for (int i = 0; i < Animation::animationMeshFunctions.size(); ++i) {
				auto currentMeshAnimation = (Animation::animationMeshFunctions[i]->function.target<void(*)(Mesh&, unsigned int)>());
				auto rawCurrentMeshAnimation = currentMeshAnimation ? (void*)*(currentMeshAnimation) : nullptr;

				if (rawHighlightedMeshAnimation == rawCurrentMeshAnimation) {
					current_selection = i;
					break;
				}

			}

			const char* current_selection_name = Animation::animationMeshFunctions[current_selection]->name.c_str();

			// Create drop down component
			if (BeginCombo("", current_selection_name)) {

				for (int i = 0; i < Animation::animationMeshFunctions.size(); ++i) {

					const bool is_selected = (current_selection == i);

					// Assign new selected animation to highlighted mesh
					if (Selectable(Animation::animationMeshFunctions[i]->name.c_str(), is_selected)) {
						current_selection = i;
						highlightedMesh->animation = Animation::animationMeshFunctions[i]->function;
					}

					if (is_selected) SetItemDefaultFocus();

				}

				EndCombo();
			}

			TableNextRow();
			TableNextColumn();
			if (Button("Delete")) {
				erase(scene.meshCollection, highlightedMesh);
				scene.debugSettings.highlightedMeshIndex = -1;
				scene.link();
			}

			EndTable();

		}

	}

}

void DebugWindow::drawMiscTab(Scene& scene) {

	if (BeginTable("ShaderLayoutTable", 2)) {

		TableSetupColumn("Misc");
		TableSetupColumn("");
		TableHeadersRow();

		TableNextRow();

		TableNextColumn();
		BeginDisabled(!scene.debugSettings.debugUniversalRoughness);
		if (SliderFloat("Roughness", &scene.debugSettings.debugUniversalRoughnessAmount, 0, 1)) scene.debugSettings.currentSample = 0;
		EndDisabled();

		TableNextRow();

		TableNextColumn();
		if (Checkbox("Universal Roughness", &scene.debugSettings.debugUniversalRoughness)) scene.debugSettings.currentSample = 0;
		TableNextColumn();
		if (Button("Pause")) scene.debugSettings.pause = !scene.debugSettings.pause;


		TableNextColumn();

		EndTable();
	}

}