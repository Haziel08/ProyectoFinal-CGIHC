//>>>>>>>>>>>>>>>>>>>>>> Model >>>>>>>>>>>>>>>>>>>>>>
#include "Model.h"
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>

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

Model::Model() {}

void Model::LoadModel(const std::string& file_name, bool aplicar_transformaciones, bool invertir_normales)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        file_name, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices |
                       (aplicar_transformaciones ? aiProcess_PreTransformVertices : 0));
    if (!scene)
    {
        printf("Falló en cargar el modelo: %s (%s)\n", file_name.c_str(), importer.GetErrorString());
        return;
    }
    // Una recarga valida sustituye el modelo anterior; un fallo conserva el que ya estaba cargado.
    ClearModel();
    normales_geometricas = !invertir_normales;
    carpeta_modelo = CarpetaDe(file_name);
    // Metadatos OBJ locales: centros luminosos ya calculados sobre las superficies LED.
    if (file_name.size() >= 4 && file_name.substr(file_name.size() - 4) == ".obj")
    {
        std::ifstream archivo(file_name);
        std::string linea;
        while (std::getline(archivo, linea))
        {
            if (linea.rfind("# emisor_led ", 0) == 0)
            {
                EmisorLed led;
                std::istringstream datos(linea.substr(13));
                if (datos >> led.posicion.x >> led.posicion.y >> led.posicion.z >> led.color.r >>
                    led.color.g >> led.color.b >> led.radio)
                    emisores_led.push_back(led);
            }
            else if (!linea.empty() && linea[0] != '#')
                break;
        }
    }
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

    texturas_noche.clear();
    tipos_emision.clear();
    emisores_led.clear();
    mesh_list.clear();
    texture_list.clear();
    mesh_to_texture.clear();
    intensidad_especular.clear();
    brillo_material.clear();
    separar_superficie.clear();
    emision_material.clear();
    opacidad_material.clear();
    carpeta_modelo.clear();
}

void Model::RenderModel(bool aplicar_materiales, GLint specular_location, GLint shininess_location)
{
    GLint programa_material = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programa_material);
    GLint uniform_emision = glGetUniformLocation(programa_material, "emision_material");
    GLint uniform_opacidad = glGetUniformLocation(programa_material, "opacidad_material");
    const GLint uniform_noche = glGetUniformLocation(programa_material, "tiene_textura_noche");
    const GLint uniform_tipo = glGetUniformLocation(programa_material, "tipo_emision");
    glUniform1i(glGetUniformLocation(programa_material, "textura_noche"), 1);
    GLint uniform_normales = -1;
    if (normales_geometricas)
    {
        uniform_normales = glGetUniformLocation(programa_material, "normales_geometricas");
        glUniform1i(uniform_normales, 1);
    }
    for (unsigned int i = 0; i < mesh_list.size(); i++)
    {
        unsigned int material_index = mesh_to_texture[i];
        glUniform1i(uniform_tipo, tipos_emision[material_index]);
        glUniform1i(uniform_noche, texturas_noche[material_index] != nullptr);
        if (texturas_noche[material_index])
            texturas_noche[material_index]->UseTexture(GL_TEXTURE1);
        glActiveTexture(GL_TEXTURE0);
        glUniform1f(uniform_emision, emision_material[material_index]);
        glUniform1f(uniform_opacidad, opacidad_material[material_index]);
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
        if (separar_superficie[material_index])
            glDisable(GL_POLYGON_OFFSET_FILL);
    }
    if (normales_geometricas)
        glUniform1i(uniform_normales, 0);
    glUniform1f(uniform_emision, 0.0f);
    glUniform1f(uniform_opacidad, 1.0f);
    glUniform1i(uniform_tipo, 0);
    glUniform1i(uniform_noche, 0);
}

Model::~Model() {}

void Model::LoadNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        LoadNode(node->mChildren[i], scene);
    }
}

void Model::LoadMesh(aiMesh* mesh, const aiScene*)
{

    std::vector<GLfloat> vertices;
    std::vector<unsigned int> indices;
    vertices.reserve(size_t(mesh->mNumVertices)*8);
    indices.reserve(size_t(mesh->mNumFaces)*3);
    const float signo = normales_geometricas ? 1.0f : -1.0f;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        vertices.insert(vertices.end(), {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z});
        // UV
        if (mesh->mTextureCoords[0]) // si tiene coordenadas de texturizado
        {
            vertices.insert(vertices.end(), {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y});
        }
        else
        {
            vertices.insert(vertices.end(), {0.0f, 0.0f});
        }
        // Normales invertidas para la convencion del shader de iluminacion.

        vertices.insert(vertices.end(), {signo * mesh->mNormals[i].x, signo * mesh->mNormals[i].y,
                                         signo * mesh->mNormals[i].z});
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        const aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    MeshModel* new_mesh = new MeshModel();
    new_mesh->CreateMeshModel(&vertices[0], &indices[0], static_cast<unsigned int>(vertices.size()),
                              static_cast<unsigned int>(indices.size()));
    mesh_list.push_back(new_mesh);
    mesh_to_texture.push_back(mesh->mMaterialIndex);
}

// Reutiliza la textura mientras algun modelo de la escena la conserve.
static std::shared_ptr<Texture> CargarTexturaCompartida(const std::string& ruta, bool alfa)
{
    static std::unordered_map<std::string,std::weak_ptr<Texture>> texturas;
    const std::string clave = std::filesystem::path(ruta).lexically_normal().generic_string();
    auto& entrada = texturas[clave];
    if (auto textura = entrada.lock()) return textura;
    auto textura = std::make_shared<Texture>(ruta.c_str());
    if (!(alfa ? textura->LoadTextureA() : textura->LoadTexture())) return {};
    entrada = textura;
    return textura;
}

void Model::LoadMaterials(const aiScene* scene)
{
    texture_list.resize(scene->mNumMaterials);
    texturas_noche.resize(scene->mNumMaterials, nullptr);
    tipos_emision.resize(scene->mNumMaterials, 0);
    intensidad_especular.resize(scene->mNumMaterials, 0.25f);
    brillo_material.resize(scene->mNumMaterials, 32.0f);
    separar_superficie.resize(scene->mNumMaterials, false);
    emision_material.resize(scene->mNumMaterials, 0.0f);
    opacidad_material.resize(scene->mNumMaterials, 1.0f);
    for (unsigned int i = 0; i < scene->mNumMaterials; i++)
    {
        aiMaterial* material = scene->mMaterials[i];
        material->Get(AI_MATKEY_OPACITY, opacidad_material[i]);
        aiString nombre_material;
        material->Get(AI_MATKEY_NAME, nombre_material);
        const std::string nombre(nombre_material.C_Str());
        if (nombre == "luminaria_nocturna" || nombre == "tira_nocturna" ||
            nombre == "separadores_carriles" || nombre.rfind("dron_luz_", 0) == 0)
        {
            tipos_emision[i] = 1;
            emision_material[i] = nombre == "separadores_carriles" ? 0.65f : 1.5f;
        }
        if (nombre == "pista_superficie" || nombre == "pista_salida" || nombre == "pista_meta" ||
            nombre == "metal_rojo" || nombre == "panel_solar")
            tipos_emision[i] = 2;
        if (std::string(nombre_material.C_Str()) == "cristal")
            emision_material[i] = 0.8f;
        if (std::string(nombre_material.C_Str()) == "borde_rojo")
            emision_material[i] = 1.6f;
        if (std::string(nombre_material.C_Str()) == "borde_blanco")
            emision_material[i] = 1.25f;
        // La emision del MTL conserva las luces propias de los modelos importados.
        aiColor3D emision(0.0f, 0.0f, 0.0f);
        material->Get(AI_MATKEY_COLOR_EMISSIVE, emision);
        emision_material[i] =
            std::max(emision_material[i], std::max(emision.r, std::max(emision.g, emision.b)));
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
                auto tex = std::make_shared<Texture>();
                if (tex->LoadMemory(reinterpret_cast<const unsigned char*>(embedded->pcData),
                                    static_cast<int>(embedded->mWidth)))
                {
                    texture_list[i] = tex;
                    continue;
                }
            }
        }
        if (material->GetTextureCount(aiTextureType_DIFFUSE))
        {
            aiString path;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
            {
                // Quitar del path todo lo que esté antes del último \ o / (Blender guarda rutas absolutas)
                std::string original(path.data);
                size_t idx = original.find_last_of("/\\");
                std::string filename = (idx == std::string::npos) ? original : original.substr(idx + 1);

                // Respetar la ruta del MTL permite compartir texturas entre escenas.
                // Como alternativa se busca por nombre junto al modelo y en su carpeta de texturas.
                std::string tex_path = carpeta_modelo + original;
                if (!ExisteArchivo(tex_path))
                    tex_path = carpeta_modelo + filename;
                if (!ExisteArchivo(tex_path))
                    tex_path = carpeta_modelo + "../texturas/" + filename;
                const size_t extension = tex_path.find_last_of('.');
                if (extension != std::string::npos)
                {
                    const std::string ruta_noche =
                        tex_path.substr(0,extension) + "_noche" + tex_path.substr(extension);
                    if (ExisteArchivo(ruta_noche))
                        texturas_noche[i] = CargarTexturaCompartida(ruta_noche,true);
                }
                const std::string ext = filename.substr(filename.find_last_of('.')+1);
                texture_list[i] = CargarTexturaCompartida(tex_path,ext == "tga" || ext == "png");
            }
        }
        // Si no se pudo cargar la textura o no tiene una textura asignada, asignar una textura por defecto
        if (!texture_list[i])
        {
            aiColor3D diffuse(1.0f, 1.0f, 1.0f);
            material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
            texture_list[i] = std::make_shared<Texture>();
            texture_list[i]->LoadColor(diffuse.r, diffuse.g, diffuse.b);
        }
    }
}
