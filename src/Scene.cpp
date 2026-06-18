#include "Scene.h"

Scene::Scene(Camera& i_camera, unsigned int i_textureWidth, unsigned int i_textureHeight) : camera(i_camera), textureWidth(i_textureWidth), textureHeight(i_textureHeight) {}

Scene::Scene(Camera& i_camera, string fileName, unsigned int i_textureWidth, unsigned int i_textureHeight) : camera(i_camera), textureWidth(i_textureWidth), textureHeight(i_textureHeight) {
	importScene(fileName);
}

Scene::~Scene() {

	for (Mesh* mesh : meshCollection) {
		delete mesh;
	}

}

void Scene::importScene(string fileName) {

	ifstream file("scenes/" + fileName + SCENE_FILE_EXTENSION);

	string line;
	while (getline(file, line)) {

		if (line[0] == '{') {

			getline(file, line); string s_fileName = line.substr(1, line.length() - 2);
			getline(file, line); string s_position = line.substr(1, line.length() - 2);
			getline(file, line); string s_rotation = line.substr(1, line.length() - 2);
			getline(file, line); string s_scale = line.substr(1, line.length() - 2);
			getline(file, line); string s_albedo = line.substr(1, line.length() - 2);
			getline(file, line); string s_normal = line.substr(1, line.length() - 2);
			getline(file, line); string s_roughness = line.substr(1, line.length() - 2);
			getline(file, line); string s_metallic = line.substr(1, line.length() - 2);
			getline(file, line); string s_tint = line.substr(1, line.length() - 2);
			getline(file, line); string s_emissive = line.substr(1, line.length() - 2);

			string temp[3]("");
			int commaCount;

			// POSITION PARSE
			commaCount = 0;
			temp[0] = ""; temp[1] = ""; temp[2] = "";
			for (char c : s_position) {

				if (c == ',') {
					++commaCount;
					continue;
				}

				temp[commaCount].push_back(c);
			}
			glm::vec3 position(stof(temp[0]), stof(temp[1]), stof(temp[2]));

			// ROTATION PARSE
			commaCount = 0;
			temp[0] = ""; temp[1] = ""; temp[2] = "";
			for (char c : s_rotation) {

				if (c == ',') {
					++commaCount;
					continue;
				}

				temp[commaCount].push_back(c);
			}
			glm::vec3 rotation(stof(temp[0]), stof(temp[1]), stof(temp[2]));

			// SCALE PARSE
			commaCount = 0;
			temp[0] = ""; temp[1] = ""; temp[2] = "";
			for (char c : s_scale) {

				if (c == ',') {
					++commaCount;
					continue;
				}

				temp[commaCount].push_back(c);
			}
			glm::vec3 scale(stof(temp[0]), stof(temp[1]), stof(temp[2]));

			// TINT PARSE
			commaCount = 0;
			temp[0] = ""; temp[1] = ""; temp[2] = "";
			for (char c : s_tint) {

				if (c == ',') {
					++commaCount;
					continue;
				}

				temp[commaCount].push_back(c);
			}
			glm::vec3 tint(stof(temp[0]), stof(temp[1]), stof(temp[2]));

			// EMISSION PARSE
			float emissive = stof(s_emissive);

			Mesh* mesh = new Mesh(s_fileName, tint);
			mesh->position = position;
			mesh->rotation = rotation;
			mesh->scale = scale;
			mesh->material->albedo = s_albedo;
			mesh->material->normal = s_normal;
			mesh->material->roughness = s_roughness;
			mesh->material->metallic = s_metallic;
			mesh->emissive = emissive;

			meshCollection.push_back(mesh);
		}

	}
}

void Scene::exportScene(string fileName) {

	// Back up file
	auto now = chrono::system_clock::now();
	auto local_time = chrono::current_zone()->to_local(now);
	string timeStamp = format("{:%Y-%m-%d_%H-%M-%S}", local_time);

	filesystem::path destination = string("scenes/backups/" + fileName + "_backup_" + timeStamp + SCENE_FILE_EXTENSION);
	filesystem::path source = string("scenes/" + fileName + SCENE_FILE_EXTENSION);
	filesystem::copy_file(source, destination, filesystem::copy_options::overwrite_existing);

	ofstream file("scenes/" + fileName + SCENE_FILE_EXTENSION);

	file << "// FORMAT:\n";
	file << "// \t fileName\n";
	file << "// \t position\n";
	file << "// \t rotation\n";
	file << "// \t scale\n";
	file << "// \t albedo\n";
	file << "// \t normal\n";
	file << "// \t roughness\n";
	file << "// \t metallic\n";
	file << "// \t tint\n";
	file << "// \t emissive\n\n";

	for (int i = 0; i < meshCollection.size(); ++i) {

		Mesh* mesh = meshCollection[i];

		string position = to_string(mesh->position.x) + ", " + to_string(mesh->position.y) + ", " + to_string(mesh->position.z);
		string rotation = to_string(mesh->rotation.x) + ", " + to_string(mesh->rotation.y) + ", " + to_string(mesh->rotation.z);
		string scale = to_string(mesh->scale.x) + ", " + to_string(mesh->scale.y) + ", " + to_string(mesh->scale.z);

		string albedo = mesh->material->albedo;
		string normal = mesh->material->normal;
		string roughness = mesh->material->roughness;
		string metallic = mesh->material->metallic;

		string tint = to_string(mesh->tint.r) + ", " + to_string(mesh->tint.g) + ", " + to_string(mesh->tint.b);
		string emissive = to_string(mesh->emissive);

		file << "{\n";
		file << "\t" << mesh->fileName << ",\n";
		file << "\t" << position << ",\n";
		file << "\t" << rotation << ",\n";
		file << "\t" << scale << ",\n";
		file << "\t" << albedo << ",\n";
		file << "\t" << normal << ",\n";
		file << "\t" << roughness << ",\n";
		file << "\t" << metallic << ",\n";
		file << "\t" << tint << ",\n";
		file << "\t" << emissive << "\n";
		file << "}";

		// writes last line
		if (i != meshCollection.size() - 1) {
			file << ",\n\n";
		}
	}


	file.close();

}

void Scene::Draw(GLFWwindow* window) {

	auto start = chrono::high_resolution_clock::now();

	updateVertexSSBO(vertexSSBO);

	camera.Inputs(window, imguiWindow);
	camera.updateMatrix(45.0f, 0.1f, 100.0f);

	if (!imguiWindow.pause) {

		// CLEAR BACKGROUND
		glViewport(0, 0, camera.width, camera.height);
		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Draw_DepthPrepass(*depthPrepassShader);
		Draw_PathTracingPass(*pathTracingShader);
		if (imguiWindow.debugMode == 0) Draw_PostProcessingPass(*postProcessingShader);

	}

	if (imguiWindow.currentSample != imguiWindow.maxSamples) ++imguiWindow.currentSample;

	auto end = chrono::high_resolution_clock::now();
	auto raw_duration = end - start;
	chrono::duration<double, milli> ms_double = raw_duration;

	Mesh* highlightedMesh = (imguiWindow.highlightedMesh == -1) ? nullptr : meshCollection[imguiWindow.highlightedMesh];
	imguiWindow.drawImgui(ms_double.count(), highlightedMesh);

	if (imguiWindow.importScene) {
		imguiWindow.importScene = false;
		importScene(imguiWindow.importName);
		link();
	}

	if (imguiWindow.exportScene) {
		imguiWindow.exportScene = false;
		exportScene(imguiWindow.exportName);
	}

	if (imguiWindow.deleteSelectedMesh) {
		imguiWindow.deleteSelectedMesh = false;
		erase(meshCollection, highlightedMesh);
		link();
	}

}

void Scene::Draw_DepthPrepass(Shader& Depth_shader) {

	Depth_shader.Activate();
	generateDepthUniforms(Depth_shader, camera);

	// SET-UP SHADER PROGRAM TO WRITE ONLY DEPTH
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glClear(GL_DEPTH_BUFFER_BIT);

	// DRAW MESHES
	for (size_t i = 0; i < meshCollection.size(); ++i) {
		meshCollection[i]->Draw(Depth_shader, i, meshTexturesOutput);
	}

}

void Scene::Draw_PathTracingPass(Shader& PathTracing_shader) {

	// UPDATE HIGHLIGHT BUFFER WITH IMGUI WINDOW (basically sets it to -1 incase the shader doesn't pass anything)
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, highlightedMeshBuffer);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &imguiWindow.highlightedMesh);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

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
		meshCollection[i]->Draw(PathTracing_shader, i, meshTexturesOutput);
	}

	// SEND HIGHLIGHT BUFFER INFORMATION TO IMGUI WINDOW
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, highlightedMeshBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &imguiWindow.highlightedMesh);

}

void Scene::Draw_PostProcessingPass(Shader& PostProcessing_shader) {

	PostProcessing_shader.Activate();
	generatePostProcessingUniforms(PostProcessing_shader, camera);
	
	// RE-USE SAME DEPTH
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
		meshCollection[i]->Draw(PostProcessing_shader, i, meshTexturesOutput);
	}

}

void Scene::generatePostProcessingUniforms(Shader& shader, Camera& camera) {

	int debugHighlightedMesh = glGetUniformLocation(shader.ID, "u_debugHighlightedMesh");
	glUniform1i(debugHighlightedMesh, static_cast<int>(imguiWindow.highlightedMesh));

	int debugMinBrightness = glGetUniformLocation(shader.ID, "u_debugMinBrightness");
	glUniform1f(debugMinBrightness, imguiWindow.minBrightness);

	int debugMaxBrightness = glGetUniformLocation(shader.ID, "u_debugMaxBrightness");
	glUniform1f(debugMaxBrightness, imguiWindow.maxBrightness);

	camera.Matrix(shader, "u_camMatrix");
}

void Scene::generateDepthUniforms(Shader& shader, Camera& camera) {

	int camPosUniformLocation = glGetUniformLocation(shader.ID, "u_camPos");
	glUniform3f(camPosUniformLocation, camera.Position.x, camera.Position.y, camera.Position.z);

	camera.Matrix(shader, "u_camMatrix");
}

void Scene::generatePathTracingUniforms(Shader& shader, Camera& camera) {

	GLint mousePos[2] = { imguiWindow.mouseX, camera.height - imguiWindow.mouseY };
	int debugMousePosLoc = glGetUniformLocation(shader.ID, "u_debugMousePos");
	glUniform2iv(debugMousePosLoc, 1, mousePos);

	int debugMouseLeftClick = glGetUniformLocation(shader.ID, "u_debugMouseLeftClick");
	glUniform1i(debugMouseLeftClick, imguiWindow.mouseLeftClick);

	int camPosUniformLocation = glGetUniformLocation(shader.ID, "u_camPos");
	glUniform3f(camPosUniformLocation, camera.Position.x, camera.Position.y, camera.Position.z);

	int camOrientationUniformLocation = glGetUniformLocation(shader.ID, "u_camOrientation");
	glUniform3f(camOrientationUniformLocation, camera.Orientation.x, camera.Orientation.y, camera.Orientation.z);

	int debugModeLoc = glGetUniformLocation(shader.ID, "u_debugMode");
	glUniform1ui(debugModeLoc, static_cast<unsigned int> (imguiWindow.debugMode));

	int debugLambertianLoc = glGetUniformLocation(shader.ID, "u_debugLambertian");
	glUniform1i(debugLambertianLoc, imguiWindow.debugLambertian);

	int debugUniversalRoughnessLoc = glGetUniformLocation(shader.ID, "u_debugUniversalRoughness");
	glUniform1i(debugUniversalRoughnessLoc, imguiWindow.debugUniversalRoughness);

	int debugUniversalRoughnessAmountLoc = glGetUniformLocation(shader.ID, "u_debugUniversalRoughnessAmount");
	glUniform1f(debugUniversalRoughnessAmountLoc, imguiWindow.debugUniversalRoughnessAmount);

	int frameLoc = glGetUniformLocation(shader.ID, "u_currentSample");
	glUniform1ui(frameLoc, imguiWindow.currentSample);

	int maxBouncesLoc = glGetUniformLocation(shader.ID, "u_maxBounces");
	glUniform1ui(maxBouncesLoc, static_cast<unsigned int>(imguiWindow.maxBounces));

	int seedColorLoc = glGetUniformLocation(shader.ID, "u_seed");
	glUniform1ui(seedColorLoc, m_distrib(m_gen));

	camera.Matrix(shader, "u_camMatrix");
}

void Scene::updateVertexSSBO(GLuint vertexSSBO) {

	vector<Vertex> globalVertices;

	for (Mesh* mesh : meshCollection) {

		for (auto vertex : mesh->vertices) {

			vertex.position = mesh->getModelMatrix() * vertex.position;
			globalVertices.push_back(vertex);

		}
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
	glBufferSubData(
		GL_SHADER_STORAGE_BUFFER,
		0,
		globalVertices.size() * sizeof(globalVertices[0]),
		globalVertices.data()
	);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


}

void Scene::generateSSBOs(unsigned int width, unsigned int height, GLuint& vertexSSBO, GLuint& indicesSSBO, GLuint& meshTextureSSBO, GLuint& meshHeaderSSBO, GLuint& textureArray, vector<glm::vec4>& meshTexturesOutput) {

	vector<Vertex> globalVertices;
	vector<GLuint> globalIndices;
	vector<glm::vec4> meshTextures; // <albedoIndex, normalIndex, roughnessIndex, metallicIndex>
	vector<glm::vec4> meshHeader;	// <indicesStartPointer, indicesSize, emissiveValue>

	set<string> texturePool;

	size_t indexPointer = 0;
	for (Mesh* mesh : meshCollection) {

		// VERTICES
		for (auto vertex : mesh->vertices) {

			vertex.position = mesh->getModelMatrix() * vertex.position;
			globalVertices.push_back(vertex);

		}

		// INDICES
		vector<GLuint> tempIndices = mesh->indices;
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
		texturePointers.x = distance(texturePool.begin(), texturePool.find(mesh->material->albedo));
		texturePointers.y = distance(texturePool.begin(), texturePool.find(mesh->material->normal));
		texturePointers.z = distance(texturePool.begin(), texturePool.find(mesh->material->roughness));
		texturePointers.w = distance(texturePool.begin(), texturePool.find(mesh->material->metallic));

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

	// 2D-TEXTURE ARRAY
	glGenTextures(1, &textureArray);

	GLuint maxTextureCount = texturePool.size();

	vector<string> texturePoolVector(texturePool.begin(), texturePool.end());
	Texture::loadTextureArray(textureArray, texturePoolVector, width, height);

	// output this (used for drawing)
	meshTexturesOutput = meshTextures;

}

void Scene::link() {

	depthPrepassShader = new Shader("shaders/z_prepass.vert", "shaders/z_prepass.frag");
	pathTracingShader = new Shader("shaders/path_tracing.vert", "shaders/path_tracing.frag");
	postProcessingShader = new Shader("shaders/post_processing.vert", "shaders/post_processing.frag");

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

	// ACCUMULATION BUFFER
	accumulationBuffer = new FBO{ camera.width, camera.height, GL_TEXTURE3 };
	glBindImageTexture(3, accumulationBuffer->texture->ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// HIGHLIGHTED MESH BUFFER
	glGenBuffers(1, &highlightedMeshBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, highlightedMeshBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint), &imguiWindow.highlightedMesh, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, highlightedMeshBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}