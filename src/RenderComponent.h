#pragma once

#include <string>
#include "DebugSettings.h"
#include "SSBOcomponent.h"
#include "Camera.h"

class RenderComponent {
public:

	unsigned int animationFrame = 0;

	void handleQueuedAnimationPreview(DebugSettings& debugSettings, Camera& camera, SSBOcomponent& SSBOcomponent, vector<Mesh*>& meshCollection);
	void handleQueuedVideoRender(DebugSettings& debugSettings, Camera& camera, SSBOcomponent& SSBOcomponent, vector<Mesh*>& meshCollection);
	void handleQueuedImageRender(DebugSettings& debugSettings, Camera& camera);
	void renderImage(std::string fileName, Camera& camera);

private:

	void AnimateComponents(DebugSettings& debugSettings, Camera& camera, SSBOcomponent& SSBOcomponent, vector<Mesh*>& meshCollection, unsigned int currentFrame);

};