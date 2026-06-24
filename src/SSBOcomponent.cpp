#pragma once

#include "SSBOcomponent.h"

SSBOcomponent::SSBOcomponent() {}

void SSBOcomponent::updateVertexSSBO() {

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
	glBufferSubData(
		GL_SHADER_STORAGE_BUFFER,
		0,
		globalVertices.size() * sizeof(globalVertices[0]),
		globalVertices.data()
	);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void SSBOcomponent::updateIndicesSSBO() {

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, indicesSSBO);
	glBufferSubData(
		GL_SHADER_STORAGE_BUFFER,
		0,
		globalIndices.size() * sizeof(globalIndices[0]),
		globalIndices.data()
	);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void SSBOcomponent::updateMeshTexturesSSBO() {

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshTextureSSBO);
	glBufferSubData(
		GL_SHADER_STORAGE_BUFFER,
		0,
		meshTextures.size() * sizeof(meshTextures[0]),
		meshTextures.data()
	);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void SSBOcomponent::updateMeshHeaderSSBO() {

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshHeaderSSBO);
	glBufferSubData(
		GL_SHADER_STORAGE_BUFFER,
		0,
		meshHeader.size() * sizeof(meshHeader[0]),
		meshHeader.data()
	);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void SSBOcomponent::updateBoundingBoxesSSBO() {

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, boundingBoxesSSBO);
	glBufferSubData(
		GL_SHADER_STORAGE_BUFFER,
		0,
		boundingBoxes.size() * sizeof(boundingBoxes[0]),
		boundingBoxes.data()
	);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void SSBOcomponent::generateGlobalVertices(vector<Mesh*>& meshCollection) {

	globalVertices.resize(0);

	for (Mesh* mesh : meshCollection) {

		// VERTICES
		for (auto vertex : mesh->vertices) {

			vertex.position = mesh->getModelMatrix() * vertex.position;
			vertex.normal = normalize(mesh->getRotationMatrix() * mesh->getScaleMatrix() * vertex.normal);
			vertex.color = glm::vec4(mesh->tint, 1.0f);
			globalVertices.push_back(vertex);

		}

	}

}

void SSBOcomponent::generateGlobalIndices(vector<Mesh*>& meshCollection) {

	globalIndices.resize(0);

	size_t indexPointer = 0;
	for (Mesh* mesh : meshCollection) {

		// INDICES
		vector<GLuint> tempIndices = mesh->indices;
		for (size_t i = 0; i < tempIndices.size(); ++i) tempIndices[i] += indexPointer;
		globalIndices.insert(globalIndices.end(), tempIndices.begin(), tempIndices.end());

		// Loop increment (used to convert local indices to global indices)
		indexPointer += mesh->vertices.size();

	}

}

void SSBOcomponent::generateMeshTextures(vector<Mesh*>& meshCollection) {

	meshTextures.resize(0);
	texturePool.clear();

	for (Mesh* mesh : meshCollection) {

		// TEXTURE POOL
		texturePool.insert(mesh->material->albedo);
		texturePool.insert(mesh->material->normal);
		texturePool.insert(mesh->material->roughness);
		texturePool.insert(mesh->material->metallic);

	}

	// calculates the positional index for each mesh texture
	for (Mesh* mesh : meshCollection) {

		glm::vec4 texturePointers;
		texturePointers.x = distance(texturePool.begin(), texturePool.find(mesh->material->albedo));
		texturePointers.y = distance(texturePool.begin(), texturePool.find(mesh->material->normal));
		texturePointers.z = distance(texturePool.begin(), texturePool.find(mesh->material->roughness));
		texturePointers.w = distance(texturePool.begin(), texturePool.find(mesh->material->metallic));

		meshTextures.push_back(texturePointers);

	}

}

void SSBOcomponent::generateMeshHeader(vector<Mesh*>& meshCollection) {

	meshHeader.resize(0);

	size_t indexPointer = 0;
	for (Mesh* mesh : meshCollection) {

		// MESH-HEADER
		meshHeader.push_back(glm::vec4(indexPointer, mesh->indices.size(), mesh->emissive, 0.0f));

		// Loop increment (used to convert local indices to global indices)
		indexPointer += mesh->vertices.size();

	}

}

void SSBOcomponent::generateBoundingBoxes(vector<Mesh*>& meshCollection) {

	boundingBoxes.resize(0);

	for (Mesh* mesh : meshCollection) {

		boundingBoxes.push_back(mesh->getBoundingBox());

	}

}

void SSBOcomponent::generateSSBOs(unsigned int texture_width, unsigned int texture_height) {

	// GENERATE VERTEX SSBO
	glGenBuffers(1, &vertexSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);

	glBufferData(
		GL_SHADER_STORAGE_BUFFER,
		globalVertices.size() * sizeof(globalVertices[0]),
		globalVertices.data(),
		GL_STATIC_DRAW
	);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// GENERATE INDICES SSBO
	glGenBuffers(1, &indicesSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, indicesSSBO);

	glBufferData(
		GL_SHADER_STORAGE_BUFFER,
		globalIndices.size() * sizeof(globalIndices[0]),
		globalIndices.data(),
		GL_STATIC_DRAW
	);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indicesSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// MESH-TEXTURES SSBO
	glGenBuffers(1, &meshTextureSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshTextureSSBO);

	glBufferData(
		GL_SHADER_STORAGE_BUFFER,
		meshTextures.size() * sizeof(meshTextures[0]),
		meshTextures.data(),
		GL_STATIC_DRAW
	);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, meshTextureSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// MESH-HEADER SSBO
	glGenBuffers(1, &meshHeaderSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshHeaderSSBO);

	glBufferData(
		GL_SHADER_STORAGE_BUFFER,
		meshHeader.size() * sizeof(meshHeader[0]),
		meshHeader.data(),
		GL_STATIC_DRAW
	);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, meshHeaderSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// GENERATE BOUNDING BOXES SSBO
	glGenBuffers(1, &boundingBoxesSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, boundingBoxesSSBO);

	glBufferData(
		GL_SHADER_STORAGE_BUFFER,
		boundingBoxes.size() * sizeof(boundingBoxes[0]),
		boundingBoxes.data(),
		GL_STATIC_DRAW
	);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, boundingBoxesSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// 2D-TEXTURE ARRAY
	glGenTextures(1, &textureArray);

	GLuint maxTextureCount = texturePool.size();

	vector<string> texturePoolVector(texturePool.begin(), texturePool.end());
	Texture::loadTextureArray(textureArray, texturePoolVector, texture_width, texture_height);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);

}

void SSBOcomponent::deleteSSBOs() {

	GLuint buffersToDelete[] = { vertexSSBO, indicesSSBO, meshTextureSSBO, meshHeaderSSBO, textureArray };
	glDeleteBuffers(5, buffersToDelete);

}