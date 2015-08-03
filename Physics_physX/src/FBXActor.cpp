#include "FBXActor.h"

#include "ShaderLoading.h"

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