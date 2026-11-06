#include "ModeloAnimado.h"

#include <cstdio>
#include <cmath>
#include <cstddef>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Vertice con huesos: posicion, UV, normal + 4 huesos y sus 4 pesos
struct VerticeAnimado
{
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;
	int huesos[4];
	float pesos[4];
};

// Assimp guarda las matrices por renglones y glm por columnas: hay que transponer
static glm::mat4 AGlm(const aiMatrix4x4& m)
{
	return glm::transpose(glm::make_mat4(&m.a1));
}

static const unsigned int FLAGS_ASSIMP =
	aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_LimitBoneWeights | aiProcess_JoinIdenticalVertices;

ModeloAnimado::~ModeloAnimado()
{
	if (IBO) glDeleteBuffers(1, &IBO);
	if (VBO) glDeleteBuffers(1, &VBO);
	if (VAO) glDeleteVertexArrays(1, &VAO);
}

int ModeloAnimado::CopiarNodo(const aiNode* nodo)
{
	int indice = (int)nodos.size();
	nodos.push_back({ nodo->mName.C_Str(), AGlm(nodo->mTransformation), {} });
	for (unsigned int i = 0; i < nodo->mNumChildren; i++)
	{
		int hijo = CopiarNodo(nodo->mChildren[i]);
		nodos[indice].hijos.push_back(hijo);
	}
	return indice;
}

bool ModeloAnimado::Cargar(const std::string& archivo, const std::string& rutaTextura)
{
	Assimp::Importer importer;
	// Sin nodos auxiliares de FBX: el arbol queda con un nodo por hueso
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	const aiScene* scene = importer.ReadFile(archivo, FLAGS_ASSIMP);
	if (!scene || !scene->mRootNode)
	{
		printf("No se pudo cargar el modelo animado %s: %s\n", archivo.c_str(), importer.GetErrorString());
		return false;
	}

	// --- Arbol de huesos ---
	nodos.clear();
	CopiarNodo(scene->mRootNode);
	inversaRaiz = glm::inverse(nodos[0].transformacion);

	// --- Malla: vertices con sus huesos y pesos ---
	std::vector<VerticeAnimado> vertices;
	std::vector<unsigned int> indices;
	for (unsigned int m = 0; m < scene->mNumMeshes; m++)
	{
		const aiMesh* mesh = scene->mMeshes[m];
		unsigned int base = (unsigned int)vertices.size();

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			VerticeAnimado v;
			v.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			v.uv = mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0.0f);
			// Normal invertida: misma convencion que Model y shader_light
			v.normal = -glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			for (int k = 0; k < 4; k++) { v.huesos[k] = 0; v.pesos[k] = 0.0f; }
			vertices.push_back(v);
		}

		for (unsigned int f = 0; f < mesh->mNumFaces; f++)
			for (unsigned int k = 0; k < mesh->mFaces[f].mNumIndices; k++)
				indices.push_back(base + mesh->mFaces[f].mIndices[k]);

		// Cada hueso dice a que vertices mueve y con que peso
		for (unsigned int b = 0; b < mesh->mNumBones; b++)
		{
			const aiBone* hueso = mesh->mBones[b];
			std::string nombre = hueso->mName.C_Str();
			if (indiceHueso.find(nombre) == indiceHueso.end())
			{
				indiceHueso[nombre] = (int)offsets.size();
				offsets.push_back(AGlm(hueso->mOffsetMatrix));
			}
			int id = indiceHueso[nombre];
			for (unsigned int w = 0; w < hueso->mNumWeights; w++)
			{
				VerticeAnimado& v = vertices[base + hueso->mWeights[w].mVertexId];
				for (int k = 0; k < 4; k++)          // se guarda en el primer lugar libre
				{
					if (v.pesos[k] == 0.0f)
					{
						v.huesos[k] = id;
						v.pesos[k] = hueso->mWeights[w].mWeight;
						break;
					}
				}
			}
		}
	}
	matricesHuesos.assign(offsets.size(), glm::mat4(1.0f));

	// --- Mandar la malla a la tarjeta de video ---
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VerticeAnimado), vertices.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	GLsizei paso = sizeof(VerticeAnimado);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, paso, (void*)offsetof(VerticeAnimado, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, paso, (void*)offsetof(VerticeAnimado, uv));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, paso, (void*)offsetof(VerticeAnimado, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(3, 4, GL_INT, paso, (void*)offsetof(VerticeAnimado, huesos));   // enteros: "I"
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, paso, (void*)offsetof(VerticeAnimado, pesos));
	glEnableVertexAttribArray(4);
	glBindVertexArray(0);
	numIndices = (GLsizei)indices.size();

	textura = Texture(rutaTextura.c_str());
	textura.LoadTextureA();

	printf("Modelo animado %s: %d vertices, %d huesos\n", archivo.c_str(), (int)vertices.size(), (int)offsets.size());
	LeerAnimacion(archivo);
	return true;
}

bool ModeloAnimado::LeerAnimacion(const std::string& archivo)
{
	Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	const aiScene* scene = importer.ReadFile(archivo, FLAGS_ASSIMP);
	if (!scene || scene->mNumAnimations == 0)
	{
		printf("%s no trae animacion\n", archivo.c_str());
		return false;
	}

	const aiAnimation* a = scene->mAnimations[0];
	Animacion anim;
	anim.duracionTicks = (float)a->mDuration;
	anim.ticksPorSegundo = a->mTicksPerSecond > 0.0 ? (float)a->mTicksPerSecond : 30.0f;

	for (unsigned int c = 0; c < a->mNumChannels; c++)
	{
		const aiNodeAnim* ch = a->mChannels[c];
		Canal canal;
		for (unsigned int k = 0; k < ch->mNumPositionKeys; k++)
		{
			const aiVector3D& p = ch->mPositionKeys[k].mValue;
			canal.posiciones.push_back({ (float)ch->mPositionKeys[k].mTime, glm::vec3(p.x, p.y, p.z) });
		}
		for (unsigned int k = 0; k < ch->mNumRotationKeys; k++)
		{
			const aiQuaternion& q = ch->mRotationKeys[k].mValue;
			canal.rotaciones.push_back({ (float)ch->mRotationKeys[k].mTime, glm::quat(q.w, q.x, q.y, q.z) });
		}
		for (unsigned int k = 0; k < ch->mNumScalingKeys; k++)
		{
			const aiVector3D& s = ch->mScalingKeys[k].mValue;
			canal.escalas.push_back({ (float)ch->mScalingKeys[k].mTime, glm::vec3(s.x, s.y, s.z) });
		}
		anim.canales[ch->mNodeName.C_Str()] = canal;
	}
	animaciones.push_back(anim);
	printf("Animacion %d de %s: %.1f segundos\n", (int)animaciones.size() - 1, archivo.c_str(),
		anim.duracionTicks / anim.ticksPorSegundo);
	return true;
}

int ModeloAnimado::AgregarAnimacion(const std::string& archivo)
{
	LeerAnimacion(archivo);
	return (int)animaciones.size() - 1;
}

// Busca entre que dos keyframes cae "tick" y regresa el indice del primero
template <typename T>
static int BuscarClave(const std::vector<T>& claves, float tick)
{
	for (int i = 0; i + 1 < (int)claves.size(); i++)
		if (tick < claves[i + 1].tick)
			return i;
	return (int)claves.size() - 1;
}

glm::mat4 ModeloAnimado::Interpolar(const Canal& canal, float tick) const
{
	// Posicion: interpolacion lineal entre los dos keyframes
	glm::vec3 pos(0.0f);
	if (!canal.posiciones.empty())
	{
		int i = BuscarClave(canal.posiciones, tick);
		int j = glm::min(i + 1, (int)canal.posiciones.size() - 1);
		float t = (i == j) ? 0.0f : (tick - canal.posiciones[i].tick) / (canal.posiciones[j].tick - canal.posiciones[i].tick);
		pos = glm::mix(canal.posiciones[i].valor, canal.posiciones[j].valor, t);
	}

	// Rotacion: "slerp", la interpolacion para rotaciones (cuaterniones)
	glm::quat rot(1.0f, 0.0f, 0.0f, 0.0f);
	if (!canal.rotaciones.empty())
	{
		int i = BuscarClave(canal.rotaciones, tick);
		int j = glm::min(i + 1, (int)canal.rotaciones.size() - 1);
		float t = (i == j) ? 0.0f : (tick - canal.rotaciones[i].tick) / (canal.rotaciones[j].tick - canal.rotaciones[i].tick);
		rot = glm::normalize(glm::slerp(canal.rotaciones[i].valor, canal.rotaciones[j].valor, t));
	}

	// Escala
	glm::vec3 esc(1.0f);
	if (!canal.escalas.empty())
	{
		int i = BuscarClave(canal.escalas, tick);
		int j = glm::min(i + 1, (int)canal.escalas.size() - 1);
		float t = (i == j) ? 0.0f : (tick - canal.escalas[i].tick) / (canal.escalas[j].tick - canal.escalas[i].tick);
		esc = glm::mix(canal.escalas[i].valor, canal.escalas[j].valor, t);
	}

	// Igual que en clase: trasladar * rotar * escalar
	return glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot) * glm::scale(glm::mat4(1.0f), esc);
}

void ModeloAnimado::CalcularNodo(const Animacion& anim, float tick, int indice, const glm::mat4& padre)
{
	const Nodo& nodo = nodos[indice];

	// Si el hueso tiene movimiento en la animacion se usa ese; si no, su pose de reposo
	glm::mat4 local = nodo.transformacion;
	auto canal = anim.canales.find(nodo.nombre);
	if (canal != anim.canales.end())
		local = Interpolar(canal->second, tick);

	// Jerarquia: la matriz del hijo se multiplica por la del padre
	glm::mat4 global = padre * local;

	auto hueso = indiceHueso.find(nodo.nombre);
	if (hueso != indiceHueso.end())
		matricesHuesos[hueso->second] = inversaRaiz * global * offsets[hueso->second];

	for (int hijo : nodo.hijos)
		CalcularNodo(anim, tick, hijo, global);
}

void ModeloAnimado::CalcularPose(int animacion, float segundos)
{
	if (animacion < 0 || animacion >= (int)animaciones.size() || nodos.empty())
		return;
	const Animacion& anim = animaciones[animacion];
	// Segundos -> ticks, y con fmod se repite al terminar
	float tick = fmodf(segundos * anim.ticksPorSegundo, anim.duracionTicks);
	CalcularNodo(anim, tick, 0, glm::mat4(1.0f));
}

void ModeloAnimado::Dibujar(GLuint programaShader)
{
	GLint uHuesos = glGetUniformLocation(programaShader, "huesos");
	int n = glm::min((int)matricesHuesos.size(), MAX_HUESOS);
	glUniformMatrix4fv(uHuesos, n, GL_FALSE, glm::value_ptr(matricesHuesos[0]));

	textura.UseTexture();
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
