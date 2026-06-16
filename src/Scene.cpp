#include "Scene.h"

Scene::Scene(Camera& i_camera, unsigned int i_textureWidth, unsigned int i_textureHeight) : camera(i_camera), textureWidth(i_textureWidth), textureHeight(i_textureHeight) {

	

}

Scene::~Scene() {

	for (Mesh* mesh : meshCollection) {
		delete mesh;
	}

}

void Scene::Draw(GLFWwindow* window) {

	camera.Inputs(window, imguiWindow);
	camera.updateMatrix(45.0f, 0.1f, 100.0f);

	// CLEAR BACKGROUND
	glViewport(0, 0, camera.width, camera.height);
	glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Draw_DepthPrepass(*depthPrepassShader);
	Draw_PathTracingPass(*pathTracingShader);

	if (imguiWindow.currentSample != imguiWindow.maxSamples) ++imguiWindow.currentSample;

	imguiWindow.drawImgui();
}

void Scene::Draw_DepthPrepass(Shader& depth_shader) {

	depth_shader.Activate();
	generateDepthUniforms(depth_shader, camera);

	// SET-UP SHADER PROGRAM TO WRITE ONLY DEPTH
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glClear(GL_DEPTH_BUFFER_BIT);

	// DRAW MESHES
	for (size_t i = 0; i < meshCollection.size(); ++i) {
		meshCollection[i]->Draw(depth_shader, camera, i, meshTexturesOutput);
	}
}

void Scene::Draw_PathTracingPass(Shader& PathTracing_shader) {

	PathTracing_shader.Activate();
	generatePathTracingUniforms(PathTracing_shader, camera);

	// RE-ALLOW COLOR WRITING BUT DISABLE DEPTH WRITING
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_EQUAL);
	glDepthMask(GL_FALSE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// FRAME BUFFER
	frameBuffer->Bind();
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, frameBuffer->texture->ID);
	frameBuffer->Unbind();

	// DRAW MESHES
	for (size_t i = 0; i < meshCollection.size(); ++i) {
		meshCollection[i]->Draw(PathTracing_shader, camera, i, meshTexturesOutput);
	}

}

void Scene::generateDepthUniforms(Shader& shader, Camera& camera) {

	camera.Matrix(shader, "u_camMatrix");
}

void Scene::generatePathTracingUniforms(Shader& shader, Camera& camera) {

	int camPosUniformLocation = glGetUniformLocation(shader.ID, "u_camPos");
	glUniform3f(camPosUniformLocation, camera.Position.x, camera.Position.y, camera.Position.z);

	int camOrientationUniformLocation = glGetUniformLocation(shader.ID, "u_camOrientation");
	glUniform3f(camOrientationUniformLocation, camera.Orientation.x, camera.Orientation.y, camera.Orientation.z);

	int seedColorLoc = glGetUniformLocation(shader.ID, "u_seed");
	glUniform1ui(seedColorLoc, m_distrib(m_gen));

	int debugModeLoc = glGetUniformLocation(shader.ID, "u_debugMode");
	glUniform1ui(debugModeLoc, static_cast<unsigned int> (imguiWindow.debugMode));

	int debugLambertianLoc = glGetUniformLocation(shader.ID, "u_debugLambertian");
	glUniform1i(debugLambertianLoc, imguiWindow.debugLambertian);

	int debugForceRoughnessLoc = glGetUniformLocation(shader.ID, "u_debugForceRoughness");
	glUniform1i(debugForceRoughnessLoc, imguiWindow.debugForceRoughness);

	int debugForceRoughnessAmountLoc = glGetUniformLocation(shader.ID, "u_debugForceRoughnessAmount");
	glUniform1f(debugForceRoughnessAmountLoc, imguiWindow.debugForceRoughnessAmount);

	int frameLoc = glGetUniformLocation(shader.ID, "u_currentSample");
	glUniform1ui(frameLoc, imguiWindow.currentSample);

	int maxBouncesLoc = glGetUniformLocation(shader.ID, "u_maxBounces");
	glUniform1ui(maxBouncesLoc, static_cast<unsigned int>(imguiWindow.maxBounces));

	int maxSamplesLoc = glGetUniformLocation(shader.ID, "u_maxSamples");
	glUniform1ui(maxSamplesLoc, static_cast<unsigned int>(imguiWindow.maxSamples));

	camera.Matrix(shader, "u_camMatrix");
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

void Scene::link() {

	depthPrepassShader = new Shader("shaders/z_prepass.vert", "shaders/z_prepass.frag");
	pathTracingShader = new Shader("shaders/path_tracing.vert", "shaders/path_tracing.frag");

	// TEXTURE ARRAY
	generateSSBOs(textureWidth, textureHeight, vertexSSBO, indicesSSBO, textureMeshSSBO, meshHeaderSSBO, textureArray, meshTexturesOutput);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);

	// COLOR NOISE
	colorNoise = new Texture(COLOR_NOISE);
	glActiveTexture(GL_TEXTURE1);
	colorNoise->Bind();

	// UNIFORMS (only those that need to be initalized once)
	int backgroundColorLoc = glGetUniformLocation(pathTracingShader->ID, "u_backgroundColor");
	glUniform3f(backgroundColorLoc, backgroundColor.x, backgroundColor.y, backgroundColor.z);

	// FRAME BUFFER
	frameBuffer = new FBO{ camera.width, camera.height, GL_TEXTURE2 };
	glBindImageTexture(2, frameBuffer->texture->ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}