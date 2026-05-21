#include "Material.h"

Material::Material(Texture* albedo, Texture* normal, Texture* roughness, Texture* metallic) {

	this->albedo = albedo;
	this->normal = normal;
	this->roughness = roughness;
	this->metallic = metallic;

	if (albedo == nullptr) this->albedo = new Texture(MISSING_ALBEDO, GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
	if (normal == nullptr) this->normal = new Texture(MISSING_NORMAL, GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
	if (roughness == nullptr) this->roughness = new Texture(MISSING_ROUGHNESS, GL_TEXTURE_2D, GL_TEXTURE0, GL_RED, GL_UNSIGNED_BYTE);
	if (metallic == nullptr) this->metallic = new Texture(MISSING_METALLIC, GL_TEXTURE_2D, GL_TEXTURE0, GL_RED, GL_UNSIGNED_BYTE);

}

Material::Material(const char* albedo, const char* normal, const char* roughness, const char* metallic) {

	// ALBEDO
	if (albedo == nullptr) {
		this->albedo = new Texture(MISSING_ALBEDO, GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
	} else {
		this->albedo = new Texture(albedo, GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
	}

	// NORMAL
	if (normal == nullptr) {
		this->normal = new Texture(MISSING_NORMAL, GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
	} else {
		this->normal = new Texture(normal, GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
	}

	// ROUGHNESS
	if (roughness == nullptr) {
		this->roughness = new Texture(MISSING_ROUGHNESS, GL_TEXTURE_2D, GL_TEXTURE0, GL_RED, GL_UNSIGNED_BYTE);
	}
	else {
		this->roughness = new Texture(roughness, GL_TEXTURE_2D, GL_TEXTURE0, GL_RED, GL_UNSIGNED_BYTE);
	}

	// METALIC
	if (metallic == nullptr) {
		this->metallic = new Texture(MISSING_METALLIC, GL_TEXTURE_2D, GL_TEXTURE0, GL_RED, GL_UNSIGNED_BYTE);
	}
	else {
		this->metallic = new Texture(metallic, GL_TEXTURE_2D, GL_TEXTURE0, GL_RED, GL_UNSIGNED_BYTE);
	}

}