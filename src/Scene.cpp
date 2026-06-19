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

	ifstream file("scenes/" + fileName);

	meshCollection.resize(0);

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

	file.close();

	link();
}

void Scene::exportScene(string fileName) {

	// Back up file
	auto now = chrono::system_clock::now();
	auto local_time = chrono::current_zone()->to_local(now);
	string timeStamp = format("{:%Y-%m-%d_%H-%M-%S}", local_time);

	filesystem::path destination = string("scenes/backups/" + timeStamp + "_backup_" + fileName);
	filesystem::path source = string("scenes/" + fileName);
	filesystem::copy_file(source, destination, filesystem::copy_options::overwrite_existing);

	ofstream file("scenes/" + fileName);

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

	camera.Inputs(window, imguiWindow);
	camera.updateMatrix();

	if (!imguiWindow.pause) {

		// CLEAR BACKGROUND
		glViewport(0, 0, camera.width, camera.height);
		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Draw_DepthPrepass(*depthPrepassShader);
		Draw_PathTracingPass(*pathTracingShader);
		Draw_PostProcessingPass(*postProcessingShader);

	}

	if (imguiWindow.currentSample != imguiWindow.maxSamples) ++imguiWindow.currentSample;

	auto end = chrono::high_resolution_clock::now();
	auto raw_duration = end - start;
	chrono::duration<double, milli> ms_double = raw_duration;

	Mesh* highlightedMesh = (imguiWindow.highlightedMesh == -1) ? nullptr : meshCollection[imguiWindow.highlightedMesh];
	imguiWindow.drawImgui(ms_double.count(), this, highlightedMesh);

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
		meshCollection[i]->Draw(Depth_shader, i, meshTextures);
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
		meshCollection[i]->Draw(PathTracing_shader, i, meshTextures);
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
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// FRAME BUFFER
	frameBuffer->Bind();
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, frameBuffer->texture->ID);
	frameBuffer->Unbind();

	// DRAW MESHES
	for (size_t i = 0; i < meshCollection.size(); ++i) {
		meshCollection[i]->Draw(PostProcessing_shader, i, meshTextures);
	}

	if (imguiWindow.highlightedMesh != -1) {

		Mesh* highlightedMesh = meshCollection[imguiWindow.highlightedMesh];

		gizmoX->position = highlightedMesh->position;
		gizmoY->position = highlightedMesh->position;
		gizmoZ->position = highlightedMesh->position;

		glDisable(GL_DEPTH_TEST);
		gizmoX->DrawGizmo(PostProcessing_shader);
		gizmoY->DrawGizmo(PostProcessing_shader);
		gizmoZ->DrawGizmo(PostProcessing_shader);
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

void Scene::updateVertexSSBO() {

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
	glBufferSubData(
		GL_SHADER_STORAGE_BUFFER,
		0,
		globalVertices.size() * sizeof(globalVertices[0]),
		globalVertices.data()
	);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void Scene::updateIndicesSSBO() {

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, indicesSSBO);
	glBufferSubData(
		GL_SHADER_STORAGE_BUFFER,
		0,
		globalIndices.size() * sizeof(globalIndices[0]),
		globalIndices.data()
	);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void Scene::updateMeshTexturesSSBO() {

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshTextureSSBO);
	glBufferSubData(
		GL_SHADER_STORAGE_BUFFER,
		0,
		meshTextures.size() * sizeof(meshTextures[0]),
		meshTextures.data()
	);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void Scene::updateMeshHeaderSSBO() {

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshHeaderSSBO);
	glBufferSubData(
		GL_SHADER_STORAGE_BUFFER,
		0,
		meshHeader.size() * sizeof(meshHeader[0]),
		meshHeader.data()
	);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void Scene::generateGlobalVertices() {

	globalVertices.resize(0);

	for (Mesh* mesh : meshCollection) {

		// VERTICES
		for (auto vertex : mesh->vertices) {

			vertex.position = mesh->getModelMatrix() * vertex.position;
			vertex.color = glm::vec4(mesh->tint, 1.0f);
			globalVertices.push_back(vertex);

		}

	}

}

void Scene::generateGlobalIndices() {

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

void Scene::generateMeshTextures() {

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

void Scene::generateMeshHeader() {

	meshHeader.resize(0);

	size_t indexPointer = 0;
	for (Mesh* mesh : meshCollection) {

		// MESH-HEADER
		meshHeader.push_back(glm::vec4(indexPointer, mesh->indices.size(), mesh->emissive, 0.0f));

		// Loop increment (used to convert local indices to global indices)
		indexPointer += mesh->vertices.size();

	}

}

void Scene::generateSSBOs(unsigned int width, unsigned int height) {

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

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);


}

void Scene::link() {

	depthPrepassShader = new Shader("shaders/z_prepass.vert", "shaders/z_prepass.frag");
	pathTracingShader = new Shader("shaders/path_tracing.vert", "shaders/path_tracing.frag");
	postProcessingShader = new Shader("shaders/post_processing.vert", "shaders/post_processing.frag");

	GLuint buffersToDelete[] = { vertexSSBO, indicesSSBO, meshTextureSSBO, meshHeaderSSBO, textureArray };
	glDeleteBuffers(5, buffersToDelete);

	generateGlobalVertices();
	generateGlobalIndices();
	generateMeshTextures();
	generateMeshHeader();
	generateSSBOs(textureWidth, textureHeight);

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

void ImguiWindow::drawImgui(double frameTime, Scene* scene, Mesh* highlightedMesh) {

	using namespace ImGui;

	// DEBUG WINDOW
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	NewFrame();

	if (IsKeyPressed(ImGuiKey_F)) {
		drawWindow = !drawWindow;
		currentSample = 0;
	}

	if (!drawWindow) { EndFrame();  return; }

	Begin("Debug");
	Separator();

	Text("Samples: %d/%d\t\tFT(ms): %.3f\t\tFPS: %d", currentSample, maxSamples, frameTime, static_cast<int>(1000 / frameTime));

	// RENDER VISUALIZATION
	if (BeginTable("ShaderLayoutTable", 2)) {

		TableSetupColumn("Render Visualization");
		TableSetupColumn("Settings");
		TableHeadersRow();

		TableNextRow();

		// RENDER VISUALIZATION
		TableNextColumn();
		if (RadioButton("Disabled", &debugMode, static_cast<int>(DebugTypes::DISABLED))) currentSample = 0;
		if (RadioButton("Albedo", &debugMode, static_cast<int>(DebugTypes::ALBEDO))) currentSample = 0;
		if (RadioButton("Normal", &debugMode, static_cast<int>(DebugTypes::NORMAL))) currentSample = 0;
		if (RadioButton("Roughness", &debugMode, static_cast<int>(DebugTypes::ROUGHNESS))) currentSample = 0;
		if (RadioButton("Metallic", &debugMode, static_cast<int>(DebugTypes::METALLIC))) currentSample = 0;

		// SETTINGS
		TableNextColumn();
		Checkbox("Lambertian Shading", &debugLambertian);
		SliderFloat("Min Brightness", &minBrightness, 0.0f, maxBrightness - 0.001f);
		SliderFloat("Max Brightness", &maxBrightness, minBrightness + 0.001f, 1.0f);
		if (SliderInt("Bounces", &maxBounces, 1, MAX_BOUNCES)) currentSample = 0;
		if (SliderInt("Samples", &maxSamples, 1, MAX_SAMPLES)) currentSample = 0;

		EndTable();
	}

	// SCENE SETTINGS
	if (BeginTable("ShaderLayoutTable", 3)) {

		TableSetupColumn("Scene");
		TableHeadersRow();

		TableNextRow();

		const ImVec4 validPathColor(0.1f, 0.3f, 0.1f, 1.0f);
		const ImVec4 invalidPathColor(0.15f, 0.15f, 0.15f, 1.0f);

		std::filesystem::path filePath;

		TableNextColumn();

		filePath = "scenes/" + string(importName);
		bool isImportValid = (std::filesystem::exists(filePath) && filePath.extension().string() == SCENE_FILE_EXTENSION);
		PushStyleColor(ImGuiCol_FrameBg, (isImportValid ? validPathColor : invalidPathColor));
		InputTextWithHint("##importFileName", "scene.txt", importName, IM_ARRAYSIZE(importName));
		PopStyleColor(1);
		TableNextColumn();

		filePath = "scenes/" + string(exportName);
		bool isExportValid = (filePath.extension().string() == SCENE_FILE_EXTENSION);
		PushStyleColor(ImGuiCol_FrameBg, (isExportValid ? validPathColor : invalidPathColor));
		InputTextWithHint("##exportFileName", "scene.txt", exportName, IM_ARRAYSIZE(exportName));
		PopStyleColor(1);
		TableNextColumn();

		filePath = importOBJname;
		bool isOBJValid = (std::filesystem::exists(filePath) && filePath.extension().string() == ".obj");
		PushStyleColor(ImGuiCol_FrameBg, (isOBJValid ? validPathColor : invalidPathColor));
		InputTextWithHint("##importOBJName", "models/cube.obj", importOBJname, IM_ARRAYSIZE(importOBJname));
		PopStyleColor(1);

		TableNextRow();
		TableNextColumn();
		BeginDisabled(!isImportValid);
		if (Button("Import Scene")) {
			scene->importScene(importName);
			scene->link();
			strcpy(exportName, importName);
			importName[0] = 0x00;
		}
		EndDisabled();

		TableNextColumn();
		BeginDisabled(!isExportValid);
		if (Button("Export Scene")) {
			scene->exportScene(exportName);
			exportName[0] = 0x00;
		}
		EndDisabled();

		TableNextColumn();
		BeginDisabled(!isOBJValid);
		if (Button("Import OBJ")) {

			Mesh* mesh = new Mesh(importOBJname);
			scene->meshCollection.push_back(mesh);
			scene->link();
			importOBJname[0] = 0x00;
		}
		EndDisabled();

		EndTable();
	}

	// CAMERA SETTINGS
	if (BeginTable("ShaderLayoutTable", 5)) {

		TableSetupColumn("Camera");
		TableSetupColumn("   X");
		TableSetupColumn("   Y");
		TableSetupColumn("   Z");
		TableHeadersRow();

		TableNextRow();
		TableNextColumn();
		Text("Position");
		TableNextColumn();
		{
			PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
			if (DragFloat("##posX", &(scene->camera.Position.x), 0.5f)) currentSample = 0;
			PopStyleColor(1);
			TableNextColumn();
			PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
			if (DragFloat("##posY", &(scene->camera.Position.y), 0.5f)) currentSample = 0;
			PopStyleColor(1);
			TableNextColumn();
			PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
			if (DragFloat("##posZ", &(scene->camera.Position.z), 0.5f)) currentSample = 0;
			PopStyleColor(1);
			TableNextColumn();
			if (Button("Reset##positon")) scene->camera.Position = glm::vec3(0.0f);
		}

		TableNextRow();
		TableNextColumn();
		if (SliderFloat("FOV", &(scene->camera.FOVdeg), 0.0f, 90.0f)) currentSample = 0;
		TableNextColumn();
		if (SliderFloat("Near Plane", &(scene->camera.nearPlane), 0.0f, scene->camera.farPlane)) currentSample = 0;
		TableNextColumn();
		if (SliderFloat("Far Plane", &(scene->camera.farPlane), scene->camera.nearPlane, 10000.0f)) currentSample = 0;


		EndTable();
	}

	// MESH SETTINGS
	if (highlightedMesh != nullptr) {
		
		if (BeginTable("ShaderLayoutTable", 5)) {

			TableSetupColumn("Edit Mesh");
			TableSetupColumn("   X");
			TableSetupColumn("   Y");
			TableSetupColumn("   Z");
			TableHeadersRow();

			TableNextRow();
			TableNextColumn();
			Text("Position");
			TableNextColumn();
			{
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
				if (DragFloat("##posX", &(highlightedMesh->position.x), 0.5f)) {
					currentSample = 0;
					scene->generateGlobalVertices();
					scene->updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
				if (DragFloat("##posY", &(highlightedMesh->position.y), 0.5f)) {
					currentSample = 0;
					scene->generateGlobalVertices();
					scene->updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
				if (DragFloat("##posZ", &(highlightedMesh->position.z), 0.5f)) {
					currentSample = 0;
					scene->generateGlobalVertices();
					scene->updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				if (Button("Reset##positon")) {
					currentSample = 0;
					highlightedMesh->position = glm::vec3(0.0f);
					scene->generateGlobalVertices();
					scene->updateVertexSSBO();
				}
			}

			TableNextRow();
			TableNextColumn();
			Text("Rotation");
			TableNextColumn();
			{
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
				if (DragFloat("##pitch", &(highlightedMesh->rotation.x), 0.1f)) {
					currentSample = 0;
					scene->generateGlobalVertices();
					scene->updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
				if (DragFloat("##yaw", &(highlightedMesh->rotation.y), 0.1f)) {
					currentSample = 0;
					scene->generateGlobalVertices();
					scene->updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
				if (DragFloat("##roll", &(highlightedMesh->rotation.z), 0.1f)) {
					currentSample = 0;
					scene->generateGlobalVertices();
					scene->updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				if (Button("Reset##rotation")) {
					highlightedMesh->rotation = glm::vec3(0.0f); 
					currentSample = 0;
					scene->generateGlobalVertices();
					scene->updateVertexSSBO();
				}
			}

			TableNextRow();
			TableNextColumn();
			Text("Scale");
			TableNextColumn();
			{
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
				if (DragFloat("##scaleX", &(highlightedMesh->scale.x), 0.1f)) {
					currentSample = 0;
					scene->generateGlobalVertices();
					scene->updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
				if (DragFloat("##scaleY", &(highlightedMesh->scale.y), 0.1f)) {
					currentSample = 0;
					scene->generateGlobalVertices();
					scene->updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
				if (DragFloat("##scaleZ", &(highlightedMesh->scale.z), 0.1f)) {
					currentSample = 0;
					scene->generateGlobalVertices();
					scene->updateVertexSSBO();
				}
				PopStyleColor(1);
				TableNextColumn();
				if (Button("Reset##scale")) {
					highlightedMesh->scale = glm::vec3(1.0f); 
					currentSample = 0;
					scene->generateGlobalVertices();
					scene->updateVertexSSBO();
				}
			}

			TableNextRow();
			TableNextColumn();
			Text("Tint");
			TableNextColumn();
			{
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
				if (SliderFloat("##tintR", &(highlightedMesh->tint.x), 0.0f, 1.0f)) {
					currentSample = 0;
					scene->generateGlobalVertices();
					scene->updateVertexSSBO();
					for (Mesh* mesh : scene->meshCollection) {
						mesh->updateBuffers();
					}
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.3f, 0.1f, 1.0f));
				if (SliderFloat("##tintG", &(highlightedMesh->tint.y), 0.0f, 1.0f)) {
					currentSample = 0;
					scene->generateGlobalVertices();
					scene->updateVertexSSBO();
					for (Mesh* mesh : scene->meshCollection) {
						mesh->updateBuffers();
					}
				}
				PopStyleColor(1);
				TableNextColumn();
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.3f, 1.0f));
				if (SliderFloat("##tintB", &(highlightedMesh->tint.z), 0.0f, 1.0f)) {
					currentSample = 0; 
					scene->generateGlobalVertices();
					scene->updateVertexSSBO();
					for (Mesh* mesh : scene->meshCollection) {
						mesh->updateBuffers();
					}
				}
				PopStyleColor(1);
				TableNextColumn();
				if (Button("Reset##tint")) {
					currentSample = 0;
					highlightedMesh->tint = glm::vec3(1.0f);
				}
			}

			TableNextRow();
			TableNextColumn();
			Text("Emission");
			TableNextColumn();
			if (SliderFloat("##emission", &(highlightedMesh->emissive), 0.0f, 500.0f)) {
				currentSample = 0;
				scene->generateMeshHeader();
				scene->updateMeshHeaderSSBO();
			}

			TableNextRow();
			TableNextColumn();
			if (Button("Delete")) {
				erase(scene->meshCollection, highlightedMesh);
				scene->link();
			}

			EndTable();

		}

	}

	if (BeginTable("ShaderLayoutTable", 2)) {

		TableSetupColumn("Misc");
		TableSetupColumn("");
		TableHeadersRow();

		TableNextRow();

		TableNextColumn();
		BeginDisabled(!debugUniversalRoughness);
		if (SliderFloat("Roughness", &debugUniversalRoughnessAmount, 0, 1)) currentSample = 0;
		EndDisabled();

		TableNextColumn();
		if (Button("Clear Samples")) currentSample = 0;

		TableNextRow();

		TableNextColumn();
		if (Checkbox("Universal Roughness", &debugUniversalRoughness)) currentSample = 0;
		TableNextColumn();
		if (Button("Pause")) pause = !pause;

		EndTable();
	}

	// clear samples if window is moved or resized
	if (windowSize.x != GetWindowSize().x || windowSize.y != GetWindowSize().y) currentSample = 0;
	if (windowPosition.x != GetWindowPos().x || windowPosition.y != GetWindowPos().y) currentSample = 0;

	windowSize = GetWindowSize();
	windowPosition = GetWindowPos();



	End();

	Render();
	ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());

}