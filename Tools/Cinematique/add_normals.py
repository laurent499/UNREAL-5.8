"""Ajoute des normales lissees (Phong) a un OBJ de terrain : sans elles, Unreal ombre chaque triangle a plat (aspect facette).

python add_normals.py fichier.obj [passes=2]

Les normales de sommet sont la moyenne des normales des faces voisines (ponderee par la surface), puis lissees
`passes` fois avec les sommets voisins. Les faces passent de `f v/vt` a `f v/vt/vn`. L'original est garde
en .orig.obj. A reimporter ensuite dans Unreal (clic droit sur le mesh > Reimport) ; verifier que l'import prend
les normales du fichier (Interchange : pipeline de mesh statique, ne pas recalculer les normales).
"""
import shutil
import sys

import numpy as np

path = sys.argv[1]
passes = int(sys.argv[2]) if len(sys.argv) > 2 else 2

head, V, faces = [], [], []
with open(path) as f:
    for line in f:
        if line.startswith('v '):
            V.append(line.split()[1:4])
            head.append(line)
        elif line.startswith('f '):
            p = line.split()[1:]
            faces.append([[int(t) for t in s.split('/')[:2]] for s in p])
        elif line.startswith('vn '):
            sys.exit('Ce fichier a deja des normales')
        else:
            head.append(line)
V = np.array(V, dtype=np.float64)
F = np.array(faces)  # (nf, 3, 2) : indices v, vt (base 1)
vi = F[:, :, 0] - 1
print(len(V), 'sommets', len(F), 'faces')

p0, p1, p2 = V[vi[:, 0]], V[vi[:, 1]], V[vi[:, 2]]
fn = np.cross(p1 - p0, p2 - p0)  # longueur = 2 x surface : ponderation naturelle
N = np.zeros_like(V)
for k in range(3):
    np.add.at(N, vi[:, k], fn)
if N[:, 2].sum() < 0:
    N = -N  # la surface regarde vers le haut (Z)
n = np.linalg.norm(N, axis=1, keepdims=True)
N = N / np.maximum(n, 1e-9)

# lissage supplementaire par les voisins d'arete
E = np.concatenate([vi[:, [0, 1]], vi[:, [1, 2]], vi[:, [2, 0]]])
E = np.concatenate([E, E[:, ::-1]])
for _ in range(passes):
    S = N.copy()
    np.add.at(S, E[:, 0], N[E[:, 1]])
    N = S / np.maximum(np.linalg.norm(S, axis=1, keepdims=True), 1e-9)
print('normales calculees, angle moyen avec Z :', float(np.degrees(np.arccos(np.clip(N[:, 2], -1, 1))).mean()))

shutil.copyfile(path, path.replace('.obj', '.orig.obj'))
# les lignes d'en-tete (v, vt, mtllib...) sont reecrites telles quelles ; les vn viennent juste apres
with open(path, 'w', newline='\n') as out:
    out.write(''.join(head))
    out.write(''.join('vn %.4f %.4f %.4f\n' % (a, b, c) for a, b, c in N))
    out.write(''.join('f %d/%d/%d %d/%d/%d %d/%d/%d\n' % (
        s[0][0], s[0][1], s[0][0], s[1][0], s[1][1], s[1][0], s[2][0], s[2][1], s[2][0]) for s in F))
print('ecrit', path)
