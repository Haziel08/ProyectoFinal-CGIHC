"""Comprueba el casco contra toda la pista, divisores y cruces mediante SAT."""
from pathlib import Path
import numpy as np
import re
from preparar_recorrido_naranja import cargar, recorrido, ROOT


def ejecutar():
    source = (ROOT/'src/escenas/planeta_naranja/EscenaPlanetaNaranja.cpp').read_text(encoding='utf-8')
    ship_scale = float(re.search(r'ESCALA_NAVE = ([0-9.]+)f', source).group(1))
    half_length = 13.0 * ship_scale
    points, labels, _ = recorrido()
    gltf, accessor = cargar()
    triangles = []
    for node in gltf['nodes']:
        if not (node['name'].startswith('Seg') or node['name'].startswith('Dos_divisores')):
            continue
        for p in gltf['meshes'][node['mesh']]['primitives']:
            v = accessor(p['attributes']['POSITION'])
            ids = accessor(p['indices']).reshape(-1, 3)
            triangles.extend(v[ids])
    triangles = np.array(triangles, dtype=float)
    tri_min, tri_max = triangles.min(axis=1), triangles.max(axis=1)
    distances = np.r_[0, np.cumsum(np.linalg.norm(np.diff(points, axis=0), axis=1))]
    length = distances[-1]

    def point(s):
        s = np.asarray(s) % length
        i = np.clip(np.searchsorted(distances, s, side='right') - 1, 0, len(points)-2)
        u = (s-distances[i])/(distances[i+1]-distances[i])
        return points[i] + (points[i+1]-points[i])*np.asarray(u)[..., None]

    ship = ROOT/'assets/planeta_naranja/modelos/nave_espacial.obj'
    vertices = np.array([[float(x) for x in line.split()[1:4]] for line in ship.read_text().splitlines() if line.startswith('v ')])
    bmin, bmax = vertices.min(axis=0), vertices.max(axis=0)
    box_center, half = (bmin+bmax)/2, (bmax-bmin)/2
    samples = np.unique(np.r_[np.arange(0, length, 2.0), distances[:-1]])
    bad = []
    for s in samples:
        center = point(s)
        forward = point(s+half_length)-point(s-half_length)
        forward /= np.linalg.norm(forward)
        right = np.cross(forward, [0,1,0]); right /= np.linalg.norm(right)
        up = np.cross(right, forward)
        clearance = max(0.0, np.max((point(s+np.linspace(-half_length,half_length,17))-center)@up))
        pos = center+up*(4+clearance)
        basis = np.stack([-forward, up, -right], axis=1)
        # Triángulos cercanos al casco (incluye otros niveles del cruce).
        keep = np.all(tri_min < pos+22, axis=1) & np.all(tri_max > pos-22, axis=1)
        v = (triangles[keep]-pos)@basis/ship_scale-box_center
        if not len(v): continue
        overlap = np.all(v.min(axis=1) <= half, axis=1) & np.all(v.max(axis=1) >= -half, axis=1)
        edges = np.roll(v,-1,axis=1)-v
        axes = [np.cross(edges[:,0], edges[:,1])]
        for e in range(3):
            for axis in np.eye(3): axes.append(np.cross(edges[:,e],axis))
        for axis in axes:
            projection = np.einsum('nvc,nc->nv',v,axis)
            radius = np.abs(axis)@half
            overlap &= (projection.min(axis=1) <= radius+1e-7) & (projection.max(axis=1) >= -radius-1e-7)
        if np.any(overlap): bad.append(float(s))
    assert not bad, f'Casco toca la pista en {len(bad)} posiciones: {bad[:12]}'
    assert (bmax[2]-bmin[2]) * ship_scale < 40
    assert np.allclose(points[0],points[-1])
    print(f'OK: {len(samples)} posiciones; casco sin intersecciones con pista, divisores ni cruce superior.')
    print(f'Carril: {40*.18:.2f}; ancho nave: {(bmax[2]-bmin[2])*ship_scale*.18:.2f}; vuelta: {length*.18/12:.1f} segundos.')


if __name__ == '__main__':
    ejecutar()
