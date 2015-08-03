#include "ShaderLoading.h"

bool CreateShaderProgram(const char* a_vertex_filename, const char* a_geometry_filename, const char* a_fragment_filename, GLuint *result)
{
	bool succeeded = true;

	//init program
	*result = glCreateProgram();

	//attach shaders

	//create vertex shader
#pragma region Vertex
	unsigned int vertex_shader;

	//load shader
	LoadShader(a_vertex_filename, GL_VERTEX_SHADER, &vertex_shader);

	//attach shader
	glAttachShader(*result, vertex_shader);
	glDeleteShader(vertex_shader);

#pragma endregion

	//create geometry shader
#pragma region Geometry

	if (a_geometry_filename != nullptr)
	{
		unsigned int geometry_shader;

		//load shader
		if (LoadShader(a_geometry_filename, GL_GEOMETRY_SHADER, &geometry_shader))
		{
			//attach shader
			glAttachShader(*result, geometry_shader);
			glDeleteShader(geometry_shader);
		}

	}

#pragma endregion

	//create fragment shader
#pragma region Fragment

	if (a_fragment_filename != nullptr)
	{
		unsigned int fragment_shader;

		//load shader
		if (LoadShader(a_fragment_filename, GL_FRAGMENT_SHADER, &fragment_shader))
		{
			//attach shader
			glAttachShader(*result, fragment_shader);
			glDeleteShader(fragment_shader);
		}

	}

#pragma endregion


	//link program
	glLinkProgram(*result);

	//check if the program was NOT created correctly
	GLint success;
	glGetProgramiv(*result, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		//get log length
		GLint log_length = 0;
		glGetProgramiv(*result, GL_INFO_LOG_LENGTH, &log_length);

		//get log
		char* log_msg = new char[log_length];
		glGetProgramInfoLog(*result, log_length, 0, log_msg);

		//print log
		printf("ERROR: Shader Program compile failed! \n	Error Log: \n ");
		printf("%s", log_msg);

		//delete log
		delete[] log_msg;

		//shader program creation failed
		succeeded = false;
	}

	return succeeded;
}

bool LoadShader(const char* a_filename, GLenum a_shaderType, unsigned int* output)
{
	bool succeeded = false;

	//open shader file to read from
	FILE* shader_file = fopen(a_filename, "r");

	//check if file failed to open
	if (shader_file == nullptr)
	{
		printf("ERROR: Failed to open shader file: \"%s\" \n", a_filename);

		return false;	// can return straight away because the file didn't open so we dont have to close it.
	}
	////  file opened	////

	//get how long the file is
	fseek(shader_file, 0, SEEK_END);
	int file_length = ftell(shader_file);
	fseek(shader_file, 0, SEEK_SET);

	//allocate space for the file
	char* shader_source = new char[file_length];

	//read the file and update the length to be accurate
	file_length = fread(shader_source, 1, file_length, shader_file);

	succeeded = true;

	//create shader based on the type passed in
	unsigned int shader_handle = glCreateShader(a_shaderType);

	//compile shader
	glShaderSource(shader_handle, 1, &shader_source, &file_length);
	glCompileShader(shader_handle);

	//make sure shader was compiled correctly
	int success = GL_FALSE;
	glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE)
	{
		//get log length
		int log_length = 0;
		glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &log_length);

		//get log 
		GLchar* log_msg = new char[log_length];
		glGetShaderInfoLog(shader_handle, log_length, 0, log_msg);

		//print log
		printf("ERROR: shader compile failed! File: %s \n	Error Log: \n", a_filename);
		printf("%s \n", log_msg);

		//delete log
		delete[] log_msg;

		succeeded = false;
	}

	//only give result if succeeded
	if (succeeded)
	{
		*output = shader_handle;
	}

	//cleanup
	delete[] shader_source;

	//close file
	fclose(shader_file);

	return succeeded;
}