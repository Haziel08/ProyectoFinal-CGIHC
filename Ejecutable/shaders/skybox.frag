#version 330
in vec3 tex_coords;
out vec4 color;
uniform samplerCube skybox;
uniform bool transicion_natural;

// Consultar ambas caras con pesos continuos en aristas y esquinas.
vec3 Cara(vec3 direccion, int eje)
{
    vec3 q = direccion / max(abs(direccion[eje]), 0.0001);
    q = clamp(q, vec3(-0.998), vec3(0.998));
    q[eje] = sign(direccion[eje]);
    return texture(skybox, q).rgb;
}
void main()
{
    vec3 d = normalize(tex_coords);
    if (!transicion_natural) { color = texture(skybox, d); return; }
    // Proyeccion angular para reducir el estiramiento lateral del paisaje.
    float mayor = max(abs(d.x), abs(d.z));
    vec3 cilindrica = d;
    if (abs(d.x) > abs(d.z))
        cilindrica.z = mayor * atan(d.z / max(abs(d.x), 0.0001)) / 0.785398163;
    else
        cilindrica.x = mayor * atan(d.x / max(abs(d.z), 0.0001)) / 0.785398163;
    vec3 pesos = pow(abs(d), vec3(14.0));
    pesos /= max(pesos.x + pesos.y + pesos.z, 0.00001);
    color = vec4(Cara(cilindrica, 0) * pesos.x + Cara(d, 1) * pesos.y
        + Cara(cilindrica, 2) * pesos.z, 1.0);
}
