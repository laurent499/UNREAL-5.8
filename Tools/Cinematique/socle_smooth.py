"""Arrondit le volume du socle (OBJ sans UV) : subdivision des triangles puis lissage de Taubin, normales de Phong.

python socle_smooth.py socle_volume.obj [niveaux=3] [passes=12]

Chaque niveau coupe chaque triangle en 4 (milieux d'aretes). Le lissage de Taubin (lambda/mu) arrondit sans
retrecir le volume. Les normales de sommet sont ecrites dans le fichier (`f v//vn`). Original garde en .orig.obj.
"""
import shutil
import sys

import numpy as np

path = sys.argv[1]
levels = int(sys.argv[2]) if len(sys.argv) > 2 else 3
passes = int(sys.argv[3]) if len(sys.argv) > 3 else 12

head, V, F = [], [], []
with open(path) as f:
    for line in f:
        if line.startswith('v '):
            V.append(line.split()[1:4])
        elif line.startswith('f '):
            F.append([int(t.split('/')[0]) - 1 for t in line.split()[1:4]])
        elif line.startswith('#'):
            head.append(line)
V = np.array(V, dtype=np.float64)
F = np.array(F)
print(len(V), 'sommets', len(F), 'faces')

for _ in range(levels):
    edges = {}
    new = []

    def mid(a, b):
        k = (a, b) if a < b else (b, a)
        if k not in edges:
            edges[k] = len(V) + len(new)
            new.append((V[a] + V[b]) / 2)
        return edges[k]

    nf = []
    for a, b, c in F:
        ab, bc, ca = mid(a, b), mid(b, c), mid(c, a)
        nf += [(a, ab, ca), (ab, b, bc), (ca, bc, c), (ab, bc, ca)]
    V = np.vstack([V, np.array(new)])
    F = np.array(nf)
print('apres subdivision :', len(V), 'sommets', len(F), 'faces')

E = np.concatenate([F[:, [0, 1]], F[:, [1, 2]], F[:, [2, 0]]])
E = np.unique(np.sort(E, axis=1), axis=0)
E = np.concatenate([E, E[:, ::-1]])
deg = np.bincount(E[:, 0], minlength=len(V)).astype(np.float64)


def laplace(P, k):
    S = np.zeros_like(P)
    np.add.at(S, E[:, 0], P[E[:, 1]])
    return P + k * (S / deg[:, None] - P)


for _ in range(passes):
    V = laplace(V, 0.5)
    V = laplace(V, -0.53)

fn = np.cross(V[F[:, 1]] - V[F[:, 0]], V[F[:, 2]] - V[F[:, 0]])
N = np.zeros_like(V)
for k in range(3):
    np.add.at(N, F[:, k], fn)
# normales vers l'exterieur : du centroide vers le sommet
if np.sum(N * (V - V.mean(0))) < 0:
    N = -N
N /= np.maximum(np.linalg.norm(N, axis=1, keepdims=True), 1e-9)

shutil.copyfile(path, path.replace('.obj', '.orig.obj'))
with open(path, 'w', newline='\n') as out:
    out.write(''.join(head))
    out.write(''.join('v %.2f %.2f %.2f\n' % tuple(p) for p in V))
    out.write(''.join('vn %.4f %.4f %.4f\n' % tuple(n) for n in N))
    out.write('vt 0.0 0.0\n')  # Interchange exige un indice d'UV valide
    out.write(''.join('f %d/1/%d %d/1/%d %d/1/%d\n' % (a + 1, a + 1, b + 1, b + 1, c + 1, c + 1) for a, b, c in F))
print('ecrit', path, ':', len(V), 'sommets')
