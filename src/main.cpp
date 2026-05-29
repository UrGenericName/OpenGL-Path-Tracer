#include "Scene.h"

#define WIDTH 800
#define HEIGHT 800

int main() {

	glfwInit();

	// Pass info to glfw about the version and profile of OpenGL to use
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, 0);

	// Creates the actual glfw window
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Graphcis", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Sets our created window to active
	glfwMakeContextCurrent(window);

	// loads OpenGL
	gladLoadGL();


	
	Shader shaderProgram("shaders/path_tracing.vert", "shaders/path_tracing.frag");

	/*
	Material carBaseMat("textures/car_base_albedo.png", "textures/car_base_normal.png");
	Mesh carBase("models/car_base.obj", carBaseMat);

	Material carPartsMat("textures/car_parts_albedo.png", "textures/car_parts_normal.png");
	Mesh carParts("models/car_parts.obj", carPartsMat);
	*/

	Scene scene;
	scene.meshCollection.push_back(new Mesh("models/room/cubeA.obj"));
	scene.meshCollection.push_back(new Mesh("models/room/cubeB.obj"));
	scene.meshCollection.push_back(new Mesh("models/room/sphere.obj"));
	scene.meshCollection.push_back(new Mesh("models/room/white_walls.obj"));
	scene.meshCollection.push_back(new Mesh("models/room/light.obj", glm::vec3(1.0f, 0.8f, 0.8f), true));
	scene.meshCollection.push_back(new Mesh("models/room/red_wall.obj", glm::vec3(1.0f, 0.0f, 0.0f)));
	scene.meshCollection.push_back(new Mesh("models/room/green_wall.obj", glm::vec3(0.0f, 1.0f, 0.0f)));

	glm::vec3 posA = scene.meshCollection[2]->vertices[0].position;
	glm::vec3 posB = scene.meshCollection[2]->vertices[1].position;
	glm::vec3 posC = scene.meshCollection[2]->vertices[2].position;

	printf("%f, %f, %f\n", posA.x, posA.y, posA.z);
	printf("%f, %f, %f\n", posB.x, posB.y, posB.z);
	printf("%f, %f, %f\n", posC.x, posC.y, posC.z);

	shaderProgram.Activate();
	glm::vec3 sun = { 1.0f, 1.0f, -1.0f };
	int sunUniformLocation = glGetUniformLocation(shaderProgram.ID, "sun");
	glUniform3f(sunUniformLocation, sun.x, sun.y, sun.z);

	glViewport(0, 0, WIDTH, HEIGHT);
	glfwSwapBuffers(window);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	Camera camera(WIDTH, HEIGHT, glm::vec3(0.0f, 0.0f, 0.0f));

	int camPosUniformLocation = glGetUniformLocation(shaderProgram.ID, "camPos");

	while (!glfwWindowShouldClose(window)) 
	{

		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);	// sets the "clear" color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// actually clears the background with color

		camera.updateMatrix(45.0f, 0.1f, 100.0f);
		camera.Inputs(window);

		scene.Draw(shaderProgram, camera);
		glUniform3f(camPosUniformLocation, camera.Position.x, camera.Position.y, camera.Position.z);

		glfwSwapBuffers(window);

		// Responds to actions like window resizing, minimizing, etc
		glfwPollEvents();

	}

	shaderProgram.Delete();

	// Destroys the window
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}