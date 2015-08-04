#include "FBXActor.h"

#include "ShaderLoading.h"

#include "PhysicsDemoScene.h"

FBXActor::FBXActor() {}
FBXActor::~FBXActor() {}

bool FBXActor::Init(const char* a_filename)
{
	//Load FBX file
	m_file = new FBXFile();
	m_file->load(a_filename);
	m_file->initialiseOpenGLTextures();
	GenerateGLMeshes();

	//world transform
	m_world = mat4(1);

	//shader program
	return CreateShaderProgram("./data/shaders/textured_vertex.glsl", nullptr, "./data/shaders/textured_fragment.glsl", &m_program);
}

void FBXActor::createCollisionShapes(PhysicsDemoScene *a_app)
{
	float density = 300;

	//pole
	PxBoxGeometry box = PxBoxGeometry(0.1f,4,0.1f);
	PxTransform transform(*((PxMat44*)(&m_world)));	//cast from glm to PhysX matrices

	PxRigidDynamic* dynamicActor = PxCreateDynamic(*a_app->g_Physics, transform, box, *a_app->g_PhysicsMaterial, density);

	dynamicActor->userData = this;	//set the user data to point at this FBXActor class

	//offset
	int nShapes = dynamicActor->getNbShapes();
	PxShape* shapes;
	dynamicActor->getShapes(&shapes, nShapes);

	PxTransform relativePose = PxTransform(PxVec3(0.0f,4.0f,0.0f));
	shapes->setLocalPose(relativePose);

	//head
	box = PxBoxGeometry(0.8f,0.5f,0.3f);
	relativePose = PxTransform(PxVec3(0.0f,2.0f,0.0f));
	PxShape* shape = dynamicActor->createShape(box, *a_app->g_PhysicsMaterial);
	if (shape)
	{
		shape->setLocalPose(relativePose);
	}

	PxRigidBodyExt::updateMassAndInertia(*dynamicActor, (PxReal)density);

	//add to scene
	a_app->g_PhysicsScene->addActor(*dynamicActor);
	a_app->g_PhysXActors.push_back(dynamicActor);

}

void FBXActor::Update(float a_dt)
{

}

void FBXActor::Render(float* a_viewProj)
{
	glUseProgram(m_program);

	//get uniforms
	int viewProjUniform = glGetUniformLocation(m_program, "projection_view");
	int diffuseUniform = glGetUniformLocation(m_program, "diffuse");

	//set uniforms
	glUniformMatrix4fv(viewProjUniform, 1, GL_FALSE, a_viewProj);
	glUniform1i(diffuseUniform, 0);

	for (unsigned int i = 0; i < m_meshes.size(); i++)
	{
		//get current mesh and material
		FBXMeshNode* currMesh = m_file->getMeshByIndex(i);
		FBXMaterial* meshMaterial = currMesh->m_material;

		//set active texture to bind to
		glActiveTexture(GL_TEXTURE0);

		//bind diffuse texture
		glBindTexture(GL_TEXTURE_2D, meshMaterial->textures[FBXMaterial::DiffuseTexture]->handle);

		//get world transform uniform
		int worldUniform = glGetUniformLocation(m_program, "world");
		glUniformMatrix4fv(worldUniform, 1, GL_FALSE, (float*)&m_world);

		//draw
		glBindVertexArray(m_meshes[i].VAO);
		glDrawElements(GL_TRIANGLES, m_meshes[i].IndexCount, GL_UNSIGNED_INT, nullptr);

	}
}

void FBXActor::GenerateGLMeshes()
{
	//get number of meshes
	unsigned int meshCount = m_file->getMeshCount();

	//create vector big enough for all meshes
	m_meshes = std::vector<ShaderObjs::OpenGLData>(meshCount);

	for (unsigned int i = 0; i < meshCount; i++)
	{
		FBXMeshNode* currMesh = m_file->getMeshByIndex(i);

		//set number of indices
		m_meshes[i].IndexCount = currMesh->m_indices.size();

		glGenBuffers(1, &m_meshes[i].VBO);
		glGenBuffers(1, &m_meshes[i].IBO);

		glGenVertexArrays(1, &m_meshes[i].VAO);
		glBindVertexArray(m_meshes[i].VAO);

		//VBO
		glBindBuffer(GL_ARRAY_BUFFER, m_meshes[i].VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(FBXVertex) * currMesh->m_vertices.size(), currMesh->m_vertices.data(), GL_STATIC_DRAW);

		//IBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshes[i].IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * currMesh->m_indices.size(), currMesh->m_indices.data(), GL_STATIC_DRAW);

		//set veretx properties

		glEnableVertexAttribArray(0);	//pos
		glEnableVertexAttribArray(1);	//tex coord

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (void*)FBXVertex::PositionOffset);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (void*)FBXVertex::TexCoord1Offset);

		//unbind
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}