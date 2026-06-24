#pragma once

#include <string>
#include "ImguiWindow.h"
#include "Camera.h"

class RenderComponent {
public:

	unsigned int sceneAnimationFrame = 0;

	void handleQueuedAnimationPreview(ImguiWindow& imguiWindow, Camera& camera, SSBOcomponent& SSBOcomponent, vector<Mesh*>& meshCollection);
	void handleQueuedVideoRender(ImguiWindow& imguiWindow, Camera& camera, SSBOcomponent& SSBOcomponent, vector<Mesh*>& meshCollection);
	void handleQueuedImageRender(ImguiWindow& imguiWindow, Camera& camera);
	void renderImage(std::string fileName, Camera& camera);

private:

	void AnimateComponents(ImguiWindow& imguiWindow, Camera& camera, SSBOcomponent& SSBOcomponent, vector<Mesh*>& meshCollection, unsigned int currentFrame);

};