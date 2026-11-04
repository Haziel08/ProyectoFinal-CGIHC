#include "Terreno.h"

#include <vector>
#include <cmath>
#include <gtc/type_ptr.hpp>

Terreno::~Terreno()
{
	delete tierra;
	delete fondo;
}

float Terreno::DistanciaLago(float x, float z) const
{
	// Distancia "ovalada": vale 1 justo en la orilla del lago
	float dx = (x - centroLago.x) / radioLago.x;
	float dz = (z - centroLago.y) / radioLago.y;
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
		// Dentro del lago: baja suave desde la orilla y luego queda un fondo casi plano
		float t = glm::smoothstep(1.0f, 0.55f, d);
		float ondulacionFondo = 0.8f * sinf(x * 0.15f) * sinf(z * 0.12f);
		return -profundidad * t + ondulacionFondo * t;
	}

	// Tierra: colinas suaves que crecen al alejarse del lago (cerca de la orilla es plano)
	float colinas = 2.0f + 1.5f * sinf(x * 0.035f) * sinf(z * 0.03f + 1.0f) + 0.5f * sinf(x * 0.11f + z * 0.07f);
	return colinas * glm::smoothstep(1.0f, 1.6f, d);
}

glm::vec3 Terreno::NormalEn(float x, float z) const
{
	// Pendiente por diferencias: cuanto cambia la altura al moverse un poco en X y Z
	float e = 0.5f;
	float hx = (Altura(x + e, z) - Altura(x - e, z)) / (2.0f * e);
	float hz = (Altura(x, z + e) - Altura(x, z - e)) / (2.0f * e);
	return glm::normalize(glm::vec3(-hx, 1.0f, -hz));
}

void Terreno::AgregarCuadro(std::vector<GLfloat>& v, std::vector<unsigned int>& idx, float x0, float z0, float lado)
{
	// 4 esquinas del cuadrito: x, y (altura), z | u, v | normal invertida (convencion del shader)
	float xs[4] = { x0, x0 + lado, x0 + lado, x0 };
	float zs[4] = { z0, z0, z0 + lado, z0 + lado };
	unsigned int b = (unsigned int)(v.size() / 8);   // numero del primer vertice nuevo
	for (int k = 0; k < 4; k++)
	{
		glm::vec3 n = NormalEn(xs[k], zs[k]);
		v.insert(v.end(), { xs[k], Altura(xs[k], zs[k]), zs[k],
							xs[k] / metrosPorTextura, zs[k] / metrosPorTextura,
							-n.x, -n.y, -n.z });
	}
	// 2 triangulos por cuadrito
	idx.insert(idx.end(), { b, b + 1, b + 2,  b + 2, b + 3, b });
}

void Terreno::Crear()
{
	std::vector<GLfloat> vTierra, vFondo;          // vertices del pasto y de la arena
	std::vector<unsigned int> iTierra, iFondo;     // indices del pasto y de la arena

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
				AgregarCuadro(vFondo, iFondo, x0, z0, lado);
			else
				AgregarCuadro(vTierra, iTierra, x0, z0, lado);
		}
	}

	tierra = new MeshModel();
	tierra->CreateMeshModel(vTierra.data(), iTierra.data(), (unsigned int)vTierra.size(), (unsigned int)iTierra.size());
	fondo = new MeshModel();
	fondo->CreateMeshModel(vFondo.data(), iFondo.data(), (unsigned int)vFondo.size(), (unsigned int)iFondo.size());

	texturaPasto = Texture("assets/planeta_azul/texturas/pasto.jpg");
	texturaPasto.LoadTextureA();
	texturaArena = Texture("assets/planeta_azul/texturas/arena.jpg");
	texturaArena.LoadTextureA();
}

void Terreno::Dibujar(const ContextoRender& ctx)
{
	glm::mat4 model(1.0f);                        // el terreno ya esta en coordenadas del mundo
	glm::vec3 blanco(1.0f);
	glUniformMatrix4fv(ctx.uModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(ctx.uColor, 1, glm::value_ptr(blanco));
	materialSuelo.UseMaterial(ctx.uSpecularIntensity, ctx.uShininess);

	texturaPasto.UseTexture();
	tierra->RenderMeshModel();

	texturaArena.UseTexture();
	fondo->RenderMeshModel();

}
