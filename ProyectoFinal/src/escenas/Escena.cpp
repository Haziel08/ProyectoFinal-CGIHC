//>>>>>>>>>>>>>>>>>>>>>> Escena >>>>>>>>>>>>>>>>>>>>>>
#include "Escena.h"

#include <vector>
#include <algorithm>
#include "Model.h"
#include "Texture.h"
#include <gtc/type_ptr.hpp>

void Escena::CargarSkybox(const std::string& carpeta, const std::string& ext, const std::string& prefijo,
                            const std::string& cara_inferior)
{
    // Orden de las caras para la direccion de muestreo del shader.
    std::vector<std::string> caras = {
        carpeta + prefijo + "derecha" + ext, carpeta + prefijo + "izquierda" + ext,
        cara_inferior.empty() ? carpeta + prefijo + "abajo" + ext : cara_inferior,   carpeta + prefijo + "arriba" + ext,
        carpeta + prefijo + "atras" + ext,   carpeta + prefijo + "frente" + ext};
    skybox = Skybox(caras);
    tiene_skybox = true;
}

void Escena::DibujarSkybox(const glm::mat4& vista, const glm::mat4& proyeccion)
{
    if (tiene_skybox)
        skybox.DrawSkybox(vista, proyeccion);
}

//>>>>>>>>>>>>>>>>>>>>>> Brillos pequenos unidos al objeto padre >>>>>>
void Escena::DibujarBrillos(const ContextoRender& ctx, const Model& modelo, const glm::mat4& padre,
                            Texture& textura, float intensidad)
{
    if (modelo.ObtenerEmisores().empty() || intensidad <= 0)
        return;
    if (!malla_brillos)
    {
        GLfloat vertices[] = {-.5f, -.5f, 0, 0, 0, 0, 0, 1, .5f,  -.5f, 0, 1, 0, 0, 0, 1,
                              .5f,  .5f,  0, 1, 1, 0, 0, 1, -.5f, .5f,  0, 0, 1, 0, 0, 1};
        unsigned int indices[] = {0, 1, 2, 0, 2, 3};
        malla_brillos = new MeshModel();
        malla_brillos->CreateMeshModel(vertices, indices, 32, 6);
    }
    GLint programa = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
    const GLint modo = glGetUniformLocation(programa, "modo_efecto");
    const GLint opacidad = glGetUniformLocation(programa, "opacidad_particula");
    glUniform1i(modo, 4);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    textura.UseTexture();
    const float escala = std::max({glm::length(glm::vec3(padre[0])), glm::length(glm::vec3(padre[1])),
                                   glm::length(glm::vec3(padre[2]))});
    for (const auto& led : modelo.ObtenerEmisores())
    {
        const glm::vec3 centro = glm::vec3(padre * glm::vec4(led.posicion, 1));
        const float radio = std::max(glm::clamp(led.radio * escala, .12f, 2.8f),
                                     glm::length(centro - ctx.pos_camara) * .0035f);
        // Heredar posicion y escala del objeto, pero orientar el halo hacia la camara.
        glm::mat4 matriz = glm::transpose(glm::mat4(glm::mat3(ctx.vista)));
        matriz[3] = glm::vec4(centro, 1);
        matriz = glm::scale(matriz, glm::vec3(radio * 2));
        glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(matriz));
        glUniform3fv(ctx.u_color, 1, glm::value_ptr(led.color));
        const bool rojo = led.color.r > led.color.g * 1.5f && led.color.r > led.color.b * 1.5f;
        glUniform1f(opacidad, intensidad * (rojo ? .6f : 1.0f));
        malla_brillos->RenderMeshModel();
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glUniform1i(modo, 0);
    glUniform1f(opacidad, 1);
    glUniform3f(ctx.u_color, 1, 1, 1);
}
