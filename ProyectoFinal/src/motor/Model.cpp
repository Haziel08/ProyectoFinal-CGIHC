//>>>>>>>>>>>>>>>>>>>>>> Model >>>>>>>>>>>>>>>>>>>>>>
#include "Model.h"
#include <unordered_map>
#include <fstream>
#include <unordered_set>

// Carpeta del archivo del modelo, p.ej. "assets/planeta1/modelos/"
static std::string CarpetaDe(const std::string& ruta)
{
	size_t idx = ruta.find_last_of("/\\");
	return (idx == std::string::npos) ? std::string("") : ruta.substr(0, idx + 1);
}

static bool ExisteArchivo(const std::string& ruta)
{
	std::ifstream f(ruta.c_str());
	return f.good();
}



Model::Model()
{
}

void Model::LoadModel(const std::string & file_name, bool aplicar_transformaciones, bool invertir_normales)
{
	normales_geometricas = !invertir_normales;
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(file_name, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices
		| (aplicar_transformaciones ? aiProcess_PreTransformVertices : 0));
	if (!scene)
	{	
		printf("Falló en cargar el modelo: %s (%s)\n", file_name.c_str(), importer.GetErrorString());
		return;
	}
	carpeta_modelo = CarpetaDe(file_name);
	LoadNode(scene->mRootNode, scene);
	LoadMaterials(scene);
	}

void Model::ClearModel()
{
	for (unsigned int i = 0; i < mesh_list.size(); i++)
	{
		if (mesh_list[i])
		{
			delete mesh_list[i];
			mesh_list[i] = nullptr;

		}
	}

	std::unordered_set<Texture*> liberadas;
	for (unsigned int i = 0; i < texture_list.size(); i++)
	{
		if (texture_list[i])
		{
			if (liberadas.insert(texture_list[i]).second) delete texture_list[i];
			texture_list[i] = nullptr;
		}
	}

}

void Model::RenderModel(bool aplicar_materiales, GLint specular_location, GLint shininess_location)
{
	GLint programa_material = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programa_material);
	GLint uniform_emision = glGetUniformLocation(programa_material, "emision_material");
	GLint programa = 0;
	GLint uniform_normales = -1;
	if (normales_geometricas)
	{
		glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
		uniform_normales = glGetUniformLocation(programa, "normales_geometricas");
		glUniform1i(uniform_normales, 1);
	}
	for (unsigned int i = 0; i < mesh_list.size(); i++)
	{
		unsigned int material_index = mesh_to_texture[i];
		glUniform1f(uniform_emision, emision_material[material_index]);
		if (aplicar_materiales && material_index < intensidad_especular.size())
		{
			glUniform1f(specular_location, intensidad_especular[material_index]);
			glUniform1f(shininess_location, brillo_material[material_index]);
		}
		if (material_index < texture_list.size() && texture_list[material_index])
		{
			texture_list[material_index]->UseTexture();
		}
		// Desplazamiento de profundidad para imagenes superpuestas con mascara alfa.
		if (separar_superficie[material_index])
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(-2.0f, -8.0f);
		}
		mesh_list[i]->RenderMeshModel();
		if (separar_superficie[material_index]) glDisable(GL_POLYGON_OFFSET_FILL);

	}
	if (normales_geometricas) glUniform1i(uniform_normales, 0);
	glUniform1f(uniform_emision, 0.0f);
}


Model::~Model()
{
}

void Model::LoadNode(aiNode * node, const aiScene * scene)
{
	for (unsigned int i = 0; i <node->mNumMeshes; i++)
	{
		LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		LoadNode(node->mChildren[i], scene);
	}
}

void Model::LoadMesh(aiMesh * mesh, const aiScene * scene)
{

	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		vertices.insert(vertices.end(), { mesh->mVertices[i].x,mesh->mVertices[i].y ,mesh->mVertices[i].z });
		//UV
		if (mesh->mTextureCoords[0])//si tiene coordenadas de texturizado
		{
			vertices.insert(vertices.end(), { mesh->mTextureCoords[0][i].x,mesh->mTextureCoords[0][i].y});
		}
		else
		{
			vertices.insert(vertices.end(), { 0.0f,0.0f });
		}
		// Normales invertidas para la convencion del shader de iluminacion.
		
		float signo = normales_geometricas ? 1.0f : -1.0f;
		vertices.insert(vertices.end(), { signo * mesh->mNormals[i].x, signo * mesh->mNormals[i].y, signo * mesh->mNormals[i].z });
	}
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	MeshModel* new_mesh = new MeshModel();
	new_mesh->CreateMeshModel(&vertices[0], &indices[0], vertices.size(), indices.size());
	mesh_list.push_back(new_mesh);
	mesh_to_texture.push_back(mesh->mMaterialIndex);
}

void Model::LoadMaterials(const aiScene * scene)
{
	std::unordered_map<std::string, Texture*> loaded_textures; // Mapa para evitar duplicados
	texture_list.resize(scene->mNumMaterials);
	intensidad_especular.resize(scene->mNumMaterials, 0.25f);
	brillo_material.resize(scene->mNumMaterials, 32.0f);
	separar_superficie.resize(scene->mNumMaterials, false);
	emision_material.resize(scene->mNumMaterials, 0.0f);
	for (unsigned int i = 0; i < scene ->mNumMaterials; i++)
	{
		aiMaterial* material = scene->mMaterials[i];
		aiString nombre_material;
		material->Get(AI_MATKEY_NAME, nombre_material);
		if (std::string(nombre_material.C_Str()) == "cristal") emision_material[i] = 0.8f;
		if (std::string(nombre_material.C_Str()) == "borde_rojo") emision_material[i] = 1.6f;
		if (std::string(nombre_material.C_Str()) == "borde_blanco") emision_material[i] = 1.25f;
		aiColor3D especular(0.25f, 0.25f, 0.25f);
		material->Get(AI_MATKEY_COLOR_SPECULAR, especular);
		intensidad_especular[i] = (especular.r + especular.g + especular.b) / 3.0f;
		material->Get(AI_MATKEY_SHININESS, brillo_material[i]);
		separar_superficie[i] = material->GetTextureCount(aiTextureType_OPACITY) > 0;
		texture_list[i] = nullptr;
		// GLB: imágenes empacadas en el propio modelo, sin reconstruir sus piezas.
		aiString embedded_path;
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &embedded_path) == AI_SUCCESS)
		{
			const aiTexture* embedded = scene->GetEmbeddedTexture(embedded_path.C_Str());
			if (embedded && embedded->mHeight == 0)
			{
				Texture* tex = new Texture();
				if (tex->LoadMemory(reinterpret_cast<const unsigned char*>(embedded->pcData), static_cast<int>(embedded->mWidth)))
				{
					texture_list[i] = tex;
					continue;
				}
				delete tex;
			}
		}
		if (material->GetTextureCount(aiTextureType_DIFFUSE	))
		{
			aiString path;
			if (material->GetTexture(aiTextureType_DIFFUSE,0,&path)==AI_SUCCESS)
			{
				// Quitar del path todo lo que esté antes del último \ o / (Blender guarda rutas absolutas)
				std::string original(path.data);
				size_t idx = original.find_last_of("/\\");
				std::string filename = (idx == std::string::npos) ? original : original.substr(idx + 1);

				// Se busca la textura: 1) junto al .obj  2) en la carpeta ../texturas/ de la escena
				std::string tex_path = carpeta_modelo + filename;
				if (!ExisteArchivo(tex_path))
					tex_path = carpeta_modelo + "../texturas/" + filename;
				// Verifica si ya se cargó esta textura antes
				auto it = loaded_textures.find(tex_path);
				if (it != loaded_textures.end())
				{
					texture_list[i] = it->second; // Reutiliza la textura ya cargada
				}
				else
				{
					Texture* new_tex = new Texture(tex_path.c_str());
					std::string ext = filename.substr(filename.find_last_of('.') + 1);

					bool loaded = false;
					if (ext == "tga" || ext=="png")
					{
						loaded = new_tex->LoadTextureA();
					}
					else
					{
						loaded = new_tex->LoadTexture();
					}

					if (loaded)
					{
						texture_list[i] = new_tex;
						loaded_textures[tex_path] = new_tex; // Almacena para reutilizarla
					}
					else
					{
						printf("Falló en cargar la Textura :%s\n", tex_path.c_str());
						delete new_tex;
					}
				}
			}
		}
		//Si no se pudo cargar la textura o no tiene una textura asignada, asignar una textura por defecto
		if (!texture_list[i])
		{
			aiColor3D diffuse(1.0f, 1.0f, 1.0f);
			material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
			texture_list[i] = new Texture();
			texture_list[i]->LoadColor(diffuse.r, diffuse.g, diffuse.b);
		}

	}
}
