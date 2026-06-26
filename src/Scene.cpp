#pragma once

#include "Scene.h"
#include "Animation.h"

#include <chrono>
#include <fstream>
#include <filesystem>
#include <format>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

Scene::Scene(Camera& i_camera, unsigned int i_textureWidth, unsigned int i_textureHeight) : camera(i_camera), textureWidth(i_textureWidth), textureHeight(i_textureHeight) {}

Scene::Scene(Camera& i_camera, string fileName, unsigned int i_textureWidth, unsigned int i_textureHeight) : camera(i_camera), textureWidth(i_textureWidth), textureHeight(i_textureHeight) {
	importScene(fileName);
}

Scene::~Scene() {

	for (Mesh* mesh : meshCollection) {
		delete mesh;
	}

}

void Scene::Inputs(GLFWwindow* window) {

	// HIGHLIGHT MESH
	if ((shaderPipelineComponent.gizmo.getSelection(window, camera, debugSettings.highlightedMeshIndex != -1) == Gizmo::Selection::NONE) && !ImGui::GetIO().WantCaptureMouse && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

		debugSettings.mouseLeftClick = true;
		debugSettings.highlightedMeshIndex = -1;
		glfwGetCursorPos(window, &debugSettings.mouseX, &debugSettings.mouseY);

	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		debugSettings.mouseLeftClick = false;
	}	// MOVEMENT (w, a, s, d)
	if (!ImGui::GetIO().WantCaptureMouse && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.Position += camera.speed * camera.Orientation;	// move position foward from orientation
		debugSettings.currentSample = 0;
	}

	if (!ImGui::GetIO().WantCaptureMouse && glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.Position += camera.speed * -glm::normalize(glm::cross(camera.Orientation, camera.Up)); // find the left vector from orientation and add to position
		debugSettings.currentSample = 0;
	}

	if (!ImGui::GetIO().WantCaptureMouse && glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.Position += camera.speed * -camera.Orientation;	// move position backward from orientation
		debugSettings.currentSample = 0;
	}

	if (!ImGui::GetIO().WantCaptureMouse && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.Position += camera.speed * glm::normalize(glm::cross(camera.Orientation, camera.Up));	// find the right vector from orientation and add to position
		debugSettings.currentSample = 0;
	}



	// UP & DOWN (space, ctrl)
	if (!ImGui::GetIO().WantCaptureMouse && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		camera.Position += camera.speed * camera.Up;
		debugSettings.currentSample = 0;
	}
	if (!ImGui::GetIO().WantCaptureMouse && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		camera.Position += camera.speed * -camera.Up;
		debugSettings.currentSample = 0;
	}

	// MOUSE MOVEMENT
	static bool firstClick = true;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		if (firstClick) {
			glfwSetCursorPos(window, (camera.width / 2), (camera.height / 2));
			firstClick = false;
		}

		double mouseX;
		double mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		float rotX = camera.sensitivity * (float)(mouseY - (camera.height / 2)) / camera.height;
		float rotY = camera.sensitivity * (float)(mouseX - (camera.width / 2)) / camera.width;

		glm::vec3 newOrientation = glm::rotate(camera.Orientation, glm::radians(-rotX), glm::normalize(glm::cross(camera.Orientation, camera.Up)));

		if (!((glm::angle(newOrientation, camera.Up) <= glm::radians(5.0f)) || (glm::angle(newOrientation, -camera.Up) <= glm::radians(5.0f)))) {

			camera.Orientation = newOrientation;

		}

		camera.Orientation = glm::rotate(camera.Orientation, glm::radians(-rotY), camera.Up);

		glfwSetCursorPos(window, (float(camera.width) / 2), (float(camera.height) / 2));
		debugSettings.currentSample = 0;

	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;
	}



	// SPEED UP (SHIFT)
	static bool speedUp = false;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {

		if (!speedUp) {		// speedUp check prevents the speed from continuously increasing every frame
			camera.speed *= 2;
			speedUp = true;
		}

	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {

		if (speedUp) {		// speedUp check prevents the speed from continuously decreasing every frame
			camera.speed /= 2;
			speedUp = false;
		}

	}

	// MOVE GIZMO
	static bool gizmoFirstClick = true;
	if (!ImGui::GetIO().WantCaptureMouse && shaderPipelineComponent.gizmo.getSelection(window, camera, debugSettings.highlightedMeshIndex != -1) != Gizmo::Selection::NONE && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		shaderPipelineComponent.gizmo.currentlyInUse = true;
		static glm::vec2 origMouse;

		// only executes on first click
		if (gizmoFirstClick) {

			double mouseX, mouseY;
			glfwGetCursorPos(window, &mouseX, &mouseY);
			origMouse = glm::vec2(mouseX, mouseY);

			gizmoFirstClick = false;

			return;

		}

		// fetch current mouse pos
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		glm::vec2 currentMouse(mouseX, mouseY);

		// move highlighted mesh 
		glm::vec3 gizmoPointDifference = shaderPipelineComponent.gizmo.newPointFromMouseDrag(origMouse, currentMouse, shaderPipelineComponent.gizmo.getSelection(window, camera, debugSettings.highlightedMeshIndex != -1), camera);
		meshCollection[debugSettings.highlightedMeshIndex]->position += gizmoPointDifference;

		// if gizmo has moved, update buffers
		if (gizmoPointDifference != glm::vec3(0.0f)) {
			debugSettings.currentSample = 0;
			SSBOcomponent.generateGlobalVertices(meshCollection);
			SSBOcomponent.updateVertexSSBO();
		}

		// set mouse pos back to original
		glfwSetCursorPos(window, origMouse.x, origMouse.y);

	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		shaderPipelineComponent.gizmo.currentlyInUse = false;
		gizmoFirstClick = true;
	}

}

void Scene::importScene(string fileName) {

	ifstream file("scenes/" + fileName);

	meshCollection.resize(0);

	string line;
	while (getline(file, line)) {

		if (line[0] == '{') {

			getline(file, line); string s_fileName = line.substr(1, line.length() - 2);
			getline(file, line); string s_position = line.substr(1, line.length() - 2);
			getline(file, line); string s_rotation = line.substr(1, line.length() - 2);
			getline(file, line); string s_scale = line.substr(1, line.length() - 2);
			getline(file, line); string s_albedo = line.substr(1, line.length() - 2);
			getline(file, line); string s_normal = line.substr(1, line.length() - 2);
			getline(file, line); string s_roughness = line.substr(1, line.length() - 2);
			getline(file, line); string s_metallic = line.substr(1, line.length() - 2);
			getline(file, line); string s_tint = line.substr(1, line.length() - 2);
			getline(file, line); string s_emissive = line.substr(1, line.length() - 2);

			string temp[3]("");
			int commaCount;

			// POSITION PARSE
			commaCount = 0;
			temp[0] = ""; temp[1] = ""; temp[2] = "";
			for (char c : s_position) {

				if (c == ',') {
					++commaCount;
					continue;
				}

				temp[commaCount].push_back(c);
			}
			glm::vec3 position(stof(temp[0]), stof(temp[1]), stof(temp[2]));

			// ROTATION PARSE
			commaCount = 0;
			temp[0] = ""; temp[1] = ""; temp[2] = "";
			for (char c : s_rotation) {

				if (c == ',') {
					++commaCount;
					continue;
				}

				temp[commaCount].push_back(c);
			}
			glm::vec3 rotation(stof(temp[0]), stof(temp[1]), stof(temp[2]));

			// SCALE PARSE
			commaCount = 0;
			temp[0] = ""; temp[1] = ""; temp[2] = "";
			for (char c : s_scale) {

				if (c == ',') {
					++commaCount;
					continue;
				}

				temp[commaCount].push_back(c);
			}
			glm::vec3 scale(stof(temp[0]), stof(temp[1]), stof(temp[2]));

			// TINT PARSE
			commaCount = 0;
			temp[0] = ""; temp[1] = ""; temp[2] = "";
			for (char c : s_tint) {

				if (c == ',') {
					++commaCount;
					continue;
				}

				temp[commaCount].push_back(c);
			}
			glm::vec3 tint(stof(temp[0]), stof(temp[1]), stof(temp[2]));

			// EMISSION PARSE
			float emissive = stof(s_emissive);

			Mesh* mesh = new Mesh(s_fileName, tint);
			mesh->position = position;
			mesh->rotation = rotation;
			mesh->scale = scale;
			mesh->material->albedo = s_albedo;
			mesh->material->normal = s_normal;
			mesh->material->roughness = s_roughness;
			mesh->material->metallic = s_metallic;
			mesh->emissive = emissive;

			meshCollection.push_back(mesh);
		}

	}

	file.close();
	debugSettings.currentSample = 0;

	link();

	BoundingBox temp = meshCollection[0]->getBoundingBox();

}

void Scene::exportScene(string fileName) {

	// Back up file
	auto now = chrono::system_clock::now();
	auto local_time = chrono::current_zone()->to_local(now);
	string timeStamp = format("{:%Y-%m-%d_%H-%M-%S}", local_time);

	filesystem::create_directories("scenes/backups/");
	std::ofstream createDirectoryTempA("scenes/backups/" + timeStamp + "_backup_" + fileName); createDirectoryTempA.close();
	filesystem::path destination = string("scenes/backups/" + timeStamp + "_backup_" + fileName);

	std::ofstream createDirectoryTempB("scenes/" + fileName); createDirectoryTempB.close();
	filesystem::path source = string("scenes/" + fileName);
	filesystem::copy_file(source, destination, filesystem::copy_options::overwrite_existing);

	ofstream file("scenes/" + fileName);

	file << "// FORMAT:\n";
	file << "// \t fileName\n";
	file << "// \t position\n";
	file << "// \t rotation\n";
	file << "// \t scale\n";
	file << "// \t albedo\n";
	file << "// \t normal\n";
	file << "// \t roughness\n";
	file << "// \t metallic\n";
	file << "// \t tint\n";
	file << "// \t emissive\n\n";

	for (int i = 0; i < meshCollection.size(); ++i) {

		Mesh* mesh = meshCollection[i];

		string position = to_string(mesh->position.x) + ", " + to_string(mesh->position.y) + ", " + to_string(mesh->position.z);
		string rotation = to_string(mesh->rotation.x) + ", " + to_string(mesh->rotation.y) + ", " + to_string(mesh->rotation.z);
		string scale = to_string(mesh->scale.x) + ", " + to_string(mesh->scale.y) + ", " + to_string(mesh->scale.z);

		string albedo = mesh->material->albedo;
		string normal = mesh->material->normal;
		string roughness = mesh->material->roughness;
		string metallic = mesh->material->metallic;

		string tint = to_string(mesh->tint.r) + ", " + to_string(mesh->tint.g) + ", " + to_string(mesh->tint.b);
		string emissive = to_string(mesh->emissive);

		file << "{\n";
		file << "\t" << mesh->fileName << ",\n";
		file << "\t" << position << ",\n";
		file << "\t" << rotation << ",\n";
		file << "\t" << scale << ",\n";
		file << "\t" << albedo << ",\n";
		file << "\t" << normal << ",\n";
		file << "\t" << roughness << ",\n";
		file << "\t" << metallic << ",\n";
		file << "\t" << tint << ",\n";
		file << "\t" << emissive << "\n";
		file << "}";

		// writes last line
		if (i != meshCollection.size() - 1) {
			file << ",\n\n";
		}
	}


	file.close();

}

void Scene::Draw(GLFWwindow* window) {

	SSBOcomponent.generateBoundingBoxes(meshCollection);
	SSBOcomponent.updateBoundingBoxesSSBO();

	auto start = chrono::high_resolution_clock::now();

	if (debugSettings.videoRenderPhase == DebugSettings::RenderPhase::COMPLETE) this->Inputs(window);
	camera.updateMatrix();

	if (!debugSettings.pause) {

		// CLEAR BACKGROUND
		glViewport(0, 0, camera.width, camera.height);
		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderPipelineComponent.Draw_DepthPrepass(camera, meshCollection, SSBOcomponent.meshTextures, debugSettings);
		shaderPipelineComponent.Draw_PathTracingPass(camera, meshCollection, SSBOcomponent.meshTextures, debugSettings);
		shaderPipelineComponent.Draw_PostProcessingPass(camera, meshCollection, SSBOcomponent.meshTextures, window, debugSettings);


	}

	auto end = chrono::high_resolution_clock::now();
	auto raw_duration = end - start;
	chrono::duration<double, milli> ms_double = raw_duration;
	double frameTime = ms_double.count();

	// HIGHLIGHT MESH
	Mesh* highlightedMesh = (debugSettings.highlightedMeshIndex== -1) ? nullptr : meshCollection[debugSettings.highlightedMeshIndex];
	imguiWindow.drawImgui(frameTime, RenderComponent.sceneAnimationFrame, this, highlightedMesh);

	// SCREENSHOT
	RenderComponent.handleQueuedImageRender(debugSettings, camera);
	RenderComponent.handleQueuedVideoRender(debugSettings, camera, SSBOcomponent, meshCollection);

	// ANIMATION PREVIEW
	RenderComponent.handleQueuedAnimationPreview(debugSettings, camera, SSBOcomponent, meshCollection);

	if (debugSettings.currentSample != debugSettings.maxSamples) ++debugSettings.currentSample;

	// WINDOW NAME
	setWindowTitle(window, frameTime);

}

void Scene::setWindowTitle(GLFWwindow* window, double frameTime) {

	string windowName =
		"Samples: " + to_string(debugSettings.currentSample) + "/" + to_string(debugSettings.maxSamples) + "\t" +
		"FPS: " + (to_string(static_cast<int>(1000 / frameTime)) + "      ").substr(0, 6) + "\t" +
		"Animation Frame: " + to_string(RenderComponent.sceneAnimationFrame) + "/" + to_string(debugSettings.totalAnimationFrames);

	glfwSetWindowTitle(window, windowName.c_str());

}

void Scene::link() {

	SSBOcomponent.deleteSSBOs();

	SSBOcomponent.generateGlobalVertices(meshCollection);
	SSBOcomponent.generateGlobalIndices(meshCollection);
	SSBOcomponent.generateMeshTextures(meshCollection);
	SSBOcomponent.generateMeshHeader(meshCollection);
	SSBOcomponent.generateBoundingBoxes(meshCollection);

	SSBOcomponent.generateSSBOs(textureWidth, textureHeight);

	shaderPipelineComponent.depthPrepassShader = new Shader("shaders/z_prepass.vert", "shaders/z_prepass.frag");
	shaderPipelineComponent.pathTracingShader = new Shader("shaders/path_tracing.vert", "shaders/path_tracing.frag");
	shaderPipelineComponent.postProcessingShader = new Shader("shaders/post_processing.vert", "shaders/post_processing.frag");
	shaderPipelineComponent.link(debugSettings, camera, backgroundColor);

}

void ImguiWindow::drawImgui(double frameTime, unsigned int animationFrame, Scene* scene, Mesh* highlightedMesh) {

	using namespace ImGui;

	const ImVec4 defaultComponentColor(0.26f, 0.59f, 0.98f, 0.40f);

	const ImVec4 validPathColor(0.1f, 0.3f, 0.1f, 1.0f);
	const ImVec4 invalidPathColor(0.15f, 0.15f, 0.15f, 1.0f);

	// DEBUG WINDOW
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	NewFrame();

	if (IsKeyPressed(ImGuiKey_F) && !GetIO().WantCaptureMouse) {
		scene->debugSettings.drawWindow = !scene->debugSettings.drawWindow;
		scene->debugSettings.currentSample = 0;
	}

	if (!scene->debugSettings.drawWindow) { EndFrame();  return; }

	Begin("Debug");
	Separator();

	Text("Samples: %d/%d\t\tFT(ms): %.3f\t\tFPS: %d", scene->debugSettings.currentSample, scene->debugSettings.maxSamples, frameTime, static_cast<int>(1000 / frameTime));

	if (BeginTable("ShaderLayoutTable", 2)) {

		TableSetupColumn("Render");
		TableSetupColumn("Render Settings");
		TableHeadersRow();

		TableNextRow();

		BeginDisabled(scene->debugSettings.previewAnimationPhase != DebugSettings::RenderPhase::COMPLETE);

		// RENDER
		TableNextColumn();
		BeginDisabled(scene->debugSettings.imageRenderPhase != DebugSettings::RenderPhase::COMPLETE);
		if (Button("Render Image")) {
			scene->debugSettings.imageRenderPhase = DebugSettings::RenderPhase::WAITING;
		}
		EndDisabled();
		BeginDisabled(scene->debugSettings.videoRenderPhase != DebugSettings::RenderPhase::COMPLETE);
		SameLine();
		if (Button("Render Video")) {
			scene->debugSettings.videoRenderPhase = DebugSettings::RenderPhase::WAITING;
			scene->RenderComponent.sceneAnimationFrame = 0;
		}

		SliderInt("Frames", &scene->debugSettings.totalAnimationFrames, 1, 1000);
		EndDisabled();

		if (Button("Preview")) {
			scene->debugSettings.previewAnimationPhase = DebugSettings::RenderPhase::WAITING;
			scene->debugSettings.debugMode = static_cast<int>(DebugSettings::DebugTypes::ALBEDO);
		}
		SameLine();
		Text("Frame: %d/%d", animationFrame, scene->debugSettings.totalAnimationFrames);

		// RENDER SETTINGS
		TableNextColumn();
		if (SliderInt("Bounces", &scene->debugSettings.maxBounces, 1, MAX_BOUNCES)) scene->debugSettings.currentSample = 0;
		if (SliderInt("Samples", &scene->debugSettings.maxSamples, 1, MAX_SAMPLES)) scene->debugSettings.currentSample = 0;
		if (Button("Clear Samples")) scene->debugSettings.currentSample = 0;
		Spacing();
		if (Button("Pause")) scene->debugSettings.pause = !scene->debugSettings.pause;

		EndDisabled();

		EndTable();
	}

	// RENDER VISUALIZATION / POST PROCESSING
	if (BeginTable("ShaderLayoutTable", 2)) {

		TableSetupColumn("Render Visualization");
		TableSetupColumn("Post Processing");
		TableHeadersRow();

		TableNextRow();

		// RENDER VISUALIZATION
		TableNextColumn();
		BeginDisabled(scene->debugSettings.previewAnimationPhase != DebugSettings::RenderPhase::COMPLETE);
		if (RadioButton("Disabled", &scene->debugSettings.debugMode, static_cast<int>(DebugSettings::DebugTypes::DISABLED))) scene->debugSettings.currentSample = 0;
		Spacing();
		if (RadioButton("Albedo", &scene->debugSettings.debugMode, static_cast<int>(DebugSettings::DebugTypes::ALBEDO))) scene->debugSettings.currentSample = 0;
		if (RadioButton("Normal", &scene->debugSettings.debugMode, static_cast<int>(DebugSettings::DebugTypes::NORMAL))) scene->debugSettings.currentSample = 0;
		if (RadioButton("Roughness", &scene->debugSettings.debugMode, static_cast<int>(DebugSettings::DebugTypes::ROUGHNESS))) scene->debugSettings.currentSample = 0;
		if (RadioButton("Metallic", &scene->debugSettings.debugMode, static_cast<int>(DebugSettings::DebugTypes::METALLIC))) scene->debugSettings.currentSample = 0;
		Spacing();
		if (RadioButton("VertexNormal", &scene->debugSettings.debugMode, static_cast<int>(DebugSettings::DebugTypes::VERTEX_NORMAL))) scene->debugSettings.currentSample = 0;
		EndDisabled();
		Spacing();
		Checkbox("Lambertian Shading", &scene->debugSettings.debugLambertian);

		// POST PROCESSING
		TableNextColumn();
		SliderFloat("Min Brightness", &scene->debugSettings.minBrightness, 0.0f, scene->debugSettings.maxBrightness - 0.001f);
		SliderFloat("Max Brightness", &scene->debugSettings.maxBrightness, scene->debugSettings.minBrightness + 0.001f, 1.0f);

		EndTable();
	}

	// SCENE SETTINGS
	if (BeginTable("ShaderLayoutTable", 3)) {

		TableSetupColumn("Scene");
		TableHeadersRow();

		TableNextRow();

		std::filesystem::path filePath;

		TableNextColumn();

		filePath = "scenes/" + string(scene->debugSettings.importName);
		bool isImportValid = (std::filesystem::exists(filePath) && filePath.extension().string() == SCENE_FILE_EXTENSION);
		PushStyleColor(ImGuiCol_FrameBg, (isImportValid ? validPathColor : invalidPathColor));
		InputTextWithHint("##importFileName", "scene.txt", scene->debugSettings.importName, IM_ARRAYSIZE(scene->debugSettings.importName));
		PopStyleColor(1);
		TableNextColumn();

		filePath = "scenes/" + string(scene->debugSettings.exportName);
		bool isExportValid = (filePath.extension().string() == SCENE_FILE_EXTENSION);
		PushStyleColor(ImGuiCol_FrameBg, (isExportValid ? validPathColor : invalidPathColor));
		InputTextWithHint("##exportFileName", "scene.txt", scene->debugSettings.exportName, IM_ARRAYSIZE(scene->debugSettings.exportName));
		PopStyleColor(1);
		TableNextColumn();

		filePath = scene->debugSettings.importOBJname;
		bool isOBJValid = (std::filesystem::exists(filePath) && filePath.extension().string() == ".obj");
		PushStyleColor(ImGuiCol_FrameBg, (isOBJValid ? validPathColor : invalidPathColor));
		InputTextWithHint("##importOBJName", "models/cube.obj", scene->debugSettings.importOBJname, IM_ARRAYSIZE(scene->debugSettings.importOBJname));
		PopStyleColor(1);

		TableNextRow();
		TableNextColumn();
		BeginDisabled(!isImportValid);
		if (Button("Import Scene")) {
			scene->importScene(scene->debugSettings.importName);
			scene->link();
			strcpy(scene->debugSettings.exportName, scene->debugSettings.importName);
			scene->debugSettings.importName[0] = 0x00;
		}
		EndDisabled();

		TableNextColumn();
		BeginDisabled(!isExportValid);
		if (Button("Export Scene")) {
			scene->exportScene(scene->debugSettings.exportName);
			scene->debugSettings.exportName[0] = 0x00;
		}
		EndDisabled();

		TableNextColumn();
		BeginDisabled(!isOBJValid);
		if (Button("Import OBJ")) {

			Mesh* mesh = new Mesh(scene->debugSettings.importOBJname);
			scene->meshCollection.push_back(mesh);
			scene->link();
			scene->debugSettings.importOBJname[0] = 0x00;

		}
		EndDisabled();

		EndTable();
	}

	// CAMERA SETTINGS
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
			if (DragFloat("##posX", &(scene->camera.Position.x), 0.5f)) scene->debugSettings.currentSample = 0;
			PopStyleColor(1);
			TableNextColumn();
			PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
			if (DragFloat("##posY", &(scene->camera.Position.y), 0.5f)) scene->debugSettings.currentSample = 0;
			PopStyleColor(1);
			TableNextColumn();
			PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
			if (DragFloat("##posZ", &(scene->camera.Position.z), 0.5f)) scene->debugSettings.currentSample = 0;
			PopStyleColor(1);
			TableNextColumn();
			if (Button("Reset##positon")) scene->camera.Position = glm::vec3(0.0f);
		}

		TableNextRow();
		TableNextColumn();
		Text("Animation");
		TableNextColumn();

		static int current_selection;

		// some fucked up shit to get the current_selection of the camera animation
		current_selection = 0;
		auto highlighedCameraAnimation = (scene->camera.animation.target<void(*)(Camera&, unsigned int)>());
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
					scene->camera.animation = Animation::animationCameraFunctions[i]->function;
				}

				if (is_selected) SetItemDefaultFocus();

			}

			EndCombo();
		}

		TableNextRow();
		TableNextColumn();
		Text("Settings");
		TableNextColumn();

		if (SliderFloat("FOV", &(scene->camera.FOVdeg), 0.0f, 90.0f)) scene->debugSettings.currentSample = 0;
		TableNextColumn();
		if (SliderFloat("Near Plane", &(scene->camera.nearPlane), 0.0f, scene->camera.farPlane)) scene->debugSettings.currentSample = 0;
		TableNextColumn();
		if (SliderFloat("Far Plane", &(scene->camera.farPlane), scene->camera.nearPlane, 10000.0f)) scene->debugSettings.currentSample = 0;

		EndTable();
	}

	// MESH SETTINGS
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
					scene->debugSettings.currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
				if (DragFloat("##posY", &(highlightedMesh->position.y), 0.5f)) {
					scene->debugSettings.currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
				if (DragFloat("##posZ", &(highlightedMesh->position.z), 0.5f)) {
					scene->debugSettings.currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				if (Button("Reset##positon")) {
					scene->debugSettings.currentSample = 0;
					highlightedMesh->position = glm::vec3(0.0f);
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
			}

			TableNextRow();
			TableNextColumn();
			Text("Rotation");
			TableNextColumn();
			{
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
				if (DragFloat("##pitch", &(highlightedMesh->rotation.x), glm::pi<float>() * 0.05f)) {
					scene->debugSettings.currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
				if (DragFloat("##yaw", &(highlightedMesh->rotation.y), glm::pi<float>() * 0.05f)) {
					scene->debugSettings.currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
				if (DragFloat("##roll", &(highlightedMesh->rotation.z), glm::pi<float>() * 0.05f)) {
					scene->debugSettings.currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				if (Button("Reset##rotation")) {
					highlightedMesh->rotation = glm::vec3(0.0f); 
					scene->debugSettings.currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
			}

			TableNextRow();
			TableNextColumn();
			Text("Scale");
			TableNextColumn();
			{
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
				if (DragFloat("##scaleX", &(highlightedMesh->scale.x), 0.1f)) {
					scene->debugSettings.currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
				if (DragFloat("##scaleY", &(highlightedMesh->scale.y), 0.1f)) {
					scene->debugSettings.currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
				if (DragFloat("##scaleZ", &(highlightedMesh->scale.z), 0.1f)) {
					scene->debugSettings.currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				if (Button("Reset##scale")) {
					highlightedMesh->scale = glm::vec3(1.0f); 
					scene->debugSettings.currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
			}

			TableNextRow();
			TableNextColumn();
			Text("Tint");
			TableNextColumn();
			{
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
				if (SliderFloat("##tintR", &(highlightedMesh->tint.x), 0.0f, 1.0f)) {
					scene->debugSettings.currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
					for (Mesh* mesh : scene->meshCollection) {
						mesh->updateBuffers();
					}
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
				if (SliderFloat("##tintG", &(highlightedMesh->tint.y), 0.0f, 1.0f)) {
					scene->debugSettings.currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
					for (Mesh* mesh : scene->meshCollection) {
						mesh->updateBuffers();
					}
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
				if (SliderFloat("##tintB", &(highlightedMesh->tint.z), 0.0f, 1.0f)) {
					scene->debugSettings.currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
					for (Mesh* mesh : scene->meshCollection) {
						mesh->updateBuffers();
					}
				}
				PopStyleColor(1);
				TableNextColumn();
				if (Button("Reset##tint")) {
					scene->debugSettings.currentSample = 0;
					highlightedMesh->tint = glm::vec3(1.0f);
				}
			}

			TableNextRow();
			TableNextColumn();
			Text("Emission");
			TableNextColumn();
			if (SliderFloat("##emission", &(highlightedMesh->emissive), 0.0f, 500.0f)) {
				scene->debugSettings.currentSample = 0;
				scene->SSBOcomponent.generateMeshHeader(scene->meshCollection);
				scene->SSBOcomponent.updateMeshHeaderSSBO();
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
				erase(scene->meshCollection, highlightedMesh);
				scene->debugSettings.highlightedMeshIndex = -1;
				scene->link();
			}

			EndTable();

		}

	}

	if (BeginTable("ShaderLayoutTable", 2)) {

		TableSetupColumn("Misc");
		TableSetupColumn("");
		TableHeadersRow();

		TableNextRow();

		TableNextColumn();
		BeginDisabled(!scene->debugSettings.debugUniversalRoughness);
		if (SliderFloat("Roughness", &scene->debugSettings.debugUniversalRoughnessAmount, 0, 1)) scene->debugSettings.currentSample = 0;
		EndDisabled();

		TableNextRow();

		TableNextColumn();
		if (Checkbox("Universal Roughness", &scene->debugSettings.debugUniversalRoughness)) scene->debugSettings.currentSample = 0;
		TableNextColumn();
		if (Button("Pause")) scene->debugSettings.pause = !scene->debugSettings.pause;


		TableNextColumn();

		EndTable();
	}

	// clear samples if window is moved or resized
	if (windowSize.x != GetWindowSize().x || windowSize.y != GetWindowSize().y) scene->debugSettings.currentSample = 0;
	if (windowPosition.x != GetWindowPos().x || windowPosition.y != GetWindowPos().y) scene->debugSettings.currentSample = 0;

	windowSize = GetWindowSize();
	windowPosition = GetWindowPos();



	End();

	Render();
	ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());

}