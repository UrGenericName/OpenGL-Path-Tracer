#include "Scene.h"

Scene::Scene() {}

Scene::~Scene() {

	for (Mesh* mesh : meshCollection) {
		delete mesh;
	}

}

void Scene::Draw(Shader& shader, Camera& camera) {

	for (Mesh* mesh : meshCollection) {
		mesh->Draw(shader, camera);
	}

}

void Scene::generateSSBOs(GLuint& vertexSSBO, GLuint& indicesSSBO, GLuint& textureHandelsSSBO, GLuint& meshHeaderSSBO) {

	std::vector<Vertex> globalVertices;
	std::vector<GLuint> globalIndices;
	std::vector<GLuint64> globalTextureHandles;
	std::vector<glm::vec4> meshHeader;

	for (Mesh* mesh : meshCollection) {

		globalVertices.insert(globalVertices.end(), mesh->vertices.begin(), mesh->vertices.end());
		globalIndices.insert(globalIndices.end(), mesh->indices.begin(), mesh->indices.end());

		globalTextureHandles.push_back(glGetTextureHandleARB(mesh->material->albedo->ID));
		globalTextureHandles.push_back(glGetTextureHandleARB(mesh->material->normal->ID));
		globalTextureHandles.push_back(glGetTextureHandleARB(mesh->material->roughness->ID));
		globalTextureHandles.push_back(glGetTextureHandleARB(mesh->material->metallic->ID));

		meshHeader.push_back(glm::vec4(mesh->vertices.size(), mesh->indices.size(), mesh->emissive, 0.0f));

	}

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

	// TEXTURE HANDLES SSBO
	glGenBuffers(1, &textureHandelsSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureHandelsSSBO);

	glBufferData(
		GL_SHADER_STORAGE_BUFFER,
		globalTextureHandles.size() * sizeof(globalTextureHandles[0]),
		globalTextureHandles.data(),
		GL_STATIC_DRAW
	);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, textureHandelsSSBO);

	// MESH-HEADER SSBO
	glGenBuffers(1, &meshHeaderSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshHeaderSSBO);

	glBufferData(
		GL_SHADER_STORAGE_BUFFER,
		meshHeader.size() * sizeof(meshHeader[0]),
		meshHeader.data(),
		GL_STATIC_DRAW
	);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, meshHeaderSSBO);

}