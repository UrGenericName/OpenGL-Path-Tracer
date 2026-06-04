#pragma once

#include <glad/glad.h>
#include <string>

#include "Texture.h";

#define DEFAULT_ALBEDO "textures/core/default/albedo_default.png"
#define DEFAULT_NORMAL "textures/core/default/normal_default.png"
#define DEFAULT_ROUGHNESS "textures/core/default/roughness_default.png"
#define DEFAULT_METALLIC "textures/core/default/metallic_default.png"

using namespace std;

class Material {
public:

	string albedo;
	string normal;
	string roughness;
	string metallic;

	Material(string albedo = DEFAULT_ALBEDO, string normal = DEFAULT_NORMAL, string roughness = DEFAULT_ROUGHNESS, string metallic = DEFAULT_METALLIC);

};