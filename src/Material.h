#pragma once

#include <glad/glad.h>

#include "Texture.h";

#define MISSING_ALBEDO "textures/default/albedo_default.png"
#define MISSING_NORMAL "textures/default/normal_default.png"
#define MISSING_ROUGHNESS "textures/default/roughness_default.png"
#define MISSING_METALLIC "textures/default/metallic_default.png"

class Material {
public:

	Texture* albedo;
	Texture* normal;
	Texture* roughness;
	Texture* metallic;

	Material(Texture* albedo = nullptr, Texture* normal = nullptr, Texture* roughness = nullptr, Texture* metallic = nullptr);
	Material(const char* albedo, const char* normal = nullptr, const char* roughness = nullptr, const char* metallic = nullptr);

};