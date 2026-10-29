"""ExportaciÃ³n de la escena GLB completa a OBJ/MTL con transformaciones aplicadas."""
from pathlib import Path
import numpy as np
from preparar_recorrido_naranja import ROOT, cargar


def exportar():
    gltf, accessor = cargar()
    source = ROOT/'assets/planeta_naranja/modelos/pista_3_carriles.glb'
    data = source.read_bytes()
    import struct
    length = struct.unpack_from('<I', data, 12)[0]
    binary = data[28+length:]
    folder = source.parent
    textures = folder.parent/'texturas'
    for im in gltf['images']:
        view = gltf['bufferViews'][im['bufferView']]
        start = view.get('byteOffset', 0)
        (textures/(im['name']+'.png')).write_bytes(binary[start:start+view['byteLength']])

    materials = []
    for i, mat in enumerate(gltf['materials']):
        pbr = mat.get('pbrMetallicRoughness', {})
        color = pbr.get('baseColorFactor', [1,1,1,1])
        materials += [f'newmtl Material_{i}', 'Ka 1 1 1',
                      'Kd %.6f %.6f %.6f' % tuple(color[:3]), 'Ks 0.25 0.25 0.25', 'Ns 32', 'd 1', 'illum 2']
        if 'baseColorTexture' in pbr:
            tex = gltf['textures'][pbr['baseColorTexture']['index']]
            name = gltf['images'][tex['source']]['name']
            materials.append(f'map_Kd ../texturas/{name}.png')
        materials.append('')
    (folder/'pista_3_carriles.mtl').write_text('\n'.join(materials), encoding='utf-8')

    def matrix(node):
        if 'matrix' in node:
            return np.array(node['matrix']).reshape(4,4).T
        x,y,z,w = node.get('rotation', [0,0,0,1])
        rotation = np.array([[1-2*(y*y+z*z),2*(x*y-z*w),2*(x*z+y*w)],
                             [2*(x*y+z*w),1-2*(x*x+z*z),2*(y*z-x*w)],
                             [2*(x*z-y*w),2*(y*z+x*w),1-2*(x*x+y*y)]])
        m = np.eye(4)
        m[:3,:3] = rotation @ np.diag(node.get('scale', [1,1,1]))
        m[:3,3] = node.get('translation', [0,0,0])
        return m

    offsets = [1]
    count = [0]
    with (folder/'pista_3_carriles.obj').open('w', encoding='utf-8', newline='\n') as out:
        out.write('# Circuito completo con divisores y pilares de roca\nmtllib pista_3_carriles.mtl\no PistaCompleta\n')

        def node_export(index, parent):
            node = gltf['nodes'][index]
            transform = parent @ matrix(node)
            if 'mesh' in node:
                for prim in gltf['meshes'][node['mesh']]['primitives']:
                    p = accessor(prim['attributes']['POSITION']).astype(float)
                    v = p @ transform[:3,:3].T + transform[:3,3]
                    n = accessor(prim['attributes']['NORMAL']).astype(float) @ np.linalg.inv(transform[:3,:3])
                    n /= np.linalg.norm(n, axis=1)[:,None]
                    uv = accessor(prim['attributes']['TEXCOORD_0']).copy() if 'TEXCOORD_0' in prim['attributes'] else np.zeros((len(v),2))
                    uv[:,1] = 1-uv[:,1]
                    out.writelines('v %.6f %.6f %.6f\n' % tuple(row) for row in v)
                    out.writelines('vt %.7f %.7f\n' % tuple(row) for row in uv)
                    out.writelines('vn %.7f %.7f %.7f\n' % tuple(row) for row in n)
                    out.write('usemtl Material_%d\n' % prim['material'])
                    faces = accessor(prim['indices']).reshape(-1,3).astype(int)+offsets[0]
                    if np.linalg.det(transform[:3,:3]) < 0:
                        faces = faces[:,::-1]
                    out.writelines('f %d/%d/%d %d/%d/%d %d/%d/%d\n' % tuple(np.repeat(row,3)) for row in faces)
                    offsets[0] += len(v)
                    count[0] += len(faces)
            for child in node.get('children', []):
                node_export(child, transform)

        for index in gltf['scenes'][gltf.get('scene',0)]['nodes']:
            node_export(index, np.eye(4))
    print(f'OBJ completo: {offsets[0]-1} vÃ©rtices; {count[0]} triÃ¡ngulos; {len(gltf["materials"])} materiales.')


if __name__ == '__main__':
    exportar()
