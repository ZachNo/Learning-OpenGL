#include "modelManager.h"

//see if model already imported, if so return that number
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

//load new model into opengl
GLuint ModelManager::newModel(std::string filepath, bool useMeshAsColShape)
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

	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof(unsigned short), &ind[0], GL_STATIC_DRAW);

	vertices.push_back(vertexbuffer);
	uvs.push_back(uvbuffer);
	normals.push_back(normalbuffer);
	indices.push_back(elementbuffer);
	indicesSize.push_back(ind.size());

	//use the mesh as collsion mesh
	if (useMeshAsColShape)
	{
		btTriangleMesh* trigMesh = new btTriangleMesh;
		int totalVerts = mesh->mNumVertices;

		for (int i = 0; i < totalVerts;)
		{
			//glm::vec3 vecA = vert.at(ind.at(i++));
			aiVector3D vecA = mesh->mVertices[i++];
			btVector3 vertA(vecA.x, vecA.y, vecA.z);
			//glm::vec3 vecB = vert.at(ind.at(i++));
			aiVector3D vecB = mesh->mVertices[i++];
			btVector3 vertB(vecB.x, vecB.y, vecB.z);
			//glm::vec3 vecC = vert.at(ind.at(i++));
			aiVector3D vecC = mesh->mVertices[i++];
			btVector3 vertC(vecC.x, vecC.y, vecC.z);
			trigMesh->addTriangle(vertA, vertB, vertC, 1);

			//std::cout << vecA.x << " " << vecA.y << " " << vecA.z << " : " << vecB.x << " " << vecB.y << " " << vecB.z << " : " << vecC.x << " " << vecC.y << " " << vecC.z << std::endl;
		}

		btTriangleIndexVertexArray* indexArray = new btTriangleIndexVertexArray(*trigMesh);

		btVector3 aabbMin(-1000, -1000, -1000), aabbMax(1000, 1000, 1000);

		btCollisionShape* meshCol = new btBvhTriangleMeshShape(indexArray, 1, aabbMin, aabbMax);
		colShapes.push_back(meshCol);
		vertexArrays.push_back(indexArray);
		triangleMeshes.push_back(trigMesh);
	}

	return indices.size() - 1;
}

//draw the model
bool ModelManager::draw(GLuint index, GLuint texIndex, glm::vec3 pos, glm::quat rot, glm::vec3 scale, glm::mat4* projMat, glm::mat4* viewMat, bool drawOnlyVerts, GLuint *matID)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texIndex);
	glUniform1i(texID, 0);

	glm::mat4 RotationMatrix = mat4_cast(rot);
	glm::mat4 TranslationMatrix = translate(mat4(), pos);
	glm::mat4 ScalingMatrix = glm::scale(mat4(), scale);
	glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix * ScalingMatrix;

	glm::mat4 MVP = *projMat * *viewMat * ModelMatrix;
	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	if (matID == NULL)
	{
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	}
	else if (drawOnlyVerts)
	{
		glm::mat4 depthMVP = *projMat * *viewMat * ModelMatrix;
		glUniformMatrix4fv(*matID, 1, GL_FALSE, &depthMVP[0][0]);
		depthMVPs.push_back(depthMVP);
	}
	else if (matID != NULL && !drawOnlyVerts)
	{
		glm::mat4 depthMVP = depthMVPs.at(index);
		glm::mat4 biasMatrix(
			0.5, 0.0, 0.0, 0.0,
			0.0, 0.5, 0.0, 0.0,
			0.0, 0.0, 0.5, 0.0,
			0.5, 0.5, 0.5, 1.0
			);

		glm::mat4 depthBiasMVP = biasMatrix * depthMVP;

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(*matID, 1, GL_FALSE, &depthBiasMVP[0][0]);
	}

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
	if (!drawOnlyVerts)
	{
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
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

	// Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,      // mode
		indicesSize.at(index),    // count
		GL_UNSIGNED_SHORT,   // type
		(void*)0           // element array buffer offset
		);

	return 1;
}

//delete everything
ModelManager::~ModelManager()
{
	for (unsigned int i = 0; i > indicesSize.size(); i++)
	{
		glDeleteBuffers(1, &vertices.at(i));
		glDeleteBuffers(1, &uvs.at(i));
		glDeleteBuffers(1, &normals.at(i));
		glDeleteBuffers(1, &indices.at(i));
	}
	for (unsigned int i = 0; i < vertexArrays.size(); i++)
		delete vertexArrays.at(i);
	for (unsigned int i = 0; i < triangleMeshes.size(); i++)
		delete triangleMeshes.at(i);
}