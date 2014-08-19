#ifndef Z_TEX
#define Z_TEX

#include <vector>
#include <string>

//Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GL/glew.h>

//Include GLFW
#include <GLFW/glfw3.h>

//Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

//Include image loading
#include "stb_image.h"

#include "error.h"

class TextureManager
{
	std::vector<std::string> fileNames;
	GLuint checkIfTextureExists(std::string filePath);
	std::vector<GLuint> textures;
public:
	GLuint importTexture(std::string file);
};

#endif