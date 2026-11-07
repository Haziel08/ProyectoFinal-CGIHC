#include "Terreno.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> Terreno >>>>>>>>>>>>>>>>>>>>>>


Terreno::~Terreno()
{
	delete tierra;
	delete fondo;
}

float Terreno::DistanciaLago(float x, float z) const
{
	// Distancia
	float dx = (x - centro_lago.x) / radio_lago.x;
	float dz = (z - centro_lago.y) / radio_lago.y;
	return sqrtf(dx * dx + dz * dz);
}

bool Terreno::EsLago(float x, float z) const
{
	return DistanciaLago(x, z) < 1.06f;   // un poquito mas que la orilla = playa de arena
}

float Terreno::Altura(float x, float z) const
{
	float d = DistanciaLago(x, z);

	if (d < 1.0f)
	{
		// Dentro del lago baja suave desde la orilla y luego queda un fondo casi plano
		float t = glm::smoothstep(1.0f, 0.55f, d);
		float ondulacion_fondo = 0.8f * sinf(x * 0.15f) * sinf(z * 0.12f);
		return -profundidad * t + ondulacion_fondo * t;
	}

	// Tierra
	float colinas = 2.0f + 1.5f * sinf(x * 0.035f) * sinf(z * 0.03f + 1.0f) + 0.5f * sinf(x * 0.11f + z * 0.07f);
	return colinas * glm::smoothstep(1.0f, 1.6f, d);
}

glm::vec3 Terreno::NormalEn(float x, float z) const
{
	// Pendiente por diferencias cuanto cambia la altura al moverse un poco en X y Z
	float e = 0.5f;
	float hx = (Altura(x + e, z) - Altura(x - e, z)) / (2.0f * e);
	float hz = (Altura(x, z + e) - Altura(x, z - e)) / (2.0f * e);
	return glm::normalize(glm::vec3(-hx, 1.0f, -hz));
}

void Terreno::AgregarCuadro(std::vector<GLfloat>& v, std::vector<unsigned int>& idx, float x0, float z0, float lado)
{
	// 4 esquinas del cuadrito
	float xs[4] = { x0, x0 + lado, x0 + lado, x0 };
	float zs[4] = { z0, z0, z0 + lado, z0 + lado };
	unsigned int b = (unsigned int)(v.size() / 8);   // numero del primer vertice nuevo
	for (int k = 0; k < 4; k++)
	{
		glm::vec3 n = NormalEn(xs[k], zs[k]);
		v.insert(v.end(), { xs[k], Altura(xs[k], zs[k]), zs[k],
							xs[k] / metros_por_textura, zs[k] / metros_por_textura,
							-n.x, -n.y, -n.z });
	}
	// 2 triangulos por cuadrito
	idx.insert(idx.end(), { b, b + 1, b + 2,  b + 2, b + 3, b });
}

void Terreno::Crear()
{
	std::vector<GLfloat> v_tierra, v_fondo;          // vertices del pasto y de la arena
	std::vector<unsigned int> i_tierra, i_fondo;     // indices del pasto y de la arena

	float lado = tamano / celdas;
	float inicio = -tamano / 2.0f;

	for (int i = 0; i < celdas; i++)
	{
		for (int j = 0; j < celdas; j++)
		{
			float x0 = inicio + i * lado;
			float z0 = inicio + j * lado;

			// Si el centro del cuadrito cae en el lago va a la malla de arena, si no a la de pasto
			if (EsLago(x0 + lado / 2, z0 + lado / 2))
				AgregarCuadro(v_fondo, i_fondo, x0, z0, lado);
			else
				AgregarCuadro(v_tierra, i_tierra, x0, z0, lado);
		}
	}

	tierra = new MeshModel();
	tierra->CreateMeshModel(v_tierra.data(), i_tierra.data(), (unsigned int)v_tierra.size(), (unsigned int)i_tierra.size());
	fondo = new MeshModel();
	fondo->CreateMeshModel(v_fondo.data(), i_fondo.data(), (unsigned int)v_fondo.size(), (unsigned int)i_fondo.size());

	textura_pasto = Texture("assets/planeta_azul/texturas/pasto.jpg");
	textura_pasto.LoadTextureA();
	textura_arena = Texture("assets/planeta_azul/texturas/arena.jpg");
	textura_arena.LoadTextureA();
}

void Terreno::Dibujar(const ContextoRender& ctx)
{
	glm::mat4 model(1.0f);                        
	glm::vec3 blanco(1.0f);
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));
	material_suelo.UseMaterial(ctx.u_specular_intensity, ctx.u_shininess);

	textura_pasto.UseTexture();
	tierra->RenderMeshModel();

	textura_arena.UseTexture();
	fondo->RenderMeshModel();

}
}
