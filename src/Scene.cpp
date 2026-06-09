#include "Scene.h"

Scene::Scene(unsigned int width, unsigned int height) : textureWidth(width), textureHeight(height) {}

Scene::~Scene() {

	for (Mesh* mesh : meshCollection) {
		delete mesh;
	}

}

void Scene::Draw(Shader& shader, Camera& camera, GLFWwindow* window) {

	shader.Activate();

	glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);	// sets the "clear" color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// actually clears the background with color

	camera.Inputs(window, imguiWindow);
	camera.updateMatrix(45.0f, 0.1f, 100.0f);

	int camPosUniformLocation = glGetUniformLocation(shader.ID, "camPos");
	glUniform3f(camPosUniformLocation, camera.Position.x, camera.Position.y, camera.Position.z);

	int camOrientationUniformLocation = glGetUniformLocation(shader.ID, "camOrientation");
	glUniform3f(camOrientationUniformLocation, camera.Orientation.x, camera.Orientation.y, camera.Orientation.z);

	int seedColorLoc = glGetUniformLocation(shader.ID, "seed");
	glUniform1ui(seedColorLoc, m_distrib(m_gen));

	int debugModeLoc = glGetUniformLocation(shader.ID, "debugMode");
	glUniform1ui(debugModeLoc, static_cast<unsigned int> (imguiWindow.debugMode) );

	int debugLambertianLoc = glGetUniformLocation(shader.ID, "debugLambertian");
	glUniform1i(debugLambertianLoc, imguiWindow.debugLambertian);

	int debugBouncesLoc = glGetUniformLocation(shader.ID, "BOUNCES");
	glUniform1ui(debugBouncesLoc, static_cast<unsigned int> (imguiWindow.debugBounces));

	int debugSamplesLoc = glGetUniformLocation(shader.ID, "SAMPLES");
	glUniform1ui(debugSamplesLoc, static_cast<unsigned int> (imguiWindow.debugSamples));

	int debugForceRoughnessLoc = glGetUniformLocation(shader.ID, "debugForceRoughness");
	glUniform1i(debugForceRoughnessLoc, imguiWindow.debugForceRoughness);

	int debugForceRoughnessAmountLoc = glGetUniformLocation(shader.ID, "debugForceRoughnessAmount");
	glUniform1f(debugForceRoughnessAmountLoc, imguiWindow.debugForceRoughnessAmount);

	for (size_t i = 0; i < meshCollection.size(); ++i) {
		meshCollection[i]->Draw(shader, camera, i, meshTexturesOutput);
	}

	imguiWindow.drawImgui();

}

void Scene::generateSSBOs(unsigned int width, unsigned int height, GLuint& vertexSSBO, GLuint& indicesSSBO, GLuint& meshTextureSSBO, GLuint& meshHeaderSSBO, GLuint& textureArray, std::vector<glm::vec4>& meshTexturesOutput) {

	std::vector<Vertex> globalVertices;
	std::vector<GLuint> globalIndices;
	std::vector<glm::vec4> meshTextures; // <albedoIndex, normalIndex, roughnessIndex, metallicIndex>
	std::vector<glm::vec4> meshHeader;	// <indicesStartPointer, indicesSize, emissiveValue>

	std::set<string> texturePool;

	size_t indexPointer = 0;
	for (Mesh* mesh : meshCollection) {

		// VERTICES
		globalVertices.insert(globalVertices.end(), mesh->vertices.begin(), mesh->vertices.end());

		// INDICES
		std::vector<GLuint> tempIndices = mesh->indices;
		for (size_t i = 0; i < tempIndices.size(); ++i) tempIndices[i] += indexPointer;
		globalIndices.insert(globalIndices.end(), tempIndices.begin(), tempIndices.end());

		// MESH-HEADER
		meshHeader.push_back(glm::vec4(indexPointer, mesh->indices.size(), mesh->emissive, 0.0f));

		// TEXTURE POOL
		texturePool.insert(mesh->material->albedo);
		texturePool.insert(mesh->material->normal);
		texturePool.insert(mesh->material->roughness);
		texturePool.insert(mesh->material->metallic);

		// Loop increment (used to convert local indices to global indices)
		indexPointer += mesh->vertices.size();

	}

	// calculates the positional index for each mesh texture
	for (Mesh* mesh : meshCollection) {

		glm::vec4 texturePointers;
		texturePointers.x = std::distance(texturePool.begin(), texturePool.find(mesh->material->albedo));
		texturePointers.y = std::distance(texturePool.begin(), texturePool.find(mesh->material->normal));
		texturePointers.z = std::distance(texturePool.begin(), texturePool.find(mesh->material->roughness));
		texturePointers.w = std::distance(texturePool.begin(), texturePool.find(mesh->material->metallic));

		meshTextures.push_back(texturePointers);

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

	// 2D-TEXTURE ARRAY
	glGenTextures(1, &textureArray);

	GLuint maxTextureCount = texturePool.size();

	std::vector<string> texturePoolVector(texturePool.begin(), texturePool.end());
	Texture::loadTextureArray(textureArray, texturePoolVector, width, height);

	// output this (used for drawing)
	meshTexturesOutput = meshTextures;

}

void Scene::link(Shader& shader) {

	shader.Activate();

	// TEXTURE ARRAY
	generateSSBOs(textureWidth, textureHeight, vertexSSBO, indicesSSBO, textureMeshSSBO, meshHeaderSSBO, textureArray, meshTexturesOutput);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);

	// COLOR NOISE
	colorNoise = new Texture(COLOR_NOISE);
	glActiveTexture(GL_TEXTURE1);
	colorNoise->texUnit(shader, "colorNoise", 1);
	colorNoise->Bind();

	// UNIFORMS
	GLuint texturePoolUniformLoc = glGetUniformLocation(shader.ID, "texturePool");
	glUniform1i(texturePoolUniformLoc, 0);

	int backgroundColorLoc = glGetUniformLocation(shader.ID, "backgroundColor");
	glUniform3f(backgroundColorLoc, backgroundColor.x, backgroundColor.y, backgroundColor.z);

}