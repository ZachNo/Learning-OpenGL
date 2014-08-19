#include "textureManager.h"

GLuint TextureManager::checkIfTextureExists(std::string filepath)
{
	if (fileNames.size() < 1)
		return -1;

	for (GLuint i = 0; i < fileNames.size(); i++)
	{
		if (!filepath.compare(fileNames.at(i)))
			return textures.at(i);
	}
	return -1;
}

GLuint TextureManager::importTexture(std::string filepath)
{
	GLuint preCheck = checkIfTextureExists(filepath);
	if (preCheck != -1)
		return preCheck;

	int width, height, comp;
	unsigned char *image = stbi_load(filepath.c_str(), &width, &height, &comp, NULL);
	if (image == NULL)
	{
		std::string err = "Image failed to load!(" + filepath + ")";
		reportError(err, 0);
		return -1;
	}

	fileNames.push_back(filepath);

	GLuint textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);

	if (comp == 3)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	else if (comp == 4)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	stbi_image_free(image);

	textures.push_back(textureID);

	return textureID;
}