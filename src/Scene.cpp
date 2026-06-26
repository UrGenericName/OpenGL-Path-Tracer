#pragma once

#include "Scene.h"

#include "Animation.h"
#include <iostream>
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

double Scene::getFrameTime() { return frameTime; }

void Scene::Inputs(GLFWwindow* window) {

	// HIGHLIGHT MESH
	if ((shaderPipelineComponent.gizmo.getSelection(window, camera, debugSettings.highlightedMeshIndex != -1) == Gizmo::Selection::NONE) && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

		debugSettings.mouseLeftClick = true;
		debugSettings.highlightedMeshIndex = -1;
		glfwGetCursorPos(window, &debugSettings.mouseX, &debugSettings.mouseY);

	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		debugSettings.mouseLeftClick = false;
	}	// MOVEMENT (w, a, s, d)
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.Position += camera.speed * camera.Orientation;	// move position foward from orientation
		debugSettings.currentSample = 0;
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.Position += camera.speed * -glm::normalize(glm::cross(camera.Orientation, camera.Up)); // find the left vector from orientation and add to position
		debugSettings.currentSample = 0;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.Position += camera.speed * -camera.Orientation;	// move position backward from orientation
		debugSettings.currentSample = 0;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.Position += camera.speed * glm::normalize(glm::cross(camera.Orientation, camera.Up));	// find the right vector from orientation and add to position
		debugSettings.currentSample = 0;
	}



	// UP & DOWN (space, ctrl)
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		camera.Position += camera.speed * camera.Up;
		debugSettings.currentSample = 0;
	}
	if (shaderPipelineComponent.gizmo.currentlyInUse == false && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
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
	if (shaderPipelineComponent.gizmo.getSelection(window, camera, debugSettings.highlightedMeshIndex != -1) != Gizmo::Selection::NONE && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		shaderPipelineComponent.gizmo.currentlyInUse = true;
		static glm::vec3 origPos = meshCollection[debugSettings.highlightedMeshIndex]->position;
		static glm::vec3 gizmoPointDifference(0.0f);
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
		gizmoPointDifference += shaderPipelineComponent.gizmo.newPointFromMouseDrag(origMouse, currentMouse, shaderPipelineComponent.gizmo.getSelection(window, camera, debugSettings.highlightedMeshIndex != -1), camera);
		
		// snap to local grid when ctrl is pressed
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
			meshCollection[debugSettings.highlightedMeshIndex]->position = origPos + round(gizmoPointDifference);
		} else {
			meshCollection[debugSettings.highlightedMeshIndex]->position = origPos + gizmoPointDifference;
		}



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

	auto start = chrono::high_resolution_clock::now();

	if (debugSettings.videoRenderPhase == DebugSettings::RenderPhase::COMPLETE && debugSettings.usingDebugWindow == false) this->Inputs(window);
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
	frameTime = ms_double.count();

	// SCREENSHOT
	renderComponent.handleQueuedImageRender(debugSettings, camera);
	renderComponent.handleQueuedVideoRender(debugSettings, camera, SSBOcomponent, meshCollection);

	// ANIMATION PREVIEW
	renderComponent.handleQueuedAnimationPreview(debugSettings, camera, SSBOcomponent, meshCollection);

	if (debugSettings.currentSample != debugSettings.maxSamples) ++debugSettings.currentSample;

	// WINDOW NAME
	setWindowTitle(window, frameTime);

}

void Scene::setWindowTitle(GLFWwindow* window, double frameTime) {

	string windowName =
		"Samples: " + to_string(debugSettings.currentSample) + "/" + to_string(debugSettings.maxSamples) + "\t" +
		"FPS: " + (to_string(static_cast<int>(1000 / frameTime)) + "      ").substr(0, 6) + "\t" +
		"Animation Frame: " + to_string(renderComponent.animationFrame) + "/" + to_string(debugSettings.totalAnimationFrames);

	glfwSetWindowTitle(window, windowName.c_str());

}

void Scene::link() {

	SSBOcomponent.deleteSSBOs();

	SSBOcomponent.generateGlobalVertices(meshCollection);
	SSBOcomponent.generateGlobalIndices(meshCollection);
	SSBOcomponent.generateMeshTextures(meshCollection);
	SSBOcomponent.generateMeshHeader(meshCollection);
	SSBOcomponent.generateBoundingBoxes(meshCollection);

	SSBOcomponent.generateSSBOs(meshCollection, textureWidth, textureHeight);

	shaderPipelineComponent.depthPrepassShader = new Shader("shaders/z_prepass.vert", "shaders/z_prepass.frag");
	shaderPipelineComponent.pathTracingShader = new Shader("shaders/path_tracing.vert", "shaders/path_tracing.frag");
	shaderPipelineComponent.postProcessingShader = new Shader("shaders/post_processing.vert", "shaders/post_processing.frag");
	shaderPipelineComponent.link(debugSettings, camera, backgroundColor);

}