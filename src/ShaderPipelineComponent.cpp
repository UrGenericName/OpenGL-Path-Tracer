#pragma once

#include "ShaderPipelineComponent.h"

#include "Mesh.h"

ShaderPipelineComponent::~ShaderPipelineComponent() {

	depthPrepassShader->Delete();
	pathTracingShader->Delete();
	postProcessingShader->Delete();

	frameBuffer->Delete();
	accumulationBuffer->Delete();

	colorNoise->Delete();

}

void ShaderPipelineComponent::link(DebugSettings& debugSettings, Camera& camera, glm::vec3& backgroundColor) {

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
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint), &debugSettings.highlightedMeshIndex, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, highlightedMeshBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// COLOR NOISE
	colorNoise = new Texture(COLOR_NOISE);
	glActiveTexture(GL_TEXTURE1);
	colorNoise->Bind();

}

void ShaderPipelineComponent::generateDepthUniforms(Shader& shader, Camera& camera, DebugSettings& debugSettings) {

	int camPosUniformLocation = glGetUniformLocation(shader.ID, "u_camPos");
	glUniform3f(camPosUniformLocation, camera.Position.x, camera.Position.y, camera.Position.z);

	camera.Matrix(shader, "u_camMatrix");

}

void ShaderPipelineComponent::generatePathTracingUniforms(Shader& shader, Camera& camera, DebugSettings& debugSettings) {

	GLint mousePos[2] = { debugSettings.mouseX, camera.height - debugSettings.mouseY };
	int debugMousePosLoc = glGetUniformLocation(shader.ID, "u_debugMousePos");
	glUniform2iv(debugMousePosLoc, 1, mousePos);

	int debugMouseLeftClick = glGetUniformLocation(shader.ID, "u_debugMouseLeftClick");
	glUniform1i(debugMouseLeftClick, debugSettings.mouseLeftClick);

	int camPosUniformLocation = glGetUniformLocation(shader.ID, "u_camPos");
	glUniform3f(camPosUniformLocation, camera.Position.x, camera.Position.y, camera.Position.z);

	int camOrientationUniformLocation = glGetUniformLocation(shader.ID, "u_camOrientation");
	glUniform3f(camOrientationUniformLocation, camera.Orientation.x, camera.Orientation.y, camera.Orientation.z);

	int debugModeLoc = glGetUniformLocation(shader.ID, "u_debugMode");
	glUniform1ui(debugModeLoc, static_cast<unsigned int> (debugSettings.debugMode));

	int debugLambertianLoc = glGetUniformLocation(shader.ID, "u_debugLambertian");
	glUniform1i(debugLambertianLoc, debugSettings.debugLambertian);

	int debugUniversalRoughnessLoc = glGetUniformLocation(shader.ID, "u_debugUniversalRoughness");
	glUniform1i(debugUniversalRoughnessLoc, debugSettings.debugUniversalRoughness);

	int debugUniversalRoughnessAmountLoc = glGetUniformLocation(shader.ID, "u_debugUniversalRoughnessAmount");
	glUniform1f(debugUniversalRoughnessAmountLoc, debugSettings.debugUniversalRoughnessAmount);

	int maxSamplesLoc = glGetUniformLocation(shader.ID, "u_maxSamples");
	glUniform1ui(maxSamplesLoc, debugSettings.maxSamples);

	int currentSampleLoc = glGetUniformLocation(shader.ID, "u_currentSample");
	glUniform1ui(currentSampleLoc, debugSettings.currentSample);

	int maxBouncesLoc = glGetUniformLocation(shader.ID, "u_maxBounces");
	glUniform1ui(maxBouncesLoc, static_cast<unsigned int>(debugSettings.maxBounces));

	int seedColorLoc = glGetUniformLocation(shader.ID, "u_seed");
	glUniform1ui(seedColorLoc, m_distrib(m_gen));

	camera.Matrix(shader, "u_camMatrix");
}

void ShaderPipelineComponent::generatePostProcessingUniforms(Shader& shader, Camera& camera, GLFWwindow* window, DebugSettings& debugSettings) {

	int gizmoSelectionLoc = glGetUniformLocation(shader.ID, "u_gizmoSelection");
	glUniform1i(gizmoSelectionLoc, static_cast<int>(gizmo.getSelection(window, camera, debugSettings.highlightedMeshIndex != -1)));

	int debugHighlightedMeshLoc = glGetUniformLocation(shader.ID, "u_debugHighlightedMesh");
	glUniform1i(debugHighlightedMeshLoc, static_cast<int>(debugSettings.highlightedMeshIndex));

	int debugMinBrightnessLoc = glGetUniformLocation(shader.ID, "u_debugMinBrightness");
	glUniform1f(debugMinBrightnessLoc, debugSettings.minBrightness);

	int debugMaxBrightnessLoc = glGetUniformLocation(shader.ID, "u_debugMaxBrightness");
	glUniform1f(debugMaxBrightnessLoc, debugSettings.maxBrightness);

	camera.Matrix(shader, "u_camMatrix");
}

void ShaderPipelineComponent::Draw_DepthPrepass(Camera& camera, vector<Mesh*>& meshCollection, vector<MeshTextures>& meshTextures, DebugSettings& debugSettings) {

	depthPrepassShader->Activate();
	generateDepthUniforms(*depthPrepassShader, camera, debugSettings);

	// SET-UP SHADER PROGRAM TO WRITE ONLY DEPTH
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glClear(GL_DEPTH_BUFFER_BIT);

	// DRAW MESHES
	for (size_t i = 0; i < meshCollection.size(); ++i) {
		meshCollection[i]->Draw(*depthPrepassShader, i, meshTextures);
	}

}

void ShaderPipelineComponent::Draw_PathTracingPass(Camera& camera, vector<Mesh*>& meshCollection, vector<MeshTextures>& meshTextures, DebugSettings& debugSettings) {

	// UPDATE HIGHLIGHT BUFFER WITH IMGUI WINDOW (basically sets it to -1 incase the shader doesn't pass anything)
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, highlightedMeshBuffer);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &debugSettings.highlightedMeshIndex);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	pathTracingShader->Activate();
	generatePathTracingUniforms(*pathTracingShader, camera, debugSettings);

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
		meshCollection[i]->Draw(*pathTracingShader, i, meshTextures);
	}

	// SEND HIGHLIGHT BUFFER INFORMATION TO IMGUI WINDOW
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, highlightedMeshBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &debugSettings.highlightedMeshIndex);
}

void ShaderPipelineComponent::Draw_PostProcessingPass(Camera& camera, vector<Mesh*>& meshCollection, vector<MeshTextures>& meshTextures, GLFWwindow* window, DebugSettings& debugSettings) {

	if (debugSettings.highlightedMeshIndex != -1) {

		Mesh* highlightedMesh = meshCollection[debugSettings.highlightedMeshIndex];
		gizmo.setPos(highlightedMesh->position);

	}

	postProcessingShader->Activate();
	generatePostProcessingUniforms(*postProcessingShader, camera, window, debugSettings);

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
		meshCollection[i]->Draw(*postProcessingShader, i, meshTextures);
	}

	if (debugSettings.highlightedMeshIndex != -1) {

		glDisable(GL_DEPTH_TEST);
		gizmo.Draw(*postProcessingShader);

	}
}