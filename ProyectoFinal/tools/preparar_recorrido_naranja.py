"""Extrae la polilÃ­nea del carril central del GLB, sin modificar la pista."""
import json
import struct
from pathlib import Path
import numpy as np

ROOT = Path(__file__).resolve().parents[1]


def cargar():
    data = (ROOT / 'assets/planeta_naranja/modelos/pista_3_carriles.glb').read_bytes()
    size = struct.unpack_from('<I', data, 12)[0]
    gltf = json.loads(data[20:20 + size])
    binary = data[28 + size:]

    def accessor(index):
        a = gltf['accessors'][index]
        b = gltf['bufferViews'][a['bufferView']]
        dtype = {5126: '<f4', 5125: '<u4', 5123: '<u2'}[a['componentType']]
        count = {'VEC3': 3, 'VEC2': 2, 'SCALAR': 1}[a['type']]
        item = np.dtype(dtype).itemsize
        return np.ndarray((a['count'], count), dtype=dtype, buffer=binary,
                          offset=b.get('byteOffset', 0) + a.get('byteOffset', 0),
                          strides=(b.get('byteStride', item * count), item))
    return gltf, accessor


def recorrido():
    gltf, accessor = cargar()
    points, labels, surfaces = [], [], []
    for node in gltf['nodes']:
        if not node['name'].startswith('Seg'):
            continue
        p = gltf['meshes'][node['mesh']]['primitives'][0]
        vertices = accessor(p['attributes']['POSITION'])
        uv = accessor(p['attributes']['TEXCOORD_0'])
        indices = accessor(p['indices']).reshape(-1, 3)
        triangles = vertices[indices]
        normal = np.cross(triangles[:, 1] - triangles[:, 0], triangles[:, 2] - triangles[:, 0])
        normal /= np.linalg.norm(normal, axis=1)[:, None]
        top = normal[:, 1] > 0.5
        surfaces.extend(triangles[top])
        ids = np.unique(indices[top])
        v, tex = vertices[ids], uv[ids]
        rows = []
        if 'Curva' in node['name']:
            # Cada anillo tiene cuatro lÃ­mites de carril; su media es el centro.
            for t in sorted(set(tex[:, 1].round(5)), reverse=True):
                ring = np.unique(v[np.isclose(tex[:, 1], t, atol=1e-5)].round(4), axis=0)
                rows.append(ring.mean(axis=0))
        else:
            first = v[0]
            second = next(row for row in v[1:] if np.linalg.norm(row - first) > 0.01)
            transverse = second - first
            axis = 0 if abs(transverse[2]) > abs(transverse[0]) else 2
            for t in sorted(set(v[:, axis].round(3))):
                ring = np.unique(v[np.isclose(v[:, axis], t, atol=0.002)].round(4), axis=0)
                rows.append(ring.mean(axis=0))
            previous = points[-1] if points else np.zeros(3)
            if np.linalg.norm(rows[-1] - previous) < np.linalg.norm(rows[0] - previous):
                rows.reverse()
        if points:
            assert np.linalg.norm(points[-1] - rows[0]) < 0.02, node['name']
        for row in rows:
            if points and np.linalg.norm(row - points[-1]) < 0.02:
                continue
            points.append(row)
            labels.append(node['name'])
    assert np.linalg.norm(points[-1] - points[0]) < 0.02
    points[-1] = points[0].copy()
    return np.array(points, dtype=float), labels, np.array(surfaces, dtype=float)


def generar():
    points, labels, _ = recorrido()
    output = ROOT / 'src/escenas/planeta_naranja/RecorridoNaranja.h'
    lines = ['#pragma once', '// Generado desde las caras superiores del GLB por tools/preparar_recorrido_naranja.py.',
             '// Coordenadas locales, Y arriba. Curvas subdivididas en rectas del carril central.',
             '#include <glm.hpp>', 'namespace RecorridoNaranja {', 'inline const glm::vec3 PUNTOS[] = {']
    for p, label in zip(points, labels):
        lines.append('    { %.6ff, %.6ff, %.6ff }, // %s' % (*p, label))
    lines += ['};', 'inline constexpr int CANTIDAD = sizeof(PUNTOS) / sizeof(PUNTOS[0]);', '}']
    output.write_text('\n'.join(lines) + '\n', encoding='utf-8')
    print(f'{len(points)-1} segmentos consecutivos; cierre exacto; 70 secciones del modelo.')
    print('ExtensiÃ³n local:', points.min(axis=0), points.max(axis=0))


if __name__ == '__main__':
    generar()
