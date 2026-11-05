//>>>>>>>>>>>>>>>>>>>>>> Mesh_tn >>>>>>>>>>>>>>>>>>>>>>
#include "Mesh_tn.h"

Mesh::Mesh()
{
	vao = 0;
	vbo = 0;
	ibo = 0;
	index_count = 0;

}
MeshColor::MeshColor()
{
	vao = 0;
	vbo = 0;
	vertex_count = 0;

}

MeshModel::MeshModel()
{
	vao = 0;
	vbo = 0;
	ibo = 0;
	index_count = 0;
}


void Mesh::CreateMesh(GLfloat* vertices, unsigned int* indices, unsigned int num_of_vertices, unsigned int number_of_indices)
{

	index_count = number_of_indices;
	glGenVertexArrays(1, &vao); //generar 1 vao
	glBindVertexArray(vao);//asignar vao

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * number_of_indices, indices, GL_STATIC_DRAW);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * num_of_vertices, vertices, GL_STATIC_DRAW); //pasarle los datos al vbo asignando tamaño, los datos y en este caso es estático pues no se modificarán los valores

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void Mesh::CreateMeshGeometry(std::vector<GLfloat>& vertices, std::vector<unsigned int>& indices, unsigned int num_of_vertices, unsigned int number_of_indices)
{

	index_count = number_of_indices;
	glGenVertexArrays(1, &vao); //generar 1 vao
	glBindVertexArray(vao);//asignar vao

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * number_of_indices, &indices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * num_of_vertices, &vertices[0], GL_STATIC_DRAW); //pasarle los datos al vbo asignando tamaño, los datos y en este caso es estático pues no se modificarán los valores

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void MeshColor::CreateMeshColor(GLfloat* vertices, unsigned int num_of_vertices)
{
	vertex_count = num_of_vertices;
	glGenVertexArrays(1, &vao); //generar 1 vao
	glBindVertexArray(vao);//asignar vao

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * num_of_vertices, vertices, GL_STATIC_DRAW); //pasarle los datos al vbo asignando tamaño, los datos y en este caso es estático pues no se modificarán los valores
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 6, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 6, (void*)(sizeof(vertices[0]) * 3));//Offset de los 3 vértices para tomar los 3 de color
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}



void MeshModel::CreateMeshModel(GLfloat* vertices, unsigned int* indices, unsigned int num_of_vertices, unsigned int num_of_indices)
{
	index_count = num_of_indices;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * num_of_indices, indices, GL_STATIC_DRAW);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * num_of_vertices, vertices, GL_STATIC_DRAW);
	//geometría
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, 0);
	glEnableVertexAttribArray(0);
	//ST: Texturizado
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, (void*)(sizeof(vertices[0]) * 3));
	glEnableVertexAttribArray(1);
	//Normales en vértices
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, (void*)(sizeof(vertices[0]) * 5));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void Mesh::RenderMesh()
{
	////////////Para dibujar desde los índices
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void MeshColor::RenderMeshColor()
{
	////////////Para dibujar desde los vértices
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	glBindVertexArray(0);
}

void Mesh::RenderMeshGeometry()
{
	////////////Para dibujar desde los índices
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glDrawElements(GL_TRIANGLE_FAN, index_count, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}



void MeshModel::RenderMeshModel()
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}



void Mesh::ClearMesh()
{
	if (ibo != 0)
	{
		glDeleteBuffers(1, &ibo); ///LIMPIAR BUFFER PARAE EVITAR OCUPAR LA MEMORIA
		ibo = 0;
	}
	if (vbo != 0)
	{
		glDeleteBuffers(1, &vbo); ///LIMPIAR BUFFER PARAE EVITAR OCUPAR LA MEMORIA
		vbo = 0;
	}
	if (vao != 0)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
	index_count = 0;
}

void MeshColor::ClearMeshColor()
{

	if (vbo != 0)
	{
		glDeleteBuffers(1, &vbo); ///LIMPIAR BUFFER PARAE EVITAR OCUPAR LA MEMORIA
		vbo = 0;
	}
	if (vao != 0)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
	vertex_count = 0;
}

void MeshModel::ClearMeshModel()
{
	if (ibo != 0)
	{
		glDeleteBuffers(1, &ibo);
		ibo = 0;
	}

	if (vbo != 0)
	{
		glDeleteBuffers(1, &vbo);
		vbo = 0;
	}

	if (vao != 0)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}

	index_count = 0;
}

Mesh::~Mesh()
{
	ClearMesh();
}
MeshColor::~MeshColor()
{
	ClearMeshColor();
}

MeshModel::~MeshModel()
{
	ClearMeshModel();
};
