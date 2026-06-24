#pragma once

#include "RenderComponent.h"

#include "SSBOcomponent.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

void RenderComponent::AnimateComponents(ImguiWindow& imguiWindow, Camera& camera, SSBOcomponent& SSBOcomponent, vector<Mesh*>& meshCollection, unsigned int currentFrame) {

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

void RenderComponent::handleQueuedAnimationPreview(ImguiWindow& imguiWindow, Camera& camera, SSBOcomponent& SSBOcomponent, vector<Mesh*>& meshCollection) {

	static vector<Mesh*> meshCollectionDeepCopy;
	static Camera cameraCopy(camera);

	if (imguiWindow.previewAnimationPhase == ImguiWindow::RenderPhase::WAITING) {

		// save previous mesh data before animating
		for (auto mesh : meshCollection) {
			meshCollectionDeepCopy.push_back(new Mesh(*mesh));
		}

		// First frame
		AnimateComponents(imguiWindow, camera, SSBOcomponent, meshCollection, sceneAnimationFrame);

		imguiWindow.previewAnimationPhase = ImguiWindow::RenderPhase::RENDERING;

	}
	else if (imguiWindow.previewAnimationPhase == ImguiWindow::RenderPhase::RENDERING) {

		AnimateComponents(imguiWindow, camera, SSBOcomponent, meshCollection, ++sceneAnimationFrame);

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

void RenderComponent::handleQueuedVideoRender(ImguiWindow& imguiWindow, Camera& camera, SSBOcomponent& SSBOcomponent, vector<Mesh*>& meshCollection) {

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
		AnimateComponents(imguiWindow, camera, SSBOcomponent, meshCollection, sceneAnimationFrame);

		imguiWindow.videoRenderPhase = ImguiWindow::RenderPhase::RENDERING;

	}
	else if (imguiWindow.videoRenderPhase == ImguiWindow::RenderPhase::RENDERING) {

		if (imguiWindow.currentSample == imguiWindow.maxSamples) {

			string fileName = "video_" + timeStamp + "/" + to_string(sceneAnimationFrame) + ".png";

			AnimateComponents(imguiWindow, camera, SSBOcomponent, meshCollection, ++sceneAnimationFrame);
			renderImage(fileName, camera);

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

void RenderComponent::handleQueuedImageRender(ImguiWindow& imguiWindow, Camera& camera) {

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

			renderImage(fileName, camera);

			imguiWindow.drawWindow = lastDrawWindowValue;
			imguiWindow.highlightedMesh = lastHighlightedMeshValue;

			imguiWindow.imageRenderPhase = ImguiWindow::RenderPhase::COMPLETE;

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