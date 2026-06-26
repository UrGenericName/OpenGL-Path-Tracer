#pragma once

#include "Material.h"

class DebugSettings {
public:

	enum class DebugTypes {

		DISABLED,
		ALBEDO,
		NORMAL,
		ROUGHNESS,
		METALLIC,
		VERTEX_NORMAL

	};

	enum class RenderPhase {

		WAITING,
		RENDERING,
		COMPLETE

	};

	// DEBUG
	int debugMode = static_cast<int>(DebugTypes::DISABLED);
	bool debugLambertian = true;
	bool debugUniversalRoughness = false;
	float debugUniversalRoughnessAmount = 1.0f;

	// PATH-TRACING
	int maxBounces = 8;
	int maxSamples = 32;
	float minBrightness = 0.0f;
	float maxBrightness = 0.5f;
	unsigned int currentSample = 0;

	// HIGHLIGHTED MESH
	int highlightedMeshIndex = -1;
	bool mouseLeftClick = false;
	double mouseX;
	double mouseY;

	// IMPORT / EXPORT
	inline static char importName[128] = "scene.txt";
	inline static char exportName[128] = "";
	inline static char importOBJname[128] = "";

	// MATERIALS
	inline static char albedo[128] = "";
	inline static char normal[128] = "";
	inline static char roughness[128] = "";
	inline static char metallic[128] = "";

	// RENDER
	RenderPhase imageRenderPhase = RenderPhase::COMPLETE;
	RenderPhase videoRenderPhase = RenderPhase::COMPLETE;
	RenderPhase previewAnimationPhase = RenderPhase::COMPLETE;
	int totalAnimationFrames = 60;

	// WINDOW
	bool pause = false;
	bool usingDebugWindow = false;
	bool drawWindow = true;

};