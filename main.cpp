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

//#include <vld.h>

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
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}
	GLFWwindow* window;
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context

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

	glfwSetWindowPos(window, 600, 200);

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
	GLuint depthProgramID = LoadShaders("depth_vert.glsl", "depth_frag.glsl");

	// Get a handle for our "MVP" uniform
	GLuint depthMatrixID = glGetUniformLocation(depthProgramID, "depthMVP");

	// ---------------------------------------------
	// Render to Texture - specific code begins here
	// ---------------------------------------------

	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	GLuint FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// Depth texture. Slower than a depth buffer, but you can sample it later in your shader
	GLuint depthTexture;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

	// No color output in the bound framebuffer, only depth.
	glDrawBuffer(GL_NONE);

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;


	// The quad's FBO. Used only for visualizing the shadowmap.
	static const GLfloat g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
	};

	GLuint quad_vertexbuffer;
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

	// Create and compile our GLSL program from the shaders
	GLuint quad_programID = LoadShaders("debug_vert.glsl", "debug_frag.glsl");
	GLuint texID = glGetUniformLocation(quad_programID, "texture");



	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("vertex.glsl", "fragment.glsl");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
	GLuint DepthBiasID = glGetUniformLocation(programID, "DepthBiasMVP");
	GLuint ShadowMapID = glGetUniformLocation(programID, "shadowMap");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Get a handle for our "LightPosition" uniform
	GLuint lightInvDirID = glGetUniformLocation(programID, "LightInvDirection_worldspace");
	
	//Bullet physics stuffs
	PhysicsManager* physMan = new PhysicsManager;
	btDynamicsWorld* dynamicsWorld = physMan->getDW();

	//Entity Manager
	EntityManager *entities = new EntityManager(TextureID, MatrixID, ViewMatrixID, ModelMatrixID, dynamicsWorld);

	//btCollisionShape* groundShape = new btBoxShape(btVector3(30, 0.1, 30));
	btCollisionShape* sphereShape = new btSphereShape(1.0f);

	//create the player ball and the level
	entities->createEntity("sphere.obj", "checker.png", glm::vec3(0, 3, 0), glm::quat(0, 0, 0, 1), sphereShape, btScalar(1), &btVector3(0, 0, 0));
	entities->createEntity("ball_testCourse.obj", "test_texture.png", glm::vec3(0, 0, 0), glm::quat(1, 0, 0, 0), NULL); //put NULL in last parameter to have the mesh be the collsion mesh also

	//Set ball physical properties
	entities->getEntity(0)->setRestitution(0.8f);
	entities->getEntity(0)->getRigidBody()->setRollingFriction(0.3f);
	entities->getEntity(0)->setFriction(0.8f);
	//Make sure ball doesn't get deactivated by Bullet if resting too long
	entities->getEntity(0)->getRigidBody()->setActivationState(DISABLE_DEACTIVATION);

	//set level physical properties
	entities->getEntity(1)->setRestitution(0.5f);
	entities->getEntity(1)->setFriction(1);
	entities->getEntity(1)->getRigidBody()->setRollingFriction(0.5f);

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
	glm::vec3 lightPos = glm::vec3(0, 20, 0);

	bool mouseLock = 1;
	bool L_keyDown = 0;

	try{
		do{
			// Measure speed
			double currentTime = glfwGetTime();
			nbFrames++;
			if (currentTime - lastTime >= 1.0)
			{ // If last prinf() was more than 1sec ago
				// printf and reset
				printf("%f ms/frame %f FPS\n", 1000.0 / double(nbFrames), double(nbFrames));
				nbFrames = 0;
				lastTime += 1.0;
			}

			//step the physics
			dynamicsWorld->stepSimulation(1 / 144.0f, 10);
			entities->updateAll();

			// Render to our framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
			glViewport(0, 0, 1024, 1024); // Render on the whole framebuffer, complete from the lower left corner to the upper right

			// We don't use bias in the shader, but instead we draw back faces, 
			// which are already separated from the front faces by a small distance 
			// (if your geometry is made this way)
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles

			// Clear the screen
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Use our shader
			glUseProgram(depthProgramID);

			glm::vec3 lightInvDir = glm::vec3(0.5f, 2, 2);

			// Compute the MVP matrix from the light's point of view
			glm::mat4 depthProjectionMatrix = glm::ortho<float>(-40, 40, -40, 40, -30, 50);
			glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			//glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, &depthMVP[0][0]);
			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform

			entities->getModMan()->clearDepthMVP();
			entities->drawAll(&depthProjectionMatrix, &depthViewMatrix, 1, &depthMatrixID);

			// Render to the screen
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, 1024, 768); // Render on the whole framebuffer, complete from the lower left corner to the upper right

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles

			// Clear the screen
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Use our shader
			glUseProgram(programID);

			//Unlock mouse from program
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

			//glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
			glUniform3f(lightInvDirID, lightInvDir.x, lightInvDir.y, lightInvDir.z);
			
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			glUniform1i(ShadowMapID, 1);

			//Update and draw all entities
			entities->drawAll(&ProjectionMatrix, &ViewMatrix, 0, &DepthBiasID);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);


			// Optionally render the shadowmap (for debug only)

			// Render only on a corner of the window (or we we won't see the real rendering...)
			glViewport(0, 0, 256, 256);
			// Use our shader
			glUseProgram(quad_programID);
			// Bind our texture in Texture Unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			// Set our "renderedTexture" sampler to user Texture Unit 0
			glUniform1i(texID, 0);
			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
			glVertexAttribPointer(
				0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
				);

			// Draw the triangle !
			// You have to disable GL_COMPARE_R_TO_TEXTURE above in order to see anything !
			glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
			glDisableVertexAttribArray(0);

			// Swap buffers
			glfwSwapBuffers(window);
			glfwPollEvents();
		} // Check if the ESC key was pressed or the window was closed
		while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
	}
	catch (const char *str)
	{
		std::cout << str << std::endl;
	}


	// Cleanup VBO and shader
	glDeleteProgram(programID);
	glDeleteProgram(depthProgramID);
	glDeleteTextures(1, &TextureID);
	glDeleteFramebuffers(1, &FramebufferName);
	glDeleteTextures(1, &depthTexture);
	glDeleteVertexArrays(1, &VertexArrayID);


	//Delete entity manager and physics manager
	delete entities;
	delete physMan;

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	//system("pause");

	return 0;
}