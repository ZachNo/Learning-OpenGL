//Standard Headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

//Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GL/glew.h>

//Include GLFW
#include <GLFW/glfw3.h>

//Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

//Include Bullet Physics
#include <btBulletDynamicsCommon.h>

//Include Visual Studio Memory Leak Detection
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//Defined for texturemanager
#define STB_IMAGE_IMPLEMENTATION

//Include shader
#include "shader.h"

//Include error reporting system
#include "error.h"

//Include controls
#include "controls.h"

//Include entitys
#include "entity.h"

//Include PhysicsManager
#include "physics.h"

//Include modelManager for model data
#include "modelManager.h"

int main()
{
	_CrtMemState s1;
	_CrtMemCheckpoint(&s1);
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	GLFWwindow* window;
	window = glfwCreateWindow(1024, 768, "MEOW", NULL, NULL);
	if (window == NULL){
		reportError("Failed to open GLFW window.\n",1);
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		reportError("Failed to initialize GLEW\n",1);
		return -1;
	}

	glfwSetWindowPos(window, 400, 200);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	//glfwSwapInterval(0);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("vertex.glsl", "fragment.glsl");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	
	//Bullet physics stuffs
	PhysicsManager* physMan = new PhysicsManager;
	btDynamicsWorld* dynamicsWorld = physMan->getDW();

	//Entity Manager
	EntityManager *entities = new EntityManager(TextureID, MatrixID, ViewMatrixID, ModelMatrixID, dynamicsWorld);

	//btCollisionShape* groundShape = new btBoxShape(btVector3(30, 0.1, 30));
	btCollisionShape* sphereShape = new btSphereShape(1.0f);

	entities->createEntity("sphere.obj", "checker.png", glm::vec3(0, 0, 0), glm::quat(0, 0, 0, 1), sphereShape, btScalar(1), &btVector3(0, 0, 0));
	entities->createEntity("ball_testCourse.obj", "test_texture.png", glm::vec3(0, -5, 0), glm::quat(1, 0, 0, 0), NULL);

	std::cout << "initialized col mesh successfully\n";

	entities->getEntity(0)->setRestitution(0.8f);
	entities->getEntity(0)->getRigidBody()->setRollingFriction(0.3f);
	entities->getEntity(0)->setFriction(1);
	entities->getEntity(0)->getRigidBody()->setActivationState(DISABLE_DEACTIVATION);
	entities->getEntity(1)->setRestitution(0.5f);
	entities->getEntity(1)->setFriction(1);
	entities->getEntity(1)->getRigidBody()->setRollingFriction(1);

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	// Initial horizontal angle : toward -Z
	float horizontalAngle = 3.14f;
	// Initial vertical angle : none
	float verticalAngle = 0.0f;
	// Initial Field of View
	float fov = 90.0f;
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;

	bool mouseLock = 1;
	bool L_keyDown = 0;

	try{
		do{
			try{
				// Measure speed
				double currentTime = glfwGetTime();
				nbFrames++;
				if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
					// printf and reset
					printf("%f ms/frame %f FPS\n", 1000.0 / double(nbFrames), double(nbFrames));
					nbFrames = 0;
					lastTime += 1.0;
				}

				// Clear the screen
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// Use our shader
				glUseProgram(programID);

				if (glfwGetKey(window, GLFW_KEY_L) && !L_keyDown)
				{
					if (mouseLock)
						mouseLock = 0;
					else
						mouseLock = 1;
					L_keyDown = 1;
				}
				else if (!glfwGetKey(window, GLFW_KEY_L))
					L_keyDown = 0;

				// Compute the MVP matrix from keyboard and mouse input
				computeMatricesFromInputs(window, dynamicsWorld, &horizontalAngle, &verticalAngle, fov, entities->getEntity(0), &ViewMatrix, &ProjectionMatrix, mouseLock);

				glm::vec3 lightPos = glm::vec3(4, 4, 4);
				glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

				dynamicsWorld->stepSimulation(1 / 144.0f, 10);

				entities->updateAll();
				entities->drawAll(&ProjectionMatrix, &ViewMatrix);


				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);
				glDisableVertexAttribArray(2);

				// Swap buffers
				glfwSwapBuffers(window);
				glfwPollEvents();
			}
			catch (const char *str)
			{
				throw str;
			}
			catch (...)
			{
				throw "Unknown error occured!\n";
			}
		} // Check if the ESC key was pressed or the window was closed
		while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
	}
	catch (const char *str)
	{
		std::cout << str << std::endl;
	}


	// Cleanup VBO and shader
	glDeleteProgram(programID);
	glDeleteTextures(1, &TextureID);
	glDeleteVertexArrays(1, &VertexArrayID);

	delete entities;
	delete physMan;

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	_CrtMemState s2;
	_CrtMemCheckpoint(&s2);
	_CrtMemState s3;
	if (_CrtMemDifference(&s3, &s1, &s2))
		_CrtMemDumpStatistics(&s3);
	else
		std::cout << "No difference1\n";

	//system("pause");

	return 0;
}