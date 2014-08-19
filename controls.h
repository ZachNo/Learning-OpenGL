#ifndef CONTROLS
#define CONTROLS

//Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "entity.h"

void computeMatricesFromInputs(GLFWwindow* window, float* horzAng, float* vertAng, float fov, Entity* ent, glm::mat4* ViewMatrix, glm::mat4* ProjectionMatrix);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

#endif