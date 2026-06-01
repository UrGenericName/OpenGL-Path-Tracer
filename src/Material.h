#pragma once

#include <glad/glad.h>
#include <string>

#include "Texture.h";

#define MISSING_ALBEDO "textures/default/albedo_default.png"
#define MISSING_NORMAL "textures/default/normal_default.png"
#define MISSING_ROUGHNESS "textures/default/roughness_default.png"
#define MISSING_METALLIC "textures/default/metallic_default.png"

using namespace std;

class Material {
public:

	string albedo;
	string normal;
	string roughness;
	string metallic;

	Material(string albedo = MISSING_ALBEDO, string normal = MISSING_NORMAL, string roughness = MISSING_ROUGHNESS, string metallic = MISSING_METALLIC);

};