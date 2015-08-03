#ifndef _SHADERLOADING_H_
#define _SHADERLOADING_H_

#include "glm_includes.h"
#include "gl_core_4_4.h"

bool LoadShader(const char* a_filename, GLenum a_shaderType, unsigned int* output);

bool CreateShaderProgram(const char* a_vertex_filename, const char* a_geometry_filename, const char* a_fragment_filename, GLuint *result);

#endif // !_SHADERLOADING_H_
