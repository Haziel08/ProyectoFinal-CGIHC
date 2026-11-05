#pragma once
//>>>>>>>>>>>>>>>>>>>>>> Definiciones de Mesh_tn >>>>>>>>>>>>>>>>>>>>>>
#include <glew.h>
#include <vector>

class Mesh
{
public:
	Mesh();

	void CreateMesh(GLfloat* vertices, unsigned int* indices, unsigned int num_of_vertices, unsigned int number_of_indices);
	void CreateMeshGeometry(std::vector<GLfloat>& vertices, std::vector<unsigned int>& indices, unsigned int num_of_vertices, unsigned int number_of_indices);
	void RenderMesh();
	void RenderMeshGeometry();
	void ClearMesh();
	~Mesh();
private:
	GLuint vao, vbo, ibo;
	GLsizei index_count;

};

class MeshColor
{
public:
	MeshColor();
	void CreateMeshColor(GLfloat* vertices, unsigned int num_of_vertices);
	void RenderMeshColor();
	void ClearMeshColor();
	~MeshColor();
private:
	GLuint vao, vbo;
	GLsizei  vertex_count;
};

class MeshModel
{
public:
	MeshModel();

	void CreateMeshModel(GLfloat *vertices, unsigned int *indices, unsigned int num_of_vertices, unsigned int num_of_indices);
	void RenderMeshModel();
	void ClearMeshModel();

	~MeshModel();

private:
	GLuint vao, vbo, ibo;
	GLsizei index_count;
};

