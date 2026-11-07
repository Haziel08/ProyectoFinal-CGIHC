#include "EscenaPlanetaAzul.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> EscenaPlanetaAzul >>>>>>>>>>>>>>>>>>>>>>

void EscenaPlanetaAzul::Inicializar()
{
	//Skybox de dia y de noche
	// Se cargan sin voltear
	stbi_set_flip_vertically_on_load(false);
	CargarSkybox("assets/planeta_azul/texturas/skybox/noche/");
	skybox_noche = skybox;
	CargarSkybox("assets/planeta_azul/texturas/skybox/dia/");
	skybox_dia = skybox;

	//Camara: empieza en la orilla mirando hacia +Z
	camara = Camera(glm::vec3(0.0f, 10.0f, -60.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, -8.0f, 20.0f, 0.3f);

	//Suelo y lago
	terreno.Crear();
	glm::vec2 tamano_lago = terreno.radio_lago * 2.2f;   // un poco mas grande que el lago
	agua.Crear(terreno.centro_lago, tamano_lago, nivel_agua, "assets/planeta_azul/texturas/agua.jpg");

	vegetacion.Crear(terreno);
	aldea.Crear(terreno);
    CargarLamparas();
	peces.Crear();
	necromorfos.Crear(terreno, aldea);
	stalker.Crear();
	pidgeys.Crear();
	reaper.Crear();

	luz_sol = ciclo.LuzDireccional();

}

void EscenaPlanetaAzul::Actualizar(float dt, Window& ventana)
{
	tiempo += dt;

	// Ciclo de dia y noche actualiza la luz y escoge el skybox
	ciclo.Actualizar(dt);
	luz_sol = ciclo.LuzDireccional();
	skybox = ciclo.EsDeNoche() ? skybox_noche : skybox_dia;

	// Aldea: tecla F para la fogata y antorchas que se prenden de noche
	aldea.Actualizar(tiempo, ciclo.EsDeNoche(), ventana);

	// Peces nadando
	peces.Actualizar(dt);

	// Necromorfos caminan, y bailan mientras la fogata este prendida
	necromorfos.Actualizar(dt, aldea.fogata_encendida);

	// Avatar nadando, aves volando, y el Reaper (tecla G: salta por un Pidgey)
	stalker.Actualizar(dt);
	pidgeys.Actualizar(dt);
	reaper.Actualizar(dt, ventana, pidgeys);

}

void EscenaPlanetaAzul::Dibujar(const ContextoRender& ctx)
{
	// 1) Todo lo opaco primero
	terreno.Dibujar(ctx);
	aldea.Dibujar(ctx);
    DibujarLamparas(ctx);
	peces.Dibujar(ctx);          // bajo el agua: se dibujan antes que el agua
	stalker.Dibujar(ctx);
	reaper.Dibujar(ctx);
	pidgeys.Dibujar(ctx);


	// 2) Plantas y rocas
	vegetacion.Dibujar(ctx, &luz_sol, aldea.luces, aldea.num_luces);
	necromorfos.Dibujar(ctx, &luz_sol, aldea.luces, aldea.num_luces);   // shader del esqueleto

	// 3) El agua es transparente
	agua.Dibujar(ctx, tiempo, ciclo.Luz());

	// 4) El fuego al final de todo
	aldea.DibujarLlamas(ctx, tiempo);
}

bool EscenaPlanetaAzul::ObjetivoCamara(int modo, glm::vec3& objetivo, glm::vec3& rumbo,
                                 float& distancia, float& altura) const
{
    if (modo == 3)
    {
        objetivo = stalker.posicion;
        rumbo = glm::vec3(sinf(stalker.giro), 0, cosf(stalker.giro));
        distancia = 8.0f;
        altura = -2.0f;
        return true;
    }
    if (modo != 4) return false;
    objetivo = necromorfos.PosicionPatrulla() + glm::vec3(0,1,0);
    rumbo = necromorfos.RumboPatrulla();
    distancia = 8.0f;
    altura = 4.5f;
    return true;
}

//>>>>>>>>>>>>>>>>>>>>>> Luminarias de suelo y tejados >>>>>>>>>>>>>>>>>>>>>>
namespace
{
const glm::vec2 lampara_entrada(-65,-40);
const glm::vec2 lampara_sendero(-51,-31);
const glm::vec2 lampara_orilla(-27,4);
const glm::vec2 lampara_puente(19,6);
const glm::vec2 lampara_pinos(87,66);
const glm::vec2 lampara_playa(27,132);
const glm::vec2 luces_sendero[] = {lampara_entrada,lampara_sendero,lampara_orilla,
                                  lampara_puente,lampara_pinos,lampara_playa};
}

void EscenaPlanetaAzul::CargarLamparas()
{
    lampara_suelo.LoadModel("assets/planeta_azul/modelos/lampara_suelo.obj");
    halo_suelo = Texture("assets/planeta_naranja/texturas/halo_faro.png");
    halo_suelo.LoadTextureA();
    std::vector<GLfloat> vertices;
    std::vector<unsigned> indices;
    for (const auto& p : luces_sendero)
    {
        const unsigned base = unsigned(vertices.size()/8);
        for (int z=0;z<=8;++z)
            for (int x=0;x<=8;++x)
            {
                const float u=x/8.0f,v=z/8.0f;
                const float px=p.x+(u-.5f)*16.0f,pz=p.y+(v-.5f)*16.0f;
                vertices.insert(vertices.end(),{px,terreno.Altura(px,pz)+.035f,pz,u,v,0,-1,0});
                if (x<8 && z<8)
                {
                    const unsigned j=base+z*9+x;
                    indices.insert(indices.end(),{j,j+1,j+9,j+9,j+1,j+10});
                }
            }
    }
    malla_halo_suelo.CreateMeshModel(vertices.data(),indices.data(),unsigned(vertices.size()),unsigned(indices.size()));
    GLfloat vertices_tejado[] = {-.5f,-.5f,0,0,0,0,0,-1, .5f,-.5f,0,1,0,0,0,-1,
                                 .5f,.5f,0,1,1,0,0,-1, -.5f,.5f,0,0,1,0,0,-1};
    unsigned indices_tejado[] = {0,1,2,0,2,3};
    malla_halo_tejado.CreateMeshModel(vertices_tejado,indices_tejado,32,6);
}

void EscenaPlanetaAzul::DibujarLamparas(const ContextoRender& ctx)
{
    GLint programa = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM,&programa);
    const GLint modo = glGetUniformLocation(programa,"modo_efecto");
    const GLint opacidad = glGetUniformLocation(programa,"opacidad_particula");
    const GLint noche = glGetUniformLocation(programa,"factor_noche");
    auto enviar = [&](const glm::mat4& m) {
        glUniformMatrix4fv(ctx.u_model,1,GL_FALSE,glm::value_ptr(m));
    };
    glUniform1f(noche,ciclo.EsDeNoche() ? 1.0f : 0.0f);
    for (const auto& p : luces_sendero)
    {
        enviar(glm::translate(glm::mat4(1),glm::vec3(p.x,terreno.Altura(p.x,p.y),p.y)));
        lampara_suelo.RenderModel(true,ctx.u_specular_intensity,ctx.u_shininess);
    }
    if (!ciclo.EsDeNoche()) return;
    glUniform1i(modo,4);
    glUniform1f(opacidad,.62f);
    glUniform3f(ctx.u_color,1.2f,1.03f,.72f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glDepthMask(GL_FALSE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-2,-4);
    halo_suelo.UseTexture();
    enviar(glm::mat4(1));
    malla_halo_suelo.RenderMeshModel();
    glDisable(GL_POLYGON_OFFSET_FILL);
    const float tejados[] = {8.4f,8.05f,8.4f,8.33f,8.4f,8.05f,8.4f};
    for (int i=0;i<aldea.NumChozas();++i)
    {
        const glm::vec2 p = aldea.PosicionChoza(i);
        glm::mat4 m = glm::transpose(glm::mat4(glm::mat3(ctx.vista)));
        m[3] = glm::vec4(p.x,terreno.Altura(p.x,p.y)+tejados[i],p.y,1);
        enviar(glm::scale(m,glm::vec3(2.8f)));
        malla_halo_tejado.RenderMeshModel();
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glUniform1i(modo,0);
    glUniform1f(opacidad,1);
    glUniform1f(noche,0);
    glUniform3f(ctx.u_color,1,1,1);
}
}
