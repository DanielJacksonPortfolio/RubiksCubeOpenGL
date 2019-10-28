#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
	GLuint ID;

	Shader(std::string vertexPath, std::string fragmentPath)
	{
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;

		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
		
			vShaderFile.close();
			fShaderFile.close();
		
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cerr << "ERROR: Shader file not successfully read" << std::endl;
			std::cin.get();
		}
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();
		
		GLuint vertex, fragment;
		
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		CheckCompileErrors(vertex, "VERTEX");

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		CheckCompileErrors(fragment, "FRAGMENT");

		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);

		glLinkProgram(ID);
		CheckCompileErrors(ID, "PROGRAM");

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}
	void Use() { glUseProgram(ID); }

	void SetBool(	const std::string& name, bool value)								const { glUniform1i(		glGetUniformLocation(ID, name.c_str()), (int)value); }
	
	void SetInt(	const std::string& name, int value)									const { glUniform1i(		glGetUniformLocation(ID, name.c_str()), value); }
	
	void SetFloat(	const std::string& name, float value)								const { glUniform1f(		glGetUniformLocation(ID, name.c_str()), value); }

	void SetVector2(const std::string& name, const glm::vec2& value)					const { glUniform2fv(		glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }

	void SetVector2(const std::string& name, float x, float y)							const { glUniform2f(		glGetUniformLocation(ID, name.c_str()), x, y);}
	
	void SetVector3(const std::string& name, const glm::vec3& value)					const { glUniform3fv(		glGetUniformLocation(ID, name.c_str()), 1, &value[0]);}
	
	void SetVector3(const std::string& name, float x, float y, float z)					const { glUniform3f(		glGetUniformLocation(ID, name.c_str()), x, y, z);}
	
	void SetVector4(const std::string& name, const glm::vec4& value)					const { glUniform4fv(		glGetUniformLocation(ID, name.c_str()), 1, &value[0]);}
	
	void SetVector4(const std::string& name, float x, float y, float z, float w)		const { glUniform4f(		glGetUniformLocation(ID, name.c_str()), x, y, z, w);}

	void SetMatrix2(	const std::string& name, const glm::mat2& mat)					const { glUniformMatrix2fv(	glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);}
	
	void SetMatrix3(	const std::string& name, const glm::mat3& mat)					const { glUniformMatrix3fv( glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);}

	void SetMatrix4(	const std::string& name, const glm::mat4& mat)					const { glUniformMatrix4fv( glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);}

private:
	void CheckCompileErrors(GLuint shader, std::string type)
	{
		GLint success;
		GLchar error[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, error);
				std::cerr << "ERROR: Shader compile error of type: " << type << "\n" << error << "\n -- --------------------------------------------------- -- " << std::endl;
				std::cin.get();
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, error);
				std::cerr << "ERROR: Shader link error of type:  " << type << "\n" << error << "\n -- --------------------------------------------------- -- " << std::endl;
				std::cin.get();
			}
		}
	}
};