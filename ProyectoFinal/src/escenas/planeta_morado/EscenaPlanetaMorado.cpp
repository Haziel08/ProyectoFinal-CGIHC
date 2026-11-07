//>>>>>>>>>>>>>>>>>>>>>> Recursos y geometria del planeta morado >>>>>>>>>>>>>>>>>>>>>>
#include "EscenaPlanetaMorado.h"
#include "planeta_naranja/RecorridoNaranja.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <random>
#include <fstream>
#include <sstream>

namespace
{
const glm::vec3 origen_cohete(0, .8f, -6);
const float altura_cubierta = 6.46f;
const float gravedad = 9.8f;
const float duracion_despegue = 12.0f;
const glm::vec3 offsets_motores[] = {{0,22.629f,5.781f},{-5.035f,15.256f,5.379f},{5.442f,15.256f,5.491f}};
const float mitad_motores[] = {17.371f,15.256f,15.256f};

glm::mat4 Colocar(glm::vec3 posicion, float giro, glm::vec3 escala)
{
    return glm::scale(glm::rotate(glm::translate(glm::mat4(1), posicion), giro, glm::vec3(0, 1, 0)), escala);
}

glm::vec3 RutaSaru(float tiempo)
{
    const float a = tiempo * .032f;
    return {455*std::cos(a),0,455*std::sin(a)};
}

void Vertice(std::vector<GLfloat>& datos, glm::vec3 p, glm::vec2 uv, glm::vec3 normal)
{
    datos.insert(datos.end(), {p.x,p.y,p.z,uv.x,uv.y,normal.x,normal.y,normal.z});
}

void CrearDisco(MeshModel& malla, float radio, float altura, float grosor, float repeticion)
{
    std::vector<GLfloat> vertices;
    std::vector<unsigned> indices;
    constexpr int lados = 96;
    Vertice(vertices, {0,altura,0}, {0,0}, {0,-1,0});
    for (int i = 0; i <= lados; ++i)
    {
        const float a = glm::two_pi<float>() * i / lados;
        const glm::vec3 p(radio * std::cos(a), altura, radio * std::sin(a));
        Vertice(vertices, p, {p.x/repeticion,p.z/repeticion}, {0,-1,0});
        if (i != lados) indices.insert(indices.end(), {0u,unsigned(i+1),unsigned(i+2)});
    }
    if (grosor > 0)
    {
        const unsigned base = unsigned(vertices.size() / 8);
        for (int i = 0; i <= lados; ++i)
        {
            const float a = glm::two_pi<float>() * i / lados;
            const glm::vec3 radial(std::cos(a),0,std::sin(a));
            const glm::vec3 borde = radial * radio + glm::vec3(0,altura,0);
            Vertice(vertices, borde, {i/8.0f,0}, -radial);
            Vertice(vertices, borde-glm::vec3(0,grosor,0), {i/8.0f,1}, -radial);
            if (i != lados)
            {
                const unsigned j = base + 2*i;
                indices.insert(indices.end(), {j,j+1,j+2,j+2,j+1,j+3});
            }
        }
    }
    malla.CreateMeshModel(vertices.data(), indices.data(), unsigned(vertices.size()), unsigned(indices.size()));
}

void CrearCaminos(MeshModel& malla)
{
    const glm::vec2 calles[][4] = {
        {{-29,102},{-25,183},{-12,230},{0,272}},
        {{0,272},{-48,286},{-102,298},{-148,322}},
        {{0,272},{49,285},{101,298},{149,321}},
        {{33,101},{97,111},{166,70},{184,27}},
        {{184,-29},{201,-95},{245,-145},{319,-205}}};
    std::vector<GLfloat> vertices;
    std::vector<unsigned> indices;
    for (const auto& calle : calles)
    {
        const unsigned base = unsigned(vertices.size()/8);
        for (int i = 0; i <= 40; ++i)
        {
            const float t = i / 40.0f, s = 1.0f-t;
            const glm::vec2 p = s*s*s*calle[0]+3.0f*s*s*t*calle[1]+3.0f*s*t*t*calle[2]+t*t*t*calle[3];
            const glm::vec2 d = glm::normalize(s*s*(calle[1]-calle[0])+2.0f*s*t*(calle[2]-calle[1])+t*t*(calle[3]-calle[2]));
            const glm::vec2 borde(-d.y,d.x);
            const float ancho = 7.0f + std::sin(i * .45f);
            const float v = t * glm::distance(calle[0],calle[3]) / 14.0f;
            Vertice(vertices,{p.x-borde.x*ancho,.045f,p.y-borde.y*ancho},{0,v},{0,-1,0});
            Vertice(vertices,{p.x+borde.x*ancho,.045f,p.y+borde.y*ancho},{1,v},{0,-1,0});
            if (i != 40)
            {
                const unsigned j = base + 2*i;
                indices.insert(indices.end(),{j,j+1,j+2,j+2,j+1,j+3});
            }
        }
    }
    malla.CreateMeshModel(vertices.data(),indices.data(),unsigned(vertices.size()),unsigned(indices.size()));
}

//>>>>>>>>>>>>>>>>>>>>>> Lanzamiento, desprendimiento y orbita >>>>>>>>>>>>>>>>>>>>>>
float Ascenso(float t)
{
    return 60.0f * (1.0f - std::cos(glm::pi<float>() * glm::clamp(t/duracion_despegue,0.0f,1.0f)));
}

glm::mat4 MatrizMotor(int indice, float t)
{
    const float separacion = indice == 0 ? duracion_despegue : 8.0f;
    const float caida = std::max(0.0f,t-separacion);
    const glm::vec3 local = offsets_motores[indice];
    glm::vec3 punto = origen_cohete + glm::vec3(local.z,altura_cubierta+local.y,-local.x);
    punto.y = std::max(.8f+mitad_motores[indice],
                       punto.y+Ascenso(std::min(t,separacion))-.5f*gravedad*caida*caida);
    return Colocar(punto,glm::half_pi<float>(),glm::vec3(1));
}

glm::mat4 MatrizCohete(float t)
{
    const glm::vec3 inicio = origen_cohete + glm::vec3(-3.626f,altura_cubierta+14.849f+Ascenso(t),0);
    if (t <= duracion_despegue)
        return Colocar(inicio,glm::half_pi<float>(),glm::vec3(1));

    const float vuelo = t-duracion_despegue;
    const float radio = 418.0f;
    const float velocidad = 63.0f;
    const glm::vec3 centro(57.0f/1.18f,inicio.y,29.0f/1.18f);
    const glm::vec3 entrada = centro+glm::vec3(0,0,-radio);
    if (vuelo < 11.0f)
    {
        const float u = vuelo/11.0f;
        const auto curva = [&](float f) {
            return RecorridoEspiral::Hermite(inicio,entrada,{280,0,0},{velocidad*11.0f,0,0},f);
        };
        const glm::vec3 avance = curva(std::min(1.0f,u+.002f))-curva(std::max(0.0f,u-.002f));
        return glm::rotate(Colocar(curva(u),std::atan2(avance.x,avance.z),glm::vec3(1)),
            glm::half_pi<float>()*RecorridoEspiral::Suavizar(u*4.0f),glm::vec3(1,0,0));
    }
    const float fase = std::fmod((vuelo-11.0f)*velocidad/radio,glm::two_pi<float>());
    return glm::rotate(Colocar(centro+glm::vec3(radio*std::sin(fase),0,-radio*std::cos(fase)),
        glm::half_pi<float>()-fase,glm::vec3(1)),glm::half_pi<float>(),glm::vec3(1,0,0));
}

glm::mat4 MatrizNeptuno(float tiempo_vuelo)
{
    glm::vec3 posicion_neptuno(-90,13.4f+Ascenso(tiempo_vuelo),55);
    float rumbo_neptuno = 0;
    float inclinacion_neptuno = 0;
    if (tiempo_vuelo > duracion_despegue)
    {
        const float vuelo = tiempo_vuelo-duracion_despegue;
        const float a = glm::pi<float>()+std::max(0.0f,vuelo-10)*.134f;
        const glm::vec3 orbita(470*std::cos(a),posicion_neptuno.y,470*std::sin(a));
        const float u = glm::clamp(vuelo/10.0f,0.0f,1.0f);
        posicion_neptuno = RecorridoEspiral::Hermite(posicion_neptuno,orbita,
            {-390,0,0}, {0,0,-630},u);
        const glm::vec3 tangente = vuelo < 10 ?
            RecorridoEspiral::Hermite(glm::vec3(-90,posicion_neptuno.y,55),orbita,
                {-390,0,0},{0,0,-630},std::min(1.0f,u+.003f))-posicion_neptuno :
            glm::vec3(-std::sin(a),0,std::cos(a));
        rumbo_neptuno = std::atan2(tangente.x,tangente.z);
        inclinacion_neptuno = glm::half_pi<float>()*RecorridoEspiral::Suavizar(vuelo/2.0f);
    }
    return glm::rotate(Colocar(posicion_neptuno,rumbo_neptuno,glm::vec3(1.2f)),
        inclinacion_neptuno,glm::vec3(1,0,0));
}

glm::mat4 RaizCoheteBlanco()
{
    return Colocar({-57,5.2f,-29},0,glm::vec3(1.18f));
}
}

//>>>>>>>>>>>>>>>>>>>>>> Carga de recursos y distribucion >>>>>>>>>>>>>>>>>>>>>>
void EscenaPlanetaMorado::Inicializar()
{
    stbi_set_flip_vertically_on_load(false);
    CargarSkybox("assets/planeta_morado/texturas/skybox/noche/");
    cielo_noche = skybox;
    CargarSkybox("assets/planeta_morado/texturas/skybox/dia/");
    cielo_dia = skybox;
    camara = Camera(glm::vec3(45,44,-112),glm::vec3(0,1,0),107,-12,65,.3f);

    const std::string ruta = "assets/planeta_morado/modelos/";
    refugios[0].LoadModel(ruta+"refugio.obj",false,false);
    refugios[1].LoadModel(ruta+"modulo.obj",false,false);
    flora[0].LoadModel(ruta+"arbol_luminoso.obj",false,false);
    flora[1].LoadModel(ruta+"coral_morado.obj",false,false);
    flora[2].LoadModel(ruta+"hongos_acidos.obj",false,false);
    sonda_cuerpo.LoadModel(ruta+"sonda_cuerpo.obj",false,false);
    sonda_rotor.LoadModel(ruta+"sonda_rotor.obj",false,false);
    cohete.LoadModel(ruta+"p10_nave.obj");
    plataforma.LoadModel(ruta+"p10_plataforma.obj");
    antena.LoadModel("assets/planeta_naranja/modelos/radiotelescopio.obj",false,false);
    CargarSegundoCohete(ruta);
    auxiliar.Cargar();
    flora[3].LoadModel("assets/planeta_azul/modelos/roca_mediana_1.obj");
    flora[4].LoadModel("assets/planeta_azul/modelos/roca_mediana_2.obj");
    saru_cargado = saru.Cargar(ruta+"saru.fbx","assets/planeta_morado/texturas/saru_color.jpg",true);
    if (saru_cargado) saru.QuitarAvance(0,"mixamorig:Hips");
    if (saru_cargado)
        shader_avatar.CreateFromFiles("shaders/shader_esqueleto.vert", "shaders/shader_light.frag");
    const char* piezas[] = {"central","izquierdo","derecho"};
    const char* partes[] = {"cohete","torre","plataforma"};
    for (int i=0;i<3;++i)
    {
        motores[i].LoadModel(ruta+"p10_motor_"+piezas[i]+".obj");
        neptuno[i].LoadModel(ruta+"neptuno_"+partes[i]+".obj");
        montanas[i].LoadModel("assets/planeta_naranja/modelos/montana_0"+std::to_string(i+1)+".obj",false,false);
        meteoritos[i].LoadModel("assets/planeta_naranja/modelos/asteroide_0"+std::to_string(i+1)+".obj",false,false);
    }
    textura_suelo = Texture("assets/planeta_morado/texturas/suelo_morado.jpg");
    textura_suelo.LoadTexture();
    textura_suelo.UseTexture();
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    textura_camino = Texture("assets/planeta_naranja/texturas/camino_terraceria.png");
    textura_camino.LoadTextureA();
    textura_estela = Texture("assets/planeta_naranja/texturas/fuego_asteroides.png");
    textura_estela.LoadTextureA();
    textura_humo = Texture("assets/planeta_naranja/texturas/humo.png");
    textura_humo.LoadTextureA();
    halo_arbol = Texture("assets/planeta_naranja/texturas/halo_faro.png");
    halo_arbol.LoadTextureA();
    CrearDisco(suelo,865,0,0,26);
    CrearDisco(base_inferior,186,2,2,19);
    CrearDisco(base_media,171,4,2,19);
    CrearDisco(base_lanzamiento,155,6,2,19);
    CrearCaminos(caminos);
    GLfloat plano[] = {-.5f,-.5f,0,0,0,0,0,-1, .5f,-.5f,0,1,0,0,0,-1,
                        .5f,.5f,0,1,1,0,0,-1, -.5f,.5f,0,0,1,0,0,-1};
    unsigned indices[] = {0,1,2,0,2,3};
    plano_estela.CreateMeshModel(plano,indices,32,6);
    GLfloat estela[] = {0,-.5f,0,0,0,0,0,1, 0,.5f,0,1,0,0,0,1,
                       -1,.5f,0,1,1,0,0,1, -1,-.5f,0,0,1,0,0,1,
                        0,0,-.5f,0,0,0,1,0, 0,0,.5f,1,0,0,1,0,
                       -1,0,.5f,1,1,0,1,0, -1,0,-.5f,0,1,0,1,0};
    unsigned caras_estela[] = {0,1,2,0,2,3,4,5,6,4,6,7};
    malla_estela.CreateMeshModel(estela,caras_estela,64,12);

    poblado = {
        {0,Colocar({-58,0,208},0.000f,glm::vec3(30,35,30))}, // Refugio 1
        {0,Colocar({9,0,208},0.530f,glm::vec3(30,35,30))}, // Refugio 2
        {0,Colocar({-48,0,247},1.060f,glm::vec3(30,35,30))}, // Refugio 3
        {0,Colocar({24,0,252},1.590f,glm::vec3(30,35,30))}, // Refugio 4
        {0,Colocar({-42,0,277},2.120f,glm::vec3(30,35,30))}, // Refugio 5
        {0,Colocar({46,0,274},2.650f,glm::vec3(30,35,30))}, // Refugio 6
        {0,Colocar({-65,0,318},3.180f,glm::vec3(30,35,30))}, // Refugio 7
        {0,Colocar({-115,0,266},3.710f,glm::vec3(30,35,30))}, // Refugio 8
        {0,Colocar({-118,0,348},4.240f,glm::vec3(30,35,30))}, // Refugio 9
        {0,Colocar({65,0,324},4.770f,glm::vec3(30,35,30))}, // Refugio 10
        {0,Colocar({107,0,271},5.300f,glm::vec3(30,35,30))}, // Refugio 11
        {0,Colocar({125,0,350},5.830f,glm::vec3(30,35,30))}, // Refugio 12
        {0,Colocar({-282,0,96},6.360f,glm::vec3(30,35,30))}, // Refugio 13
        {0,Colocar({-315,0,143},6.890f,glm::vec3(30,35,30))}, // Refugio 14
        {0,Colocar({-353,0,93},7.420f,glm::vec3(30,35,30))}, // Refugio 15
        {1,Colocar({287,0,-233},7.950f,glm::vec3(42,49,42))}, // Modulo 1
        {1,Colocar({333,0,-175},8.480f,glm::vec3(42,49,42))}, // Modulo 2
        {1,Colocar({367,0,-243},9.010f,glm::vec3(42,49,42))}, // Modulo 3
        {1,Colocar({418,0,-200},9.540f,glm::vec3(42,49,42))}, // Modulo 4
        {1,Colocar({399,0,-292},10.070f,glm::vec3(42,49,42))}, // Modulo 5
    };

    std::mt19937 azar(924);
    std::uniform_real_distribution<float> unidad(0,1);
    for (int i=0;i<22;++i)
    {
        const float a = glm::two_pi<float>()*i/22.0f;
        sierra.push_back({(i*2+1)%3,Colocar({752*std::cos(a),-12,752*std::sin(a)},
            -a+.22f*(unidad(azar)-.5f),{370+90*unidad(azar),58+45*unidad(azar),375+90*unidad(azar)})});
    }
    const glm::vec2 grupos[] = {{-340,-270},{-453,275},{-96,-430},{375,230},{550,-105},{265,-470},{91,488},{-545,-90}};
    for (const auto& grupo : grupos)
        for (int i=0;i<26;++i)
        {
            const float a = unidad(azar)*glm::two_pi<float>();
            const float r = 22+105*unidad(azar);
            const glm::vec2 p = grupo + r*glm::vec2(std::cos(a),std::sin(a));
            if (glm::length(p)<205 || glm::length(p)>670 || (p.x>0 && p.x<380 && std::abs(p.y)<51)) continue;
            const int tipo = i%3;
            const float escala = tipo == 0 ? 105.0f : 17.0f+16*unidad(azar);
            vegetacion.push_back({tipo,Colocar({p.x,0,p.y},a,glm::vec3(escala))});
        }
    for (int i = 0; i < 39; ++i)
    {
        const float a = glm::two_pi<float>()*(i+.35f*unidad(azar))/39.0f;
        const float radio = 480+45*unidad(azar);
        vegetacion.push_back({0,Colocar({radio*std::cos(a),0,radio*std::sin(a)},a,
                                       glm::vec3(105))});
    }
    for (int i = 0; i < 45; ++i)
    {
        const float a = glm::two_pi<float>()*unidad(azar);
        const float radio = 220+335*unidad(azar);
        const glm::vec3 punto(radio*std::cos(a),0,radio*std::sin(a));
        bool libre = true;
        for (const auto& casa : poblado)
            if (glm::distance(punto,glm::vec3(casa.matriz[3])) < 35) libre = false;
        if (libre) vegetacion.push_back({3+i%2,Colocar(punto,a,glm::vec3(2+4*unidad(azar)))});
    }
    // Los halos fijos de los arboles comparten una sola malla.
    std::vector<GLfloat> vertices_halo;
    std::vector<unsigned> indices_halo;
    for (const auto& objeto : vegetacion)
    {
        if (objeto.modelo != 0) continue;
        const glm::vec3 punto(objeto.matriz[3].x,.055f,objeto.matriz[3].z);
        const unsigned base = unsigned(vertices_halo.size()/8);
        Vertice(vertices_halo,punto+glm::vec3(-23,0,-23),{0,0},{0,-1,0});
        Vertice(vertices_halo,punto+glm::vec3(23,0,-23),{1,0},{0,-1,0});
        Vertice(vertices_halo,punto+glm::vec3(23,0,23),{1,1},{0,-1,0});
        Vertice(vertices_halo,punto+glm::vec3(-23,0,23),{0,1},{0,-1,0});
        indices_halo.insert(indices_halo.end(),{base,base+1,base+2,base,base+2,base+3});
    }
    plano_halo.CreateMeshModel(vertices_halo.data(),indices_halo.data(),
        unsigned(vertices_halo.size()),unsigned(indices_halo.size()));
    luz = DirectionalLight(1,.86f,.76f,.42f,.72f,0,-1,-.4f);
    matriz_cohete = RaizCoheteBlanco()*MatrizCohete(0);
    for (int i=0;i<3;++i) matrices_motores[i] = RaizCoheteBlanco()*MatrizMotor(i,0);
}

//>>>>>>>>>>>>>>>>>>>>>> Animacion y ciclo de luz >>>>>>>>>>>>>>>>>>>>>>
void EscenaPlanetaMorado::Actualizar(float dt, Window& ventana)
{
    tiempo = std::fmod(tiempo+dt,1800.0f);
    tiempo_npc += dt;
    const bool reiniciar = ventana.TeclaPresionada(GLFW_KEY_P);
    if (reiniciar)
    {
        lanzamiento = true;
        tiempo_cohete = 0.0f;
        segundo_activo = true;
        cuadro_segundo = 0;
        tiempo_segundo = 0.0f;
    }
    if (lanzamiento) tiempo_cohete += dt;
    auxiliar.Actualizar(dt,reiniciar);
    ActualizarSegundoCohete(dt);
    matriz_cohete = RaizCoheteBlanco()*MatrizCohete(tiempo_cohete);
    for (int i=0;i<3;++i) matrices_motores[i] = RaizCoheteBlanco()*MatrizMotor(i,tiempo_cohete);
    const float a = glm::two_pi<float>()*tiempo/45.0f;
    matriz_sonda = Colocar({590*std::cos(a),110,590*std::sin(a)},-a,glm::vec3(68));
    const float dia = .5f+.5f*std::cos(glm::two_pi<float>()*tiempo/120.0f);
    factor_noche = 1.0f-dia;
    skybox = dia < .5f ? cielo_noche : cielo_dia;
    const glm::vec3 color = glm::mix(glm::vec3(.55f,.42f,.85f),glm::vec3(1,.86f,.76f),dia);
    luz = DirectionalLight(color.r,color.g,color.b,.28f+.14f*dia,.18f+.54f*dia,0,-1,-.4f);
}

//>>>>>>>>>>>>>>>>>>>>>> Dibujo >>>>>>>>>>>>>>>>>>>>>>
void EscenaPlanetaMorado::Dibujar(const ContextoRender& ctx)
{
    GLint programa = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM,&programa);
    const GLint tinte = glGetUniformLocation(programa,"recolorar_superficie");
    const GLint efecto = glGetUniformLocation(programa,"modo_efecto");
    const GLint opacidad = glGetUniformLocation(programa,"opacidad_particula");
    const GLint offset = glGetUniformLocation(programa,"desplazamiento_textura");
    glUniform1f(glGetUniformLocation(programa,"factor_noche"),factor_noche);
    auto enviar = [&](const glm::mat4& m) { glUniformMatrix4fv(ctx.u_model,1,GL_FALSE,glm::value_ptr(m)); };
    auto dibujar = [&](Model& modelo,const glm::mat4& m) {
        enviar(m); modelo.RenderModel(true,ctx.u_specular_intensity,ctx.u_shininess);
    };
    glUniform3f(ctx.u_color,1,1,1);
    glUniform1f(ctx.u_specular_intensity,.08f);
    glUniform1f(ctx.u_shininess,8);
    enviar(glm::mat4(1));
    textura_suelo.UseTexture();
    suelo.RenderMeshModel();
    glUniform3f(ctx.u_color,.62f,.48f,.75f);
    base_inferior.RenderMeshModel();
    base_media.RenderMeshModel();
    base_lanzamiento.RenderMeshModel();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glUniform1i(tinte,1);
    glUniform3f(ctx.u_color,.54f,.30f,.68f);
    textura_camino.UseTexture();
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-2.0f,-8.0f);
    caminos.RenderMeshModel();
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_BLEND);
    for (const auto& objeto : sierra)
    {
        const float tono = 1.0f + objeto.modelo * .12f;
        glUniform3f(ctx.u_color,.65f*tono,.40f*tono,.82f*tono);
        dibujar(montanas[objeto.modelo],objeto.matriz);
    }
    glUniform1i(tinte,0);
    glUniform3f(ctx.u_color,1,1,1);
    for (const auto& objeto : poblado) dibujar(refugios[objeto.modelo],objeto.matriz);
    const GLint frutos = glGetUniformLocation(programa,"brillo_frutos");
    for (const auto& objeto : vegetacion)
    {
        glUniform1f(frutos,objeto.modelo == 0 ? .55f+3.4f*factor_noche : 0);
        dibujar(flora[objeto.modelo],objeto.matriz);
    }
    glUniform1f(frutos,0);
    glUniform1i(efecto,4);
    glUniform1f(opacidad,.12f+.52f*factor_noche);
    glUniform3f(ctx.u_color,.28f,.62f,1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glDepthMask(GL_FALSE);
    halo_arbol.UseTexture();
    enviar(glm::mat4(1));
    plano_halo.RenderMeshModel();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glUniform1i(efecto,0);
    glUniform1f(opacidad,1);
    glUniform3f(ctx.u_color,1,1,1);
    dibujar(antena,Colocar({-390,0,-147},.38f,glm::vec3(58)));
    dibujar(antena,Colocar({385,0,365},-1.2f,glm::vec3(54)));
    dibujar(plataforma,RaizCoheteBlanco()*Colocar({0,.8f,0},glm::half_pi<float>(),glm::vec3(1)));
    dibujar(plataforma,Colocar({65,6,-35},glm::half_pi<float>(),glm::vec3(1)));
    auxiliar.Dibujar(ctx,{24,6,91},0);
    dibujar(cohete,matriz_cohete);
    if (!cuadros_cohete.empty())
        for (size_t i = 0; i < segundo_cohete.size(); ++i)
            dibujar(segundo_cohete[i],matrices_segundo[i]);
    if (saru_cargado) DibujarAvatar(ctx);
    for (int i=0;i<3;++i) dibujar(motores[i],matrices_motores[i]);
    dibujar(neptuno[2],Colocar({-90,6,55},0,glm::vec3(1.2f)));
    dibujar(neptuno[0],MatrizNeptuno(tiempo_cohete));
    dibujar(neptuno[1],Colocar({-105,13.4f,55},0,glm::vec3(1.2f)));
    dibujar(sonda_cuerpo,matriz_sonda);
    const glm::mat4 pivote = glm::translate(matriz_sonda,glm::vec3(0,0,.079f));
    dibujar(sonda_rotor,glm::translate(glm::rotate(pivote,glm::two_pi<float>()*tiempo/5.0f,glm::vec3(0,0,1)),glm::vec3(0,0,-.079f)));

    glm::mat4 marcos[3];
    for (int i=0;i<3;++i)
    {
        const float fase = glm::two_pi<float>()*(tiempo/RecorridoAereo::periodo_asteroides+i/3.0f);
        const glm::vec3 frente = glm::normalize(RecorridoAereo::TangenteElipse(fase));
        const glm::vec3 lado = glm::normalize(glm::cross(frente,glm::vec3(0,1,0)));
        marcos[i] = glm::mat4(glm::vec4(frente,0),glm::vec4(glm::cross(lado,frente),0),
                            glm::vec4(lado,0),glm::vec4(RecorridoAereo::Elipse(fase),1));
        const glm::mat4 giro = glm::rotate(marcos[i],glm::two_pi<float>()*tiempo/28.0f+i,
                                        glm::normalize(glm::vec3(.4f,1,.25f)));
        dibujar(meteoritos[i],glm::scale(giro,glm::vec3(64)));
    }
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glUniform1i(efecto,1);
    textura_estela.UseTexture();
    if (segundo_activo && !cuadros_cohete.empty() && cuadros_cohete[cuadro_segundo].motor)
    {
        glUniform1f(opacidad,1);
        glUniform2f(offset,0,std::fmod(-tiempo,1.0f));
        for (size_t i = 0; i < 4; ++i)
        {
            if (i && !cuadros_cohete[cuadro_segundo].piezas[i].acoplada) continue;
            glm::mat4 llama = glm::translate(matrices_segundo[i],glm::vec3(0,0,i ? -1.19f : -2.2f));
            llama = glm::rotate(llama,-glm::half_pi<float>(),glm::vec3(0,1,0));
            enviar(glm::scale(llama,glm::vec3(3.2f,.85f,.85f)));
            malla_estela.RenderMeshModel();
        }
    }
    for (int i=0;i<3;++i)
    {
        enviar(glm::scale(glm::translate(marcos[i],glm::vec3(-64.0f*.28f,0,0)),glm::vec3(180,60,60)));
        glUniform2f(offset,0,std::fmod(-tiempo*1.25f+i*.23f,1.0f));
        glUniform1f(opacidad,1);
        malla_estela.RenderMeshModel();
    }

    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glUniform1i(efecto,2);
    glUniform2f(offset,0,0);
    textura_humo.UseTexture();
    for (int paso=4;paso>=0;--paso)
    {
        const float edad = std::fmod(tiempo_cohete,.4f)+paso*.4f;
        if (!lanzamiento || tiempo_cohete<edad) continue;
        const glm::mat4 emisor_neptuno = MatrizNeptuno(tiempo_cohete-edad);
        glm::mat4 humo_neptuno = glm::transpose(glm::mat4(glm::mat3(ctx.vista)));
        humo_neptuno[3] = emisor_neptuno * glm::vec4(0,-.4f-edad*3,0,1);
        enviar(glm::scale(humo_neptuno,glm::vec3(7+edad*9)));
        glUniform1f(opacidad,std::max(0.0f,1.0f-edad/2.0f));
        plano_estela.RenderMeshModel();
        for (int i=0;i<3;++i)
        {
            const glm::mat4 motor_local = MatrizMotor(i,tiempo_cohete-edad);
            if (motor_local[3].y <= .801f+mitad_motores[i]) continue;
            const glm::mat4 emisor = RaizCoheteBlanco()*motor_local;
            glm::mat4 humo = glm::transpose(glm::mat4(glm::mat3(ctx.vista)));
            humo[3] = emisor[3]-glm::vec4(0,mitad_motores[i]+edad*3.0f,0,0);
            enviar(glm::scale(humo,glm::vec3(6.0f+edad*8.0f)));
            glUniform1f(opacidad,std::max(0.0f,1.0f-edad/2.0f));
            plano_estela.RenderMeshModel();
        }
    }
    glUniform1i(efecto,0);
    glUniform1f(opacidad,1);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

EscenaPlanetaMorado::~EscenaPlanetaMorado()
{
    for (auto& modelo : refugios) modelo.ClearModel();
    for (auto& modelo : flora) modelo.ClearModel();
    for (auto& modelo : montanas) modelo.ClearModel();
    for (auto& modelo : meteoritos) modelo.ClearModel();
    for (auto& modelo : motores) modelo.ClearModel();
    for (auto& modelo : neptuno) modelo.ClearModel();
    sonda_cuerpo.ClearModel();
    sonda_rotor.ClearModel();
    plataforma.ClearModel();
    antena.ClearModel();
    cohete.ClearModel();
    for (auto& pieza : segundo_cohete) pieza.ClearModel();
}

//>>>>>>>>>>>>>>>>>>>>>> Segundo cohete por keyframes >>>>>>>>>>>>>>>>>>>>>>
void EscenaPlanetaMorado::CargarSegundoCohete(const std::string& ruta)
{
    std::ifstream archivo(ruta+"keyframes_cohete.txt");
    std::string linea;
    while (std::getline(archivo,linea))
    {
        std::istringstream fila(linea);
        if (linea.rfind("LAZO",0) == 0)
        {
            std::string etiqueta;
            fila >> etiqueta >> lazo_segundo;
            continue;
        }
        CuadroCohete cuadro{};
        if (!(fila >> cuadro.duracion >> cuadro.motor) || cuadro.duracion <= 0) continue;
        for (size_t i = 0; i < cuadro.piezas.size(); ++i)
        {
            auto& pieza = cuadro.piezas[i];
            if (i != 0) fila >> pieza.acoplada;
            fila >> pieza.posicion.x >> pieza.posicion.y >> pieza.posicion.z
                 >> pieza.angulos.x >> pieza.angulos.y >> pieza.angulos.z;
        }
        if (fila) cuadros_cohete.push_back(cuadro);
    }
    if (cuadros_cohete.size() < 2)
    {
        cuadros_cohete.clear();
        return;
    }
    if (lazo_segundo >= cuadros_cohete.size()-1) lazo_segundo = 0;
    const char* nombres[] = {"cuerpo","booster1","booster2","booster3","capsula"};
    for (size_t i = 0; i < segundo_cohete.size(); ++i)
        segundo_cohete[i].LoadModel(ruta+"cohete_"+nombres[i]+".obj");
    ActualizarSegundoCohete(0);
}

void EscenaPlanetaMorado::ActualizarSegundoCohete(float dt)
{
    if (cuadros_cohete.empty()) return;
    if (segundo_activo) tiempo_segundo += dt;
    while (tiempo_segundo >= cuadros_cohete[cuadro_segundo].duracion)
    {
        tiempo_segundo -= cuadros_cohete[cuadro_segundo].duracion;
        if (++cuadro_segundo >= cuadros_cohete.size()-1) cuadro_segundo = lazo_segundo;
    }
    const auto& a = cuadros_cohete[cuadro_segundo];
    const auto& b = cuadros_cohete[cuadro_segundo+1];
    const float u = tiempo_segundo/a.duracion;
    constexpr float escala = 2.5f;
    // Distancias locales de la practica, expresadas en diez milesimas de unidad.
    const glm::vec3 offsets[] = {{0,0,0},{0,7573,-9683},{-6579,-3841,-9683},
                                {6579,-3841,-9683},{0,0,13626}};
    auto matriz = [](glm::vec3 pos, glm::vec3 giro) {
        glm::mat4 m = glm::translate(glm::mat4(1),
            glm::vec3(65,6,-35) + (pos-glm::vec3(50,-2,-30))*2.75f);
        m = glm::rotate(m,glm::radians(giro.y),glm::vec3(0,1,0));
        m = glm::rotate(m,glm::radians(-giro.x),glm::vec3(1,0,0));
        return glm::rotate(m,glm::radians(giro.z),glm::vec3(0,0,1));
    };
    const auto& cuerpo_a = a.piezas[0];
    const auto& cuerpo_b = b.piezas[0];
    const glm::mat4 cuerpo = matriz(glm::mix(cuerpo_a.posicion,cuerpo_b.posicion,u),
                                  glm::mix(cuerpo_a.angulos,cuerpo_b.angulos,u));
    matrices_segundo[0] = glm::scale(cuerpo,glm::vec3(escala*2.75f));
    for (size_t i = 1; i < segundo_cohete.size(); ++i)
    {
        const auto& inicio = a.piezas[i];
        const auto& fin = b.piezas[i];
        const glm::vec3 offset = offsets[i]*(escala/10000.0f);
        if (inicio.acoplada && fin.acoplada)
            matrices_segundo[i] = glm::scale(glm::translate(cuerpo,offset*2.75f),glm::vec3(escala*2.75f));
        else
        {
            auto posicion = [&](const CuadroCohete::Pieza& pieza,const CuadroCohete::Pieza& padre) {
                if (!pieza.acoplada) return pieza.posicion;
                const glm::mat4 base = matriz(padre.posicion,padre.angulos);
                return padre.posicion + glm::mat3(base)*offset;
            };
            matrices_segundo[i] = glm::scale(matriz(
                glm::mix(posicion(inicio,cuerpo_a),posicion(fin,cuerpo_b),u),
                glm::mix(inicio.acoplada ? cuerpo_a.angulos : inicio.angulos,
                         fin.acoplada ? cuerpo_b.angulos : fin.angulos,u)),glm::vec3(escala*2.75f));
        }
    }
}

//>>>>>>>>>>>>>>>>>>>>>> Avatar esqueletal del poblado >>>>>>>>>>>>>>>>>>>>>>
void EscenaPlanetaMorado::DibujarAvatar(const ContextoRender& ctx)
{
    GLint anterior = 0, programa = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM,&anterior);
    shader_avatar.UseShader();
    glGetIntegerv(GL_CURRENT_PROGRAM,&programa);
    shader_avatar.SetDirectionalLight(&luz);
    shader_avatar.SetPointLights(nullptr,0);
    shader_avatar.SetSpotLights(nullptr,0);
    glUniformMatrix4fv(shader_avatar.GetViewLocation(),1,GL_FALSE,glm::value_ptr(ctx.vista));
    glUniformMatrix4fv(shader_avatar.GetProjectionLocation(),1,GL_FALSE,glm::value_ptr(ctx.proyeccion));
    glUniform3fv(shader_avatar.GetEyePositionLocation(),1,glm::value_ptr(ctx.pos_camara));
    glUniform3f(shader_avatar.getColorLocation(),1,1,1);
    glUniform1f(shader_avatar.GetSpecularIntensityLocation(),.18f);
    glUniform1f(shader_avatar.GetShininessLocation(),16);
    auto dibujar_actor = [&](ModeloAnimado& actor,glm::vec3 p,float giro,float escala,float fase) {
        const glm::mat4 modelo = Colocar(p,giro,glm::vec3(escala))*actor.Normalizacion();
        glUniformMatrix4fv(shader_avatar.GetModelLocation(),1,GL_FALSE,glm::value_ptr(modelo));
        actor.CalcularPose(0,fase);
        actor.Dibujar(GLuint(programa));
    };
    // Cada caminante recorre el perimetro con un desfase uniforme.
    for (int i = 0; i < 7; ++i)
    {
        const float fase = tiempo_npc + glm::two_pi<float>()*i/(7*.032f);
        const glm::vec3 punto = RutaSaru(fase);
        const glm::vec3 rumbo = RutaSaru(fase+.1f)-punto;
        dibujar_actor(saru,punto,std::atan2(rumbo.x,rumbo.z),i == 0 ? 19.0f : 17.0f,tiempo_npc*1.55f+i*.2f);
    }
    glUseProgram(anterior);
}

bool EscenaPlanetaMorado::ObjetivoCamara(int modo,glm::vec3& objetivo,glm::vec3& rumbo,float& distancia,float& altura) const
{
    if (modo != 6 || !saru_cargado) return false;
    const glm::vec3 punto = RutaSaru(tiempo_npc);
    objetivo = punto+glm::vec3(0,11,0);
    rumbo = glm::normalize(RutaSaru(tiempo_npc+.1f)-punto);
    distancia = 39;
    altura = 20;
    return true;
}

//>>>>>>>>>>>>>>>>>>>>>> Cohete auxiliar y keyframes de vuelo >>>>>>>>>>>>>>>>>>>>>>
void CoheteAuxiliar::Cargar()
{
    const std::string ruta = "assets/planeta_morado/modelos/";
    cabina.LoadModel(ruta+"cohete_auxiliar_cabina.obj");
    cuerpo.LoadModel(ruta+"cohete_auxiliar_cuerpo.obj");
    motor.LoadModel(ruta+"cohete_auxiliar_motor.obj");
    plataforma.LoadModel(ruta+"p10_plataforma.obj");
    std::ifstream archivo(ruta+"cohete_auxiliar_keyframes.txt");
    int cantidad = 0;
    archivo >> cantidad;
    for (int i = 0; i < cantidad; ++i)
    {
        glm::vec4 cuadro;
        if (!(archivo >> cuadro.x >> cuadro.y >> cuadro.z >> cuadro.w)) break;
        cuadros.push_back(cuadro);
    }
}

void CoheteAuxiliar::Actualizar(float dt, bool iniciar)
{
    if (iniciar) { activo = true; tiempo = 0; }
    if (activo && cuadros.size() > 1)
        tiempo = std::min(tiempo+dt,3.0f*float(cuadros.size()-1));
}

void CoheteAuxiliar::Dibujar(const ContextoRender& ctx, glm::vec3 base, float noche)
{
    if (cuadros.size() < 2) return;
    auto dibujar = [&](Model& modelo,const glm::mat4& matriz) {
        glUniformMatrix4fv(ctx.u_model,1,GL_FALSE,glm::value_ptr(matriz));
        modelo.RenderModel(true,ctx.u_specular_intensity,ctx.u_shininess);
    };
    dibujar(plataforma,Colocar(base,glm::half_pi<float>(),glm::vec3(.9f)));
    const size_t tramo = std::min(size_t(tiempo/3.0f),cuadros.size()-2);
    const glm::vec4 pose = glm::mix(cuadros[tramo],cuadros[tramo+1],tiempo/3.0f-tramo);
    glm::mat4 nave = Colocar(base+glm::vec3(pose.x*2.0f,23.0f+pose.y*3.0f,0),
                            glm::radians(pose.z),glm::vec3(2.6f));
    nave = glm::rotate(nave,glm::radians(pose.w),glm::vec3(0,0,1));
    GLint programa = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM,&programa);
    const GLint balizas = glGetUniformLocation(programa,"luces_cohete");
    glUniform1f(balizas,noche);
    dibujar(cuerpo,nave);
    dibujar(cabina,glm::translate(nave,glm::vec3(0,.368f,0)));
    dibujar(motor,glm::translate(nave,glm::vec3(-.85f,-2.8f,0)));
    dibujar(motor,glm::translate(nave,glm::vec3(.85f,-2.8f,0)));
    glUniform1f(balizas,0);
}

CoheteAuxiliar::~CoheteAuxiliar()
{
    cabina.ClearModel();
    cuerpo.ClearModel();
    motor.ClearModel();
    plataforma.ClearModel();
}
