#pragma once

#include "Texture.h"

#include <stb/stb_image.h>
#include <filesystem>

Texture::Texture(GLenum texType, GLenum slot) {

	type = texType;

	glGenTextures(1, &ID);
	glActiveTexture(slot);
	glBindTexture(type, ID);

}

Texture::Texture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType, GLint filterParam, GLint wrapParam) {

	type = texType;

	int widthImg, heightImg, numColCh;
	stbi_set_flip_vertically_on_load(true);

	// Checks to make sure file name is valid (debugging is a pain otherwise)
	if (!std::filesystem::exists(image)) throw std::invalid_argument("Texture file path does not exist");

	unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);

	glGenTextures(1, &ID);
	glActiveTexture(slot);
	glBindTexture(type, ID);

	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, filterParam);
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, filterParam);

	glTexParameteri(type, GL_TEXTURE_WRAP_S, wrapParam);
	glTexParameteri(type, GL_TEXTURE_WRAP_T, wrapParam);

	glTexImage2D(type, 0, GL_RGBA, widthImg, heightImg, 0, format, pixelType, bytes);
	glGenerateMipmap(type);

	stbi_image_free(bytes);
	glBindTexture(type, 0);

}

void Texture::texUnit(Shader &shader, const char* uniform, GLuint unit) {
	
	GLuint texUni = glGetUniformLocation(shader.ID, uniform);
	shader.Activate();
	glUniform1i(texUni, unit);

}


void Texture::Bind() {
	glBindTexture(type, ID);
}

void Texture::Unbind() {
	glBindTexture(type, 0);
}

void Texture::Delete() {
	glDeleteTextures(1, &ID);
}

void Texture::loadTextureArray(GLuint textureArrayID, std::vector<std::string>& textures, int width, int height) {

	int maxTextureCount = textures.size();
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGB8, width, height, maxTextureCount);

	stbi_set_flip_vertically_on_load(true);

	for (int i = 0; i < textures.size(); ++i) {

		const char* texturePath = textures[i].c_str();
		int widthImg, heightImg, numColCh;

		// Checks to make sure file name is valid (debugging is a pain otherwise)
		if (!std::filesystem::exists(texturePath)) throw std::invalid_argument("Texture file path does not exist");

		unsigned char* bytes = stbi_load(texturePath, &widthImg, &heightImg, &numColCh, 3);

		if (widthImg != width || heightImg != height) throw std::invalid_argument("Texture \'" + textures[i] + "\' does not conform to " + std::to_string(width) + "x" + std::to_string(height));

		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, widthImg, heightImg, 1, GL_RGB, GL_UNSIGNED_BYTE, bytes);

		stbi_image_free(bytes);
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

}