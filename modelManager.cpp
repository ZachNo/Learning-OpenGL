#include "modelManager.h"

GLuint ModelManager::checkIfModelExists(std::string filepath)
{
	if (filenames.size() < 1)
		return -1;

	for (GLuint i = 0; i < filenames.size(); i++)
	{
		if (!filepath.compare(filenames.at(i)))
			return i;
	}
	return -1;
}

GLuint ModelManager::newModel(std::string filepath)
{
	GLuint precheck = checkIfModelExists(filepath);
	if (precheck != -1)
		return precheck;

	Assimp::Importer importer;

	std::vector<unsigned short> ind;
	std::vector<glm::vec3> vert;
	std::vector<glm::vec2> uv;
	std::vector<glm::vec3> norm;

	const aiScene* scene = importer.ReadFile(filepath, aiProcess_FlipUVs);
	if (!scene)
	{
		reportError("Model failed to import!", 1);
		reportError(importer.GetErrorString(), 0);
		return -1;
	}
	
	filenames.push_back(filepath);

	const aiMesh *mesh = scene->mMeshes[0];

	vert.reserve(mesh->mNumVertices);
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		aiVector3D pos = mesh->mVertices[i];
		vert.push_back(glm::vec3(pos.x, pos.y, pos.z));
	}

	uv.reserve(mesh->mNumVertices);
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		aiVector3D UVW = mesh->mTextureCoords[0][i];
		uv.push_back(glm::vec2(UVW.x, UVW.y));
	}

	norm.reserve(mesh->mNumVertices);
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		aiVector3D n = mesh->mNormals[i];
		norm.push_back(glm::vec3(n.x, n.y, n.z));
	}

	ind.reserve(3 * mesh->mNumFaces);
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		ind.push_back(mesh->mFaces[i].mIndices[0]);
		ind.push_back(mesh->mFaces[i].mIndices[1]);
		ind.push_back(mesh->mFaces[i].mIndices[2]);
	}

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vert.size() * sizeof(glm::vec3), &vert[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(glm::vec2), &uv[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, norm.size() * sizeof(glm::vec3), &norm[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof(unsigned short), &ind[0], GL_STATIC_DRAW);

	vertices.push_back(vertexbuffer);
	uvs.push_back(uvbuffer);
	normals.push_back(normalbuffer);
	indices.push_back(elementbuffer);
	indicesSize.push_back(ind.size());

	return indices.size() - 1;
}

bool ModelManager::draw(GLuint index, GLuint texIndex, glm::vec3 pos, glm::quat rot, glm::vec3 scale, glm::mat4 projMat, glm::mat4 viewMat)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texIndex);
	glUniform1i(texID, 0);

	GLuint vertexbuffer = vertices.at(index);
	GLuint uvbuffer = uvs.at(index);
	GLuint normalbuffer = normals.at(index);
	GLuint elementbuffer = indices.at(index);


	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
		1,                                // attribute
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(
		2,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

	glm::mat4 RotationMatrix = mat4_cast(rot);
	glm::mat4 TranslationMatrix = translate(mat4(), pos);
	glm::mat4 ScalingMatrix = glm::scale(mat4(), scale);
	glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix * ScalingMatrix;

	glm::mat4 MVP = projMat * viewMat * ModelMatrix;

	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &viewMat[0][0]);


	// Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,      // mode
		indicesSize.at(index),    // count
		GL_UNSIGNED_SHORT,   // type
		(void*)0           // element array buffer offset
		);
	return 1;
}

ModelManager::~ModelManager()
{
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		glDeleteBuffers(1, &vertices.at(i));
		glDeleteBuffers(1, &uvs.at(i));
		glDeleteBuffers(1, &normals.at(i));
		glDeleteBuffers(1, &indices.at(i));
	}
}