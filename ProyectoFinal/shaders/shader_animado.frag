#version 330
// Shader animado (agua, fuego...): textura que se mueve con el tiempo,
// transparencia y luz direccional.
//   Agua:  usarLuz = 1, transparencia 0.6
//   Fuego: usarLuz = 0, transparencia 0.9, velocidadUV hacia arriba (0, 0.5)

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform sampler2D textura;
uniform float tiempo;
uniform vec2  velocidadUV;     // cuanto se mueve la textura por segundo
uniform float repeticionUV;    // cuantas veces se repite la textura
uniform vec3  colorBase;       // color que multiplica a la textura
uniform float transparencia;   // 0 = invisible, 1 = opaco

uniform int   usarLuz;         // 1 = recibe la luz direccional, 0 = no
uniform vec3  luzDireccion;    // hacia donde viaja la luz (igual que DirectionalLight)
uniform vec3  luzColor;
uniform float luzAmbiental;
uniform float luzDifusa;
uniform vec3  posCamara;
uniform float brillo;          // shininess del reflejo

void main()
{
	// La textura se recorre con el tiempo: parece que el agua fluye
	vec2 uv = TexCoord * repeticionUV + velocidadUV * tiempo;
	vec3 rgb = texture(textura, uv).rgb * colorBase;

	if (usarLuz == 1)
	{
		vec3 N = normalize(Normal);
		vec3 L = normalize(-luzDireccion);           // hacia la luz
		vec3 V = normalize(posCamara - FragPos);     // hacia la camara

		// Igual que en shader_light: ambiental + difusa + especular
		float difusa = max(dot(N, L), 0.0);
		float especular = pow(max(dot(V, reflect(-L, N)), 0.0), brillo);
		rgb = rgb * luzColor * (luzAmbiental + luzDifusa * difusa)
			+ luzColor * especular * luzDifusa;
	}

	color = vec4(rgb, transparencia);
}
