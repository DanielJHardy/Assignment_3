#ifndef _FBXACTOR_H_
#define _FBXACTOR_H_

#include <vector>
#include <FBXFile.h>

#include "glm_includes.h"
#include "shader_data_objects.h"

#include "Camera.h"

class PhysicsDemoScene;

class FBXActor
{
public:
	FBXActor();
	~FBXActor();

	bool Init(const char* a_filename);
	void createCollisionShapes(PhysicsDemoScene *a_app);

	void Update(float a_dt);
	void Render(float* a_viewProj);

	void GenerateGLMeshes();

public:
	//model file
	FBXFile* m_file;

	//model meshes
	std::vector<ShaderObjs::OpenGLData> m_meshes;

	//shader program
	unsigned int m_program;

	//world transform
	mat4 m_world;

	//should draw debug collision shapes
	//bool m_drawCollisions;

};

#endif // !_FBXACTOR_H_
