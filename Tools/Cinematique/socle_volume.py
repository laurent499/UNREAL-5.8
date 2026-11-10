"""Surface du socle (OBJ Lambert-93) -> volume ferme recentre, prolonge vers le bas en flancs evases.

Le socle livre n'est qu'une surface : vu a travers la glace translucide, il flotterait. On prolonge chaque
point du bord vers le bas et vers l'exterieur (pente --pente, en m horizontal par m vertical) jusqu'a
--profondeur m sous le point le plus bas, puis on ferme le fond.
Sortie dans la meme convention que recentre_obj.py (cm, Z haut, x = est, y = nord, origine --origine).
Usage : python -I socle_volume.py socle.obj socle_volume.obj --origine 999911 6533232 4779
"""
import argparse
import math
from collections import defaultdict

p = argparse.ArgumentParser()
p.add_argument('source')
p.add_argument('sortie')
p.add_argument('--origine', type=float, nargs=3, required=True)
p.add_argument('--profondeur', type=float, default=25.0)
p.add_argument('--pente', type=float, default=0.7)
a = p.parse_args()
ox, oy, oz = a.origine

verts, faces = [], []
for l in open(a.source, encoding='latin-1'):
    t = l.split()
    if t and t[0] == 'v':
        verts.append(tuple(map(float, t[1:4])))
    elif t and t[0] == 'f':
        faces.append([int(k.split('/')[0]) - 1 for k in t[1:]])

# Bord : aretes portees par une seule face, chainees en boucle
count = defaultdict(int)
for f in faces:
    for i in range(len(f)):
        e = tuple(sorted((f[i], f[(i + 1) % len(f)])))
        count[e] += 1
nxt = defaultdict(list)
for f in faces:
    for i in range(len(f)):
        u, v = f[i], f[(i + 1) % len(f)]
        if count[tuple(sorted((u, v)))] == 1:
            nxt[u].append(v)
start = next(iter(nxt))
loop, cur = [start], nxt[start][0]
while cur != start and len(loop) <= len(nxt):
    loop.append(cur)
    cur = nxt[cur][0]
print(f'{len(verts)} sommets, {len(faces)} faces, bord de {len(loop)} points')

cx = sum(verts[i][0] for i in loop) / len(loop)
cy = sum(verts[i][1] for i in loop) / len(loop)
bottom = min(v[2] for v in verts) - a.profondeur
low = []
for i in loop:
    x, y, z = verts[i]
    dx, dy = x - cx, y - cy
    n = math.hypot(dx, dy) or 1.0
    off = (z - bottom) * a.pente
    low.append((x + dx / n * off, y + dy / n * off, bottom))

out = verts + low
base = len(verts)
new_faces = [f[:] for f in faces]
L = len(loop)
for k in range(L):
    i, j = loop[k], loop[(k + 1) % L]
    li, lj = base + k, base + (k + 1) % L
    # meme sens que l'arete de bord dans la face du dessus : flancs tournes vers l'exterieur
    new_faces.append([j, i, li])
    new_faces.append([j, li, lj])
# Fond : eventail depuis le centre
out.append((cx, cy, bottom))
c = len(out) - 1
for k in range(L):
    new_faces.append([base + k, base + (k + 1) % L, c])

with open(a.sortie, 'w', newline='\n') as f:
    f.write(f'# {a.source} prolonge vers le bas : origine Lambert-93 {ox:.0f} {oy:.0f} {oz:.0f} (m)\n')
    f.write('# Unites cm, Z haut (x = est, y = nord, z = altitude)\n')
    for x, y, z in out:
        f.write(f'v {(x - ox) * 100:.2f} {(y - oy) * 100:.2f} {(z - oz) * 100:.2f}\n')
    for fc in new_faces:
        f.write('f ' + ' '.join(str(i + 1) for i in fc) + '\n')
print(f'{len(out)} sommets, {len(new_faces)} faces, fond a {bottom:.1f} m -> {a.sortie}')
