#pragma once

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include "Mesh.h"
#include "Shader.h"

#include <string>
#include <iostream>
#include <vector>

enum mdirections { NORMAL = 1, INVERSE = -1 };
enum moves { DLEFT, DRIGHT, DROLLR, DROLLL, DUP, DDOWN, MUP, MRIGHT, MFRONT, MLEFT, MBACK, MDOWN, MVERTICAL, MMIDDLE, MHORIZONTAL };
enum faces { FYELLOW, FGREEN, FRED, FBLUE, FORANGE, FWHITE };
enum axi { XAXIS, YAXIS, ZAXIS };
enum pieces { CORNER, EDGEU, EDGER, EDGEL, EDGED, CENTER, NONE };
enum textureTypes { TEXTURE, CUBEMAP,MATERIAL };
enum cameraMovement { FORWARD, BACKWARD, LEFT, RIGHT };

static GLuint LoadTextureFromImage(int type, std::vector<std::string> paths);

static GLuint LoadTextureFromImage(int type, std::vector<std::string> paths)
{
	GLuint texture;
	glGenTextures(1, &texture);

	switch (type)
	{
	case CUBEMAP:
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
		break;
	case TEXTURE:
	case MATERIAL:
		glBindTexture(GL_TEXTURE_2D, texture);
	}


	int width, height, nrChannels;
	GLenum format;
	for (size_t i = 0; i < paths.size(); i++)
	{
		unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			switch (type)
			{
			case CUBEMAP:
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				break;
			case TEXTURE:
			case MATERIAL:
				if (nrChannels == 1)
					format = GL_RED;
				else if (nrChannels == 3)
					format = GL_RGB;
				else if (nrChannels == 4)
					format = GL_RGBA;
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);

				if (type == TEXTURE)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
				}
				else if (type == MATERIAL)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				}
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				break;
			}
		}
		else
		{
			std::cout << "Texture failed to load at path: " << paths[i] << std::endl;
		}
		stbi_image_free(data);
	}

	return texture;
}