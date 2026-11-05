#pragma once
//>>>>>>>>>>>>>>>>>>>>>> Definiciones de Model >>>>>>>>>>>>>>>>>>>>>>
#include <vector>
#include<string>
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#include "Mesh_tn.h"
#include "Texture.h"

class Model
{
public:
	Model();

	void LoadModel(const std::string& file_name, bool aplicar_transformaciones = false, bool invertir_normales = true);
	void RenderModel(bool aplicar_materiales = false, GLint specular_location = -1, GLint shininess_location = -1);
	void ClearModel();

	~Model();

private:
	void LoadNode(aiNode* node, const aiScene* scene); //assimp
	void LoadMesh(aiMesh* mesh, const aiScene* scene);
	void LoadMaterials(const aiScene* scene);
	std::vector<MeshModel*>mesh_list;
	std::vector<Texture*>texture_list;
	std::vector<unsigned int>mesh_to_texture;
	std::vector<float> intensidad_especular;
	std::vector<float> brillo_material;
	std::vector<bool> separar_superficie;
	std::vector<float> emision_material;
	bool normales_geometricas = false;
	std::string carpeta_modelo; // carpeta del archivo cargado, para buscar sus texturas
};

