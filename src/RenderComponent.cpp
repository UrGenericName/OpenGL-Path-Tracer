#pragma once

#include "RenderComponent.h"

#include "SSBOcomponent.h"
#include "Mesh.h"
#include <vector>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <filesystem>
#include "Animation.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

void RenderComponent::AnimateComponents(DebugSettings& debugSettings, Camera& camera, SSBOcomponent& SSBOcomponent, vector<Mesh*>& meshCollection, unsigned int currentFrame) {

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

	debugSettings.currentSample = 0;

	SSBOcomponent.generateGlobalVertices(meshCollection);
	SSBOcomponent.updateVertexSSBO();

}

void RenderComponent::handleQueuedAnimationPreview(DebugSettings& debugSettings, Camera& camera, SSBOcomponent& SSBOcomponent, vector<Mesh*>& meshCollection) {

	static vector<Mesh*> meshCollectionDeepCopy;
	static Camera cameraCopy(camera);

	if (debugSettings.previewAnimationPhase == DebugSettings::RenderPhase::WAITING) {

		// save previous mesh data before animating
		for (auto mesh : meshCollection) {
			meshCollectionDeepCopy.push_back(new Mesh(*mesh));
		}

		// First frame
		AnimateComponents(debugSettings, camera, SSBOcomponent, meshCollection, animationFrame);

		debugSettings.previewAnimationPhase = DebugSettings::RenderPhase::RENDERING;

	}
	else if (debugSettings.previewAnimationPhase == DebugSettings::RenderPhase::RENDERING) {

		AnimateComponents(debugSettings, camera, SSBOcomponent, meshCollection, ++animationFrame);

		if (animationFrame == debugSettings.totalAnimationFrames) {

			// Reset mesh collection back to deep copy
			for (auto mesh : meshCollection) delete mesh;
			meshCollection = meshCollectionDeepCopy;
			meshCollectionDeepCopy.resize(0);

			SSBOcomponent.generateGlobalVertices(meshCollection); SSBOcomponent.updateVertexSSBO();
			SSBOcomponent.generateGlobalIndices(meshCollection); SSBOcomponent.updateIndicesSSBO();

			if (camera.animation != nullptr) camera = cameraCopy;

			animationFrame = 0;
			debugSettings.previewAnimationPhase = DebugSettings::RenderPhase::COMPLETE;
		}

	}

}

void RenderComponent::handleQueuedVideoRender(DebugSettings& debugSettings, Camera& camera, SSBOcomponent& SSBOcomponent, vector<Mesh*>& meshCollection) {

	static int lastHighlightedMeshValue;
	static bool lastDrawWindowValue;

	static string timeStamp;

	static vector<Mesh*> meshCollectionDeepCopy;
	static Camera cameraCopy(camera);

	if (debugSettings.videoRenderPhase == DebugSettings::RenderPhase::WAITING) {

		// save previous mesh data before animating
		for (auto mesh : meshCollection) {
			meshCollectionDeepCopy.push_back(new Mesh(*mesh));
		}

		lastDrawWindowValue = debugSettings.drawWindow;
		lastHighlightedMeshValue = debugSettings.highlightedMeshIndex;

		auto now = chrono::system_clock::now();
		auto local_time = chrono::current_zone()->to_local(now);
		timeStamp = format("{:%Y-%m-%d_%H-%M-%S}", local_time);

		filesystem::create_directories("output/video_" + timeStamp + "/");
		string fileName = "video_" + timeStamp + "/" + to_string(animationFrame) + ".png";

		debugSettings.drawWindow = false;
		debugSettings.highlightedMeshIndex = -1;

		// First frame
		AnimateComponents(debugSettings, camera, SSBOcomponent, meshCollection, animationFrame);

		debugSettings.videoRenderPhase = DebugSettings::RenderPhase::RENDERING;

	}
	else if (debugSettings.videoRenderPhase == DebugSettings::RenderPhase::RENDERING) {

		if (debugSettings.currentSample == debugSettings.maxSamples) {

			string fileName = "video_" + timeStamp + "/" + to_string(animationFrame) + ".png";

			AnimateComponents(debugSettings, camera, SSBOcomponent, meshCollection, ++animationFrame);
			renderImage(fileName, camera);

			if (animationFrame == (debugSettings.totalAnimationFrames + 1)) {

				// Reset mesh collection back to deep copy
				for (auto mesh : meshCollection) delete mesh;
				meshCollection = meshCollectionDeepCopy;
				meshCollectionDeepCopy.resize(0);

				SSBOcomponent.generateGlobalVertices(meshCollection); SSBOcomponent.updateVertexSSBO();
				SSBOcomponent.generateGlobalIndices(meshCollection); SSBOcomponent.updateIndicesSSBO();

				if (camera.animation != nullptr) camera = cameraCopy;

				animationFrame = 0;
				debugSettings.drawWindow = lastDrawWindowValue;
				debugSettings.highlightedMeshIndex = lastHighlightedMeshValue;
				debugSettings.videoRenderPhase = DebugSettings::RenderPhase::COMPLETE;
			}

		}
	}

}

void RenderComponent::handleQueuedImageRender(DebugSettings& debugSettings, Camera& camera) {

	static int lastHighlightedMeshValue;
	static bool lastDrawWindowValue;

	if (debugSettings.imageRenderPhase == DebugSettings::RenderPhase::WAITING) {

		lastDrawWindowValue = debugSettings.drawWindow;
		lastHighlightedMeshValue = debugSettings.highlightedMeshIndex;

		debugSettings.drawWindow = false;
		debugSettings.highlightedMeshIndex = -1;
		debugSettings.imageRenderPhase = DebugSettings::RenderPhase::RENDERING;

	}
	else if (debugSettings.imageRenderPhase == DebugSettings::RenderPhase::RENDERING) {

		if (debugSettings.currentSample == debugSettings.maxSamples) {

			auto now = chrono::system_clock::now();
			auto local_time = chrono::current_zone()->to_local(now);
			string timeStamp = format("{:%Y-%m-%d_%H-%M-%S}", local_time);
			string fileName = "output_" + timeStamp + ".png";

			renderImage(fileName, camera);

			debugSettings.drawWindow = lastDrawWindowValue;
			debugSettings.highlightedMeshIndex = lastHighlightedMeshValue;

			debugSettings.imageRenderPhase = DebugSettings::RenderPhase::COMPLETE;

		}

	}

}

void RenderComponent::renderImage(std::string fileName, Camera& camera) {

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