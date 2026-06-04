#pragma once

#include <glad/glad.h>
#include <string>

#include "Texture.h";

#define DEFAULT_ALBEDO "textures/default/albedo_default.png"
#define DEFAULT_NORMAL "textures/default/normal_default.png"
#define DEFAULT_ROUGHNESS "textures/default/roughness_default.png"
#define DEFAULT_METALLIC "textures/default/metallic_default.png"

using namespace std;

class Material {
public:

	string albedo;
	string normal;
	string roughness;
	string metallic;

	Material(string albedo = DEFAULT_ALBEDO, string normal = DEFAULT_NORMAL, string roughness = DEFAULT_ROUGHNESS, string metallic = DEFAULT_METALLIC);

};