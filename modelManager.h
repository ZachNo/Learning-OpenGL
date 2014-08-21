#ifndef Z_MODELS
#define Z_MODELS

#include <vector>
#include <string>

//Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GL/glew.h>

//Include GLFW
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <btBulletDynamicsCommon.h>

#include "textureManager.h"

#include "error.h"

class ModelManager
{
	std::vector<GLuint> indices;
	std::vector<GLuint> vertices;
	std::vector<GLuint> uvs;
	std::vector<GLuint> normals;
	std::vector<GLuint> indicesSize;

	std::vector<std::string> filenames;

	std::vector<btCollisionShape*> colShapes;
	std::vector<btTriangleIndexVertexArray*> vertexArrays;
	std::vector<btTriangleMesh*> triangleMeshes;

	GLuint texID;
	GLuint MatrixID;
	GLuint ViewMatrixID;
	GLuint ModelMatrixID;

	GLuint checkIfModelExists(std::string filepath);
public:
	ModelManager()
	{};
	ModelManager(GLuint TextureID, GLuint matID, GLuint VMID, GLuint MMID)
	{
		texID = TextureID;
		MatrixID = matID;
		ViewMatrixID = VMID;
		ModelMatrixID = MMID;
	};
	~ModelManager();

	GLuint newModel(std::string filepath, bool useMeshAsColShape);
	bool draw(GLuint index, GLuint texIndex, glm::vec3 pos, glm::quat rot, glm::vec3 scale, glm::mat4* projMat, glm::mat4* viewMat);
	btCollisionShape* getColShape(GLuint index)
	{ return colShapes.at(index); }
};

#endif