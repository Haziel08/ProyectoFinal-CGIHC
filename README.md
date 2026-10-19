# Odisea espacial y sistemas planetarios

Proyecto final — Computación Gráfica e IHC 2027-1 (Lab. grupo 13).
OpenGL 4.3 · C++17 · GLFW · GLEW · GLM · Assimp · Visual Studio 2022 (x64).

## Abrir y ejecutar

1. Abrir `ProyectoFinal.sln` con Visual Studio 2022.
2. Elegir **x64** y **Debug** o **Release**, y dar `F5`.

No hay que configurar nada: las rutas son relativas y las librerías vienen en `ProyectoFinal/external/`.
El `.exe` queda en `bin/Release/` (o `bin/Debug/`) junto con las DLLs, `shaders/` y `assets/`.

## Estructura

```
ProyectoFinal.sln
ProyectoFinal/
├── src/
│   ├── main.cpp                  ← solo elige la escena activa y la dibuja
│   ├── motor/                    ← clases base de las prácticas (Window, Camera, Shader, Model, Skybox…)
│   │   └── luces/
│   └── escenas/
│       ├── Escena.h/.cpp         ← clase base (skybox, cámara y luces de cada escena)
│       ├── espacio/              ← Estación XQ6 Platform, Nave Aurora, satélites, lunas, asteroides
│       ├── planeta_naranja/      ← Pista de carreras, Seamoth
│       ├── planeta_morado/       ← Plataforma Neptuno y cohete, módulos, corales
│       └── planeta_azul/         ← Criaturas y chozas tribales
├── shaders/
├── assets/
│   ├── comun/texturas/plain.png  ← textura blanca por defecto de Model
│   ├── espacio/         {modelos, texturas, skybox}
│   ├── planeta_naranja/ {modelos, texturas, skybox}
│   ├── planeta_morado/  {modelos, texturas, skybox}
│   └── planeta_azul/    {modelos, texturas, skybox}
└── external/  {include, glm, lib, bin}
docs/  {bitacoras, manuales}
```

**Regla:** cada quien trabaja en `src/escenas/<su escena>/` y `assets/<su escena>/`. `main.cpp`, `motor/` y `shaders/` solo se tocan avisando al equipo.

- El `.vcxproj` toma los archivos con comodines: agregar un `.cpp`/`.h` en tu carpeta no modifica el proyecto (si no aparece, clic derecho al proyecto → *Recargar*).
- Las teclas de cada escena se leen dentro de su `Actualizar()` con `ventana.TeclaPresionada(GLFW_KEY_X)` (una vez por pulsación) o `ventana.TeclaSostenida(GLFW_KEY_X)` (mientras esté abajo). Ya no se agregan variables a `Window`.

## Skybox

Cada escena carga el suyo desde `assets/<escena>/skybox/` con 6 imágenes `rt, lf, up, dn, bk, ft` (mismo orden que en clase). Por defecto `.jpg`; para otro formato: `CargarSkybox("assets/planeta_azul/skybox/", ".tga")`.
Los actuales son **provisionales** (generados por código): se reemplazan sobrescribiendo los archivos.

## Controles

| Tecla | Acción |
|---|---|
| `F1` | Espacio |
| `F2` | Planeta Naranja |
| `F3` | Planeta Morado |
| `F4` | Planeta Azul |
| `W A S D` + ratón | Cámara libre (cada escena recuerda dónde dejaste su cámara) |
| `ESC` | Salir |

## Git

- `main`: versiones que compilan y corren · `develop`: integración.
- Una rama por integrante: `nombre/tema` (ej. `haziel/planeta-azul`).
- Trabajar en tu rama → `git pull origin develop` → comprobar que compila → *Pull Request* a `develop`.

Restricciones: sin rutas absolutas, sin comprimidos, ningún archivo > 100 MB, modelos + texturas < 100 MB.
