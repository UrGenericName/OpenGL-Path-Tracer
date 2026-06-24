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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

Scene::Scene(Camera& i_camera, unsigned int i_textureWidth, unsigned int i_textureHeight) : camera(i_camera), textureWidth(i_textureWidth), textureHeight(i_textureHeight) {}

Scene::Scene(Camera& i_camera, string fileName, unsigned int i_textureWidth, unsigned int i_textureHeight) : camera(i_camera), textureWidth(i_textureWidth), textureHeight(i_textureHeight) {
	importScene(fileName);
}

Scene::~Scene() {

	for (Mesh* mesh : meshCollection) {
		delete mesh;
	}

	depthPrepassShader->Delete();
	pathTracingShader->Delete();
	postProcessingShader->Delete();

	frameBuffer->Delete();
	accumulationBuffer->Delete();

	colorNoise->Delete();

}

void Scene::Inputs(GLFWwindow* window) {

	// HIGHLIGHT MESH
	if ((gizmo.getSelection(window, camera, imguiWindow.highlightedMesh != -1) == Gizmo::Selection::NONE) && !ImGui::GetIO().WantCaptureMouse && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

		imguiWindow.mouseLeftClick = true;
		imguiWindow.highlightedMesh = -1;
		glfwGetCursorPos(window, &imguiWindow.mouseX, &imguiWindow.mouseY);

	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		imguiWindow.mouseLeftClick = false;
	}	// MOVEMENT (w, a, s, d)
	if (!ImGui::GetIO().WantCaptureMouse && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.Position += camera.speed * camera.Orientation;	// move position foward from orientation
		imguiWindow.currentSample = 0;
	}

	if (!ImGui::GetIO().WantCaptureMouse && glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.Position += camera.speed * -glm::normalize(glm::cross(camera.Orientation, camera.Up)); // find the left vector from orientation and add to position
		imguiWindow.currentSample = 0;
	}

	if (!ImGui::GetIO().WantCaptureMouse && glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.Position += camera.speed * -camera.Orientation;	// move position backward from orientation
		imguiWindow.currentSample = 0;
	}

	if (!ImGui::GetIO().WantCaptureMouse && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.Position += camera.speed * glm::normalize(glm::cross(camera.Orientation, camera.Up));	// find the right vector from orientation and add to position
		imguiWindow.currentSample = 0;
	}



	// UP & DOWN (space, ctrl)
	if (!ImGui::GetIO().WantCaptureMouse && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		camera.Position += camera.speed * camera.Up;
		imguiWindow.currentSample = 0;
	}
	if (!ImGui::GetIO().WantCaptureMouse && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		camera.Position += camera.speed * -camera.Up;
		imguiWindow.currentSample = 0;
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
		imguiWindow.currentSample = 0;

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
	if (!ImGui::GetIO().WantCaptureMouse && gizmo.getSelection(window, camera, imguiWindow.highlightedMesh != -1) != Gizmo::Selection::NONE && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		gizmo.currentlyInUse = true;
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
		glm::vec3 gizmoPointDifference = gizmo.newPointFromMouseDrag(origMouse, currentMouse, gizmo.getSelection(window, camera, imguiWindow.highlightedMesh != -1), camera);
		meshCollection[imguiWindow.highlightedMesh]->position += gizmoPointDifference;

		// if gizmo has moved, update buffers
		if (gizmoPointDifference != glm::vec3(0.0f)) {
			imguiWindow.currentSample = 0;
			SSBOcomponent.generateGlobalVertices(meshCollection);
			SSBOcomponent.updateVertexSSBO();
		}

		// set mouse pos back to original
		glfwSetCursorPos(window, origMouse.x, origMouse.y);

	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		gizmo.currentlyInUse = false;
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
	imguiWindow.currentSample = 0;

	link();

	BoundingBox temp = meshCollection[0]->getBoundingBox();

}

void Scene::exportScene(string fileName) {

	// Back up file
	auto now = chrono::system_clock::now();
	auto local_time = chrono::current_zone()->to_local(now);
	string timeStamp = format("{:%Y-%m-%d_%H-%M-%S}", local_time);

	filesystem::create_directories("scenes/backups/");
	filesystem::path destination = string("scenes/backups/" + timeStamp + "_backup_" + fileName);
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

	if (imguiWindow.videoRenderPhase == ImguiWindow::RenderPhase::COMPLETE) this->Inputs(window);
	camera.updateMatrix();

	if (!imguiWindow.pause) {

		// CLEAR BACKGROUND
		glViewport(0, 0, camera.width, camera.height);
		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Draw_DepthPrepass(*depthPrepassShader);
		Draw_PathTracingPass(*pathTracingShader);
		Draw_PostProcessingPass(*postProcessingShader, window);

	}

	auto end = chrono::high_resolution_clock::now();
	auto raw_duration = end - start;
	chrono::duration<double, milli> ms_double = raw_duration;
	double frameTime = ms_double.count();

	// HIGHLIGHT MESH
	Mesh* highlightedMesh = (imguiWindow.highlightedMesh == -1) ? nullptr : meshCollection[imguiWindow.highlightedMesh];
	imguiWindow.drawImgui(frameTime, sceneAnimationFrame, this, highlightedMesh);

	// SCREENSHOT
	handleQueuedImageRender();
	handleQueuedVideoRender();

	// ANIMATION PREVIEW
	handleQueuedAnimationPreview();

	if (imguiWindow.currentSample != imguiWindow.maxSamples) ++imguiWindow.currentSample;

	// WINDOW NAME
	setWindowTitle(window, frameTime);

}

void Scene::setWindowTitle(GLFWwindow* window, double frameTime) {

	string windowName =
		"Samples: " + to_string(imguiWindow.currentSample) + "/" + to_string(imguiWindow.maxSamples) + "\t" +
		"FPS: " + (to_string(static_cast<int>(1000 / frameTime)) + "      ").substr(0, 6) + "\t" +
		"Animation Frame: " + to_string(sceneAnimationFrame) + "/" + to_string(imguiWindow.totalAnimationFrames);

	glfwSetWindowTitle(window, windowName.c_str());

}

void Scene::AnimateComponents(unsigned int currentFrame) {

	// CAMERA ANIMATION
	if (camera.animation != nullptr) camera.animation(camera, currentFrame);

	// MESH ANIMATION
	for (int i = 0; i < meshCollection.size(); ++i) {

		Mesh* mesh = meshCollection[i];

		if (mesh->animation != nullptr) {
			mesh->animation(*mesh, currentFrame);
		}

		mesh->updateBuffers();

	}

	imguiWindow.currentSample = 0;

	SSBOcomponent.generateGlobalVertices(meshCollection);
	SSBOcomponent.updateVertexSSBO();

}

void Scene::handleQueuedAnimationPreview() {

	static vector<Mesh*> meshCollectionDeepCopy;
	static Camera cameraCopy(camera);

	if (imguiWindow.previewAnimationPhase == ImguiWindow::RenderPhase::WAITING) {

		// save previous mesh data before animating
		for (auto mesh : meshCollection) {
			meshCollectionDeepCopy.push_back(new Mesh(*mesh));
		}

		// First frame
		AnimateComponents(sceneAnimationFrame);

		imguiWindow.previewAnimationPhase = ImguiWindow::RenderPhase::RENDERING;

	}
	else if (imguiWindow.previewAnimationPhase == ImguiWindow::RenderPhase::RENDERING) {

		AnimateComponents(++sceneAnimationFrame);

		if (sceneAnimationFrame == imguiWindow.totalAnimationFrames) {

			// Reset mesh collection back to deep copy
			for (auto mesh : meshCollection) delete mesh;
			meshCollection = meshCollectionDeepCopy;
			meshCollectionDeepCopy.resize(0);

			SSBOcomponent.generateGlobalVertices(meshCollection); SSBOcomponent.updateVertexSSBO();
			SSBOcomponent.generateGlobalIndices(meshCollection); SSBOcomponent.updateIndicesSSBO();

			if (camera.animation != nullptr) camera = cameraCopy;

			sceneAnimationFrame = 0;
			imguiWindow.previewAnimationPhase = ImguiWindow::RenderPhase::COMPLETE;
		}

	}

}

void Scene::handleQueuedVideoRender() {

	static int lastHighlightedMeshValue;
	static bool lastDrawWindowValue;

	static string timeStamp;

	static vector<Mesh*> meshCollectionDeepCopy;
	static Camera cameraCopy(camera);

	if (imguiWindow.videoRenderPhase == ImguiWindow::RenderPhase::WAITING) {

		// save previous mesh data before animating
		for (auto mesh : meshCollection) {
			meshCollectionDeepCopy.push_back(new Mesh(*mesh));
		}

		lastDrawWindowValue = imguiWindow.drawWindow;
		lastHighlightedMeshValue = imguiWindow.highlightedMesh;

		auto now = chrono::system_clock::now();
		auto local_time = chrono::current_zone()->to_local(now);
		timeStamp = format("{:%Y-%m-%d_%H-%M-%S}", local_time);

		filesystem::create_directories("output/video_" + timeStamp + "/");
		string fileName = "video_" + timeStamp + "/" + to_string(sceneAnimationFrame) + ".png";

		imguiWindow.drawWindow = false;
		imguiWindow.highlightedMesh = -1;

		// First frame
		AnimateComponents(sceneAnimationFrame);

		imguiWindow.videoRenderPhase = ImguiWindow::RenderPhase::RENDERING;

	}
	else if (imguiWindow.videoRenderPhase == ImguiWindow::RenderPhase::RENDERING) {

		if (imguiWindow.currentSample == imguiWindow.maxSamples) {

			string fileName = "video_" + timeStamp + "/" + to_string(sceneAnimationFrame) + ".png";

			AnimateComponents(++sceneAnimationFrame);
			renderImage(fileName);

			if (sceneAnimationFrame == (imguiWindow.totalAnimationFrames + 1)) {

				// Reset mesh collection back to deep copy
				for (auto mesh : meshCollection) delete mesh;
				meshCollection = meshCollectionDeepCopy;
				meshCollectionDeepCopy.resize(0);

				SSBOcomponent.generateGlobalVertices(meshCollection); SSBOcomponent.updateVertexSSBO();
				SSBOcomponent.generateGlobalIndices(meshCollection); SSBOcomponent.updateIndicesSSBO();

				if (camera.animation != nullptr) camera = cameraCopy;

				sceneAnimationFrame = 0;
				imguiWindow.drawWindow = lastDrawWindowValue;
				imguiWindow.highlightedMesh = lastHighlightedMeshValue;
				imguiWindow.videoRenderPhase = ImguiWindow::RenderPhase::COMPLETE;
			}

		}
	}

}

void Scene::handleQueuedImageRender() {

	static int lastHighlightedMeshValue;
	static bool lastDrawWindowValue;

	if (imguiWindow.imageRenderPhase == ImguiWindow::RenderPhase::WAITING) {

		lastDrawWindowValue = imguiWindow.drawWindow;
		lastHighlightedMeshValue = imguiWindow.highlightedMesh;

		imguiWindow.drawWindow = false;
		imguiWindow.highlightedMesh = -1;
		imguiWindow.imageRenderPhase = ImguiWindow::RenderPhase::RENDERING;

	}
	else if (imguiWindow.imageRenderPhase == ImguiWindow::RenderPhase::RENDERING) {

		if (imguiWindow.currentSample == imguiWindow.maxSamples) {

			auto now = chrono::system_clock::now();
			auto local_time = chrono::current_zone()->to_local(now);
			string timeStamp = format("{:%Y-%m-%d_%H-%M-%S}", local_time);
			string fileName = "output_" + timeStamp + ".png";

			renderImage(fileName);

			imguiWindow.drawWindow = lastDrawWindowValue;
			imguiWindow.highlightedMesh = lastHighlightedMeshValue;

			imguiWindow.imageRenderPhase = ImguiWindow::RenderPhase::COMPLETE;

		}

	}

}

void Scene::renderImage(string fileName) {

	vector<GLfloat> pixels(camera.width * camera.height * 3, 0);
	glReadPixels(0, 0, camera.width, camera.height, GL_RGB, GL_FLOAT, pixels.data());

	vector<unsigned char> pixelsChar(camera.width * camera.height * 3, 0);

	for (int i = 0; i < pixels.size(); ++i) {
		pixelsChar[i] = static_cast<unsigned char>(pixels[i] * 255.0f);
	}

	filesystem::create_directories("output");
	filesystem::path destination = string("output/" + fileName);
	string destinationStr = destination.string();
	const char* destinationCstr = destinationStr.c_str();

	stbi_flip_vertically_on_write(true);
	stbi_write_png(destinationCstr, camera.width, camera.height, 3, pixelsChar.data(), camera.width * 3);

}

void Scene::Draw_DepthPrepass(Shader& Depth_shader) {

	Depth_shader.Activate();
	generateDepthUniforms(Depth_shader, camera);

	// SET-UP SHADER PROGRAM TO WRITE ONLY DEPTH
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glClear(GL_DEPTH_BUFFER_BIT);

	// DRAW MESHES
	for (size_t i = 0; i < meshCollection.size(); ++i) {
		meshCollection[i]->Draw(Depth_shader, i, SSBOcomponent.meshTextures);
	}

}

void Scene::Draw_PathTracingPass(Shader& PathTracing_shader) {

	// UPDATE HIGHLIGHT BUFFER WITH IMGUI WINDOW (basically sets it to -1 incase the shader doesn't pass anything)
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, highlightedMeshBuffer);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &imguiWindow.highlightedMesh);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	PathTracing_shader.Activate();
	generatePathTracingUniforms(PathTracing_shader, camera);

	// RE-ALLOW COLOR WRITING BUT DISABLE DEPTH WRITING
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_EQUAL);
	glDepthMask(GL_FALSE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// FRAME BUFFER
	frameBuffer->Bind();
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, frameBuffer->texture->ID);
	frameBuffer->Unbind();

	// DRAW MESHES
	for (size_t i = 0; i < meshCollection.size(); ++i) {
		meshCollection[i]->Draw(PathTracing_shader, i, SSBOcomponent.meshTextures);
	}

	// SEND HIGHLIGHT BUFFER INFORMATION TO IMGUI WINDOW
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, highlightedMeshBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &imguiWindow.highlightedMesh);

}

void Scene::Draw_PostProcessingPass(Shader& PostProcessing_shader, GLFWwindow* window) {

	if (imguiWindow.highlightedMesh != -1) {

		Mesh* highlightedMesh = meshCollection[imguiWindow.highlightedMesh];
		gizmo.setPos(highlightedMesh->position);

	}

	PostProcessing_shader.Activate();
	generatePostProcessingUniforms(PostProcessing_shader, camera, window);
	
	// RE-USE SAME DEPTH
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// FRAME BUFFER
	frameBuffer->Bind();
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, frameBuffer->texture->ID);
	frameBuffer->Unbind();

	// DRAW MESHES
	for (size_t i = 0; i < meshCollection.size(); ++i) {
		meshCollection[i]->Draw(PostProcessing_shader, i, SSBOcomponent.meshTextures);
	}

	if (imguiWindow.highlightedMesh != -1) {

		glDisable(GL_DEPTH_TEST);
		gizmo.Draw(PostProcessing_shader);

	}

}

void Scene::generatePostProcessingUniforms(Shader& shader, Camera& camera, GLFWwindow* window) {

	int gizmoSelectionLoc = glGetUniformLocation(shader.ID, "u_gizmoSelection");
	glUniform1i(gizmoSelectionLoc, static_cast<int>(gizmo.getSelection(window, camera, imguiWindow.highlightedMesh != -1)));

	int debugHighlightedMeshLoc = glGetUniformLocation(shader.ID, "u_debugHighlightedMesh");
	glUniform1i(debugHighlightedMeshLoc, static_cast<int>(imguiWindow.highlightedMesh));

	int debugMinBrightnessLoc = glGetUniformLocation(shader.ID, "u_debugMinBrightness");
	glUniform1f(debugMinBrightnessLoc, imguiWindow.minBrightness);

	int debugMaxBrightnessLoc = glGetUniformLocation(shader.ID, "u_debugMaxBrightness");
	glUniform1f(debugMaxBrightnessLoc, imguiWindow.maxBrightness);

	camera.Matrix(shader, "u_camMatrix");
}

void Scene::generatePathTracingUniforms(Shader& shader, Camera& camera) {

	GLint mousePos[2] = { imguiWindow.mouseX, camera.height - imguiWindow.mouseY };
	int debugMousePosLoc = glGetUniformLocation(shader.ID, "u_debugMousePos");
	glUniform2iv(debugMousePosLoc, 1, mousePos);

	int debugMouseLeftClick = glGetUniformLocation(shader.ID, "u_debugMouseLeftClick");
	glUniform1i(debugMouseLeftClick, imguiWindow.mouseLeftClick);

	int camPosUniformLocation = glGetUniformLocation(shader.ID, "u_camPos");
	glUniform3f(camPosUniformLocation, camera.Position.x, camera.Position.y, camera.Position.z);

	int camOrientationUniformLocation = glGetUniformLocation(shader.ID, "u_camOrientation");
	glUniform3f(camOrientationUniformLocation, camera.Orientation.x, camera.Orientation.y, camera.Orientation.z);

	int debugModeLoc = glGetUniformLocation(shader.ID, "u_debugMode");
	glUniform1ui(debugModeLoc, static_cast<unsigned int> (imguiWindow.debugMode));

	int debugLambertianLoc = glGetUniformLocation(shader.ID, "u_debugLambertian");
	glUniform1i(debugLambertianLoc, imguiWindow.debugLambertian);

	int debugUniversalRoughnessLoc = glGetUniformLocation(shader.ID, "u_debugUniversalRoughness");
	glUniform1i(debugUniversalRoughnessLoc, imguiWindow.debugUniversalRoughness);

	int debugUniversalRoughnessAmountLoc = glGetUniformLocation(shader.ID, "u_debugUniversalRoughnessAmount");
	glUniform1f(debugUniversalRoughnessAmountLoc, imguiWindow.debugUniversalRoughnessAmount);

	int frameLoc = glGetUniformLocation(shader.ID, "u_currentSample");
	glUniform1ui(frameLoc, imguiWindow.currentSample);

	int maxBouncesLoc = glGetUniformLocation(shader.ID, "u_maxBounces");
	glUniform1ui(maxBouncesLoc, static_cast<unsigned int>(imguiWindow.maxBounces));

	int seedColorLoc = glGetUniformLocation(shader.ID, "u_seed");
	glUniform1ui(seedColorLoc, m_distrib(m_gen));

	camera.Matrix(shader, "u_camMatrix");
}

void Scene::generateDepthUniforms(Shader& shader, Camera& camera) {

	int camPosUniformLocation = glGetUniformLocation(shader.ID, "u_camPos");
	glUniform3f(camPosUniformLocation, camera.Position.x, camera.Position.y, camera.Position.z);

	camera.Matrix(shader, "u_camMatrix");
}

void Scene::link() {

	depthPrepassShader = new Shader("shaders/z_prepass.vert", "shaders/z_prepass.frag");
	pathTracingShader = new Shader("shaders/path_tracing.vert", "shaders/path_tracing.frag");
	postProcessingShader = new Shader("shaders/post_processing.vert", "shaders/post_processing.frag");

	SSBOcomponent.deleteSSBOs();

	SSBOcomponent.generateGlobalVertices(meshCollection);
	SSBOcomponent.generateGlobalIndices(meshCollection);
	SSBOcomponent.generateMeshTextures(meshCollection);
	SSBOcomponent.generateMeshHeader(meshCollection);
	SSBOcomponent.generateBoundingBoxes(meshCollection);

	SSBOcomponent.generateSSBOs(textureWidth, textureHeight);

	// COLOR NOISE
	colorNoise = new Texture(COLOR_NOISE);
	glActiveTexture(GL_TEXTURE1);
	colorNoise->Bind();

	// UNIFORMS (only those that need to be initalized once)
	int backgroundColorLoc = glGetUniformLocation(pathTracingShader->ID, "u_backgroundColor");
	glUniform3f(backgroundColorLoc, backgroundColor.x, backgroundColor.y, backgroundColor.z);

	// FRAME BUFFER
	frameBuffer = new FBO{ camera.width, camera.height, GL_TEXTURE2 };
	glBindImageTexture(2, frameBuffer->texture->ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// ACCUMULATION BUFFER
	accumulationBuffer = new FBO{ camera.width, camera.height, GL_TEXTURE3 };
	glBindImageTexture(3, accumulationBuffer->texture->ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// HIGHLIGHTED MESH BUFFER
	glGenBuffers(1, &highlightedMeshBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, highlightedMeshBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint), &imguiWindow.highlightedMesh, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, highlightedMeshBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

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

	if (IsKeyPressed(ImGuiKey_F)) {
		drawWindow = !drawWindow;
		currentSample = 0;
	}

	if (!drawWindow) { EndFrame();  return; }

	Begin("Debug");
	Separator();

	Text("Samples: %d/%d\t\tFT(ms): %.3f\t\tFPS: %d", currentSample, maxSamples, frameTime, static_cast<int>(1000 / frameTime));

	if (BeginTable("ShaderLayoutTable", 2)) {

		TableSetupColumn("Render");
		TableSetupColumn("Render Settings");
		TableHeadersRow();

		TableNextRow();

		BeginDisabled(previewAnimationPhase != RenderPhase::COMPLETE);

		// RENDER
		TableNextColumn();
		BeginDisabled(imageRenderPhase != RenderPhase::COMPLETE);
		if (Button("Render Image")) {
			imageRenderPhase = RenderPhase::WAITING;
		}
		EndDisabled();
		BeginDisabled(videoRenderPhase != RenderPhase::COMPLETE);
		SameLine();
		if (Button("Render Video")) {
			videoRenderPhase = RenderPhase::WAITING;
			scene->sceneAnimationFrame = 0;
		}

		SliderInt("Frames", &totalAnimationFrames, 1, 1000);
		EndDisabled();

		if (Button("Preview")) {
			previewAnimationPhase = RenderPhase::WAITING;
			debugMode = static_cast<int>(DebugTypes::ALBEDO);
		}
		SameLine();
		Text("Frame: %d/%d", animationFrame, totalAnimationFrames);

		// RENDER SETTINGS
		TableNextColumn();
		if (SliderInt("Bounces", &maxBounces, 1, MAX_BOUNCES)) currentSample = 0;
		if (SliderInt("Samples", &maxSamples, 1, MAX_SAMPLES)) currentSample = 0;
		if (Button("Clear Samples")) currentSample = 0;
		Spacing();
		if (Button("Pause")) pause = !pause;

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
		BeginDisabled(previewAnimationPhase != RenderPhase::COMPLETE);
		if (RadioButton("Disabled", &debugMode, static_cast<int>(DebugTypes::DISABLED))) currentSample = 0;
		Spacing();
		if (RadioButton("Albedo", &debugMode, static_cast<int>(DebugTypes::ALBEDO))) currentSample = 0;
		if (RadioButton("Normal", &debugMode, static_cast<int>(DebugTypes::NORMAL))) currentSample = 0;
		if (RadioButton("Roughness", &debugMode, static_cast<int>(DebugTypes::ROUGHNESS))) currentSample = 0;
		if (RadioButton("Metallic", &debugMode, static_cast<int>(DebugTypes::METALLIC))) currentSample = 0;
		Spacing();
		if (RadioButton("VertexNormal", &debugMode, static_cast<int>(DebugTypes::VERTEX_NORMAL))) currentSample = 0;
		EndDisabled();
		Spacing();
		Checkbox("Lambertian Shading", &debugLambertian);

		// POST PROCESSING
		TableNextColumn();
		SliderFloat("Min Brightness", &minBrightness, 0.0f, maxBrightness - 0.001f);
		SliderFloat("Max Brightness", &maxBrightness, minBrightness + 0.001f, 1.0f);

		EndTable();
	}

	// SCENE SETTINGS
	if (BeginTable("ShaderLayoutTable", 3)) {

		TableSetupColumn("Scene");
		TableHeadersRow();

		TableNextRow();

		std::filesystem::path filePath;

		TableNextColumn();

		filePath = "scenes/" + string(importName);
		bool isImportValid = (std::filesystem::exists(filePath) && filePath.extension().string() == SCENE_FILE_EXTENSION);
		PushStyleColor(ImGuiCol_FrameBg, (isImportValid ? validPathColor : invalidPathColor));
		InputTextWithHint("##importFileName", "scene.txt", importName, IM_ARRAYSIZE(importName));
		PopStyleColor(1);
		TableNextColumn();

		filePath = "scenes/" + string(exportName);
		bool isExportValid = (filePath.extension().string() == SCENE_FILE_EXTENSION);
		PushStyleColor(ImGuiCol_FrameBg, (isExportValid ? validPathColor : invalidPathColor));
		InputTextWithHint("##exportFileName", "scene.txt", exportName, IM_ARRAYSIZE(exportName));
		PopStyleColor(1);
		TableNextColumn();

		filePath = importOBJname;
		bool isOBJValid = (std::filesystem::exists(filePath) && filePath.extension().string() == ".obj");
		PushStyleColor(ImGuiCol_FrameBg, (isOBJValid ? validPathColor : invalidPathColor));
		InputTextWithHint("##importOBJName", "models/cube.obj", importOBJname, IM_ARRAYSIZE(importOBJname));
		PopStyleColor(1);

		TableNextRow();
		TableNextColumn();
		BeginDisabled(!isImportValid);
		if (Button("Import Scene")) {
			scene->importScene(importName);
			scene->link();
			strcpy(exportName, importName);
			importName[0] = 0x00;
		}
		EndDisabled();

		TableNextColumn();
		BeginDisabled(!isExportValid);
		if (Button("Export Scene")) {
			scene->exportScene(exportName);
			exportName[0] = 0x00;
		}
		EndDisabled();

		TableNextColumn();
		BeginDisabled(!isOBJValid);
		if (Button("Import OBJ")) {

			Mesh* mesh = new Mesh(importOBJname);
			scene->meshCollection.push_back(mesh);
			scene->link();
			importOBJname[0] = 0x00;

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
			if (DragFloat("##posX", &(scene->camera.Position.x), 0.5f)) currentSample = 0;
			PopStyleColor(1);
			TableNextColumn();
			PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
			if (DragFloat("##posY", &(scene->camera.Position.y), 0.5f)) currentSample = 0;
			PopStyleColor(1);
			TableNextColumn();
			PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
			if (DragFloat("##posZ", &(scene->camera.Position.z), 0.5f)) currentSample = 0;
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

		if (SliderFloat("FOV", &(scene->camera.FOVdeg), 0.0f, 90.0f)) currentSample = 0;
		TableNextColumn();
		if (SliderFloat("Near Plane", &(scene->camera.nearPlane), 0.0f, scene->camera.farPlane)) currentSample = 0;
		TableNextColumn();
		if (SliderFloat("Far Plane", &(scene->camera.farPlane), scene->camera.nearPlane, 10000.0f)) currentSample = 0;

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
					currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
				if (DragFloat("##posY", &(highlightedMesh->position.y), 0.5f)) {
					currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
				if (DragFloat("##posZ", &(highlightedMesh->position.z), 0.5f)) {
					currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				if (Button("Reset##positon")) {
					currentSample = 0;
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
					currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
				if (DragFloat("##yaw", &(highlightedMesh->rotation.y), glm::pi<float>() * 0.05f)) {
					currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
				if (DragFloat("##roll", &(highlightedMesh->rotation.z), glm::pi<float>() * 0.05f)) {
					currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				if (Button("Reset##rotation")) {
					highlightedMesh->rotation = glm::vec3(0.0f); 
					currentSample = 0;
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
					currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
				if (DragFloat("##scaleY", &(highlightedMesh->scale.y), 0.1f)) {
					currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
				if (DragFloat("##scaleZ", &(highlightedMesh->scale.z), 0.1f)) {
					currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				if (Button("Reset##scale")) {
					highlightedMesh->scale = glm::vec3(1.0f); 
					currentSample = 0;
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
					currentSample = 0;
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
					currentSample = 0;
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
					currentSample = 0;
					scene->SSBOcomponent.generateGlobalVertices(scene->meshCollection);
					scene->SSBOcomponent.updateVertexSSBO();
					for (Mesh* mesh : scene->meshCollection) {
						mesh->updateBuffers();
					}
				}
				PopStyleColor(1);
				TableNextColumn();
				if (Button("Reset##tint")) {
					currentSample = 0;
					highlightedMesh->tint = glm::vec3(1.0f);
				}
			}

			TableNextRow();
			TableNextColumn();
			Text("Emission");
			TableNextColumn();
			if (SliderFloat("##emission", &(highlightedMesh->emissive), 0.0f, 500.0f)) {
				currentSample = 0;
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
				this->highlightedMesh = -1;
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
		BeginDisabled(!debugUniversalRoughness);
		if (SliderFloat("Roughness", &debugUniversalRoughnessAmount, 0, 1)) currentSample = 0;
		EndDisabled();

		TableNextRow();

		TableNextColumn();
		if (Checkbox("Universal Roughness", &debugUniversalRoughness)) currentSample = 0;
		TableNextColumn();
		if (Button("Pause")) pause = !pause;


		TableNextColumn();

		EndTable();
	}

	// clear samples if window is moved or resized
	if (windowSize.x != GetWindowSize().x || windowSize.y != GetWindowSize().y) currentSample = 0;
	if (windowPosition.x != GetWindowPos().x || windowPosition.y != GetWindowPos().y) currentSample = 0;

	windowSize = GetWindowSize();
	windowPosition = GetWindowPos();



	End();

	Render();
	ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());

}