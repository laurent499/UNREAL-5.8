"""Nuage de points colore (LAS/LAZ, Lambert-93) -> mesh texture OBJ pour Unreal.

Le nuage est rasterise sur une grille reguliere : altitude = mediane des points de la cellule,
couleur = moyenne. Les petits trous sont bouches par diffusion, les zones sans donnees restent
hors du mesh. Sortie : OBJ (cm, Z haut, recentre sur une origine Lambert-93), MTL et texture PNG
(orthophoto issue des couleurs du nuage). L'import OBJ d'Unreal garde Z haut et inverse Y :
X est, Y sud, Z haut, comme les nuages LiDAR, donc calable avec le Calage Lambert-93.

Usage : python -I laz_to_mesh.py nuage.laz sortie_sans_extension [--pas 0.5] [--texture 4096]
        [--origine 999911 6533232 4779]
Dependances : pip install "laspy[lazrs]" numpy pillow
"""
import argparse
import pathlib

import laspy
import numpy as np
from PIL import Image

p = argparse.ArgumentParser()
p.add_argument('source')
p.add_argument('sortie')
p.add_argument('--pas', type=float, default=0.5, help='taille des cellules du mesh (m)')
p.add_argument('--texture', type=int, default=4096, help='cote de la texture (px)')
p.add_argument('--origine', type=float, nargs=3, default=None, help='origine Lambert-93 retranchee (m)')
p.add_argument('--trou', type=int, default=6, help='rayon de bouchage des trous (cellules)')
a = p.parse_args()

las = laspy.read(a.source)
x, y, z = np.asarray(las.x), np.asarray(las.y), np.asarray(las.z)
has_rgb = 'red' in las.point_format.dimension_names
if has_rgb:
    rgb = np.stack([las.red, las.green, las.blue], axis=1).astype(np.float32)
    rgb /= 65535.0 if rgb.max() > 255 else 255.0
x0, y0, x1, y1 = x.min(), y.min(), x.max(), y.max()
origin = np.array(a.origine) if a.origine else np.round([(x0 + x1) / 2, (y0 + y1) / 2, (z.min() + z.max()) / 2])
print(f'{len(x):,} points, emprise {x1 - x0:.1f} x {y1 - y0:.1f} m, origine {origin}')


def fill(grid, mask, radius):
    """Bouche les trous jusqu'a `radius` cellules par moyenne des voisins connus, passe par passe."""
    g, m = grid.copy(), mask.copy()
    for _ in range(radius):
        known = m.astype(np.float32)
        acc = np.zeros_like(g)
        cnt = np.zeros(m.shape, np.float32)
        for dy in (-1, 0, 1):
            for dx in (-1, 0, 1):
                if dx == 0 and dy == 0:
                    continue
                acc += np.roll(np.roll(g * known[..., None] if g.ndim == 3 else g * known, dy, 0), dx, 1)
                cnt += np.roll(np.roll(known, dy, 0), dx, 1)
        new = (~m) & (cnt >= 3)
        if not new.any():
            break
        c = cnt[new][:, None] if g.ndim == 3 else cnt[new]
        g[new] = acc[new] / c
        m |= new
    return g, m


# --- Grille d'altitude (mediane approchee par tri)
nx, ny = int(np.ceil((x1 - x0) / a.pas)) + 1, int(np.ceil((y1 - y0) / a.pas)) + 1
ix = ((x - x0) / a.pas).astype(np.int64)
iy = ((y1 - y) / a.pas).astype(np.int64)  # ligne 0 = nord
cell = iy * nx + ix
order = np.lexsort((z, cell))
cs, zs = cell[order], z[order]
starts = np.flatnonzero(np.r_[True, cs[1:] != cs[:-1]])
ends = np.r_[starts[1:], len(cs)]
height = np.zeros(nx * ny, np.float32)
height[cs[starts]] = zs[(starts + ends) // 2]
mask = np.zeros(nx * ny, bool)
mask[cs[starts]] = True
height, mask = height.reshape(ny, nx), mask.reshape(ny, nx)
height, mask = fill(height, mask, a.trou)
print(f'grille {nx} x {ny}, {mask.mean() * 100:.1f} % couverte')

# --- Orthophoto (moyenne des couleurs par pixel)
tw = th = a.texture
if has_rgb:
    tx = np.clip(((x - x0) / (x1 - x0) * (tw - 1)).astype(np.int64), 0, tw - 1)
    ty = np.clip(((y1 - y) / (y1 - y0) * (th - 1)).astype(np.int64), 0, th - 1)
    pix = ty * tw + tx
    img = np.zeros((tw * th, 3), np.float64)
    cnt = np.bincount(pix, minlength=tw * th).astype(np.float64)
    for c in range(3):
        img[:, c] = np.bincount(pix, weights=rgb[:, c], minlength=tw * th)
    known = cnt > 0
    img[known] /= cnt[known, None]
    img, known = fill(img.reshape(th, tw, 3).astype(np.float32), known.reshape(th, tw), 12)
    Image.fromarray((np.clip(img, 0, 1) * 255).astype(np.uint8)).save(a.sortie + '.png')

# --- Mesh : un sommet par cellule couverte, deux triangles par carre entierement couvert
out = pathlib.Path(a.sortie)
vid = -np.ones((ny, nx), np.int64)
vy, vx = np.nonzero(mask)
vid[vy, vx] = np.arange(len(vx)) + 1
east = x0 + vx * a.pas
north = y1 - vy * a.pas
with open(out.with_suffix('.obj'), 'w', newline='\n') as f:
    f.write(f'# {pathlib.Path(a.source).name} : origine Lambert-93 {origin[0]:.0f} {origin[1]:.0f} {origin[2]:.0f} (m)\n')
    f.write('# Unites cm, Z haut (x = est, y = nord, z = altitude)\n')
    f.write(f'mtllib {out.name}.mtl\nusemtl {out.name}\n')
    v = np.stack([(east - origin[0]) * 100, (north - origin[1]) * 100, (height[vy, vx] - origin[2]) * 100], 1)
    f.write(''.join(f'v {p[0]:.1f} {p[1]:.1f} {p[2]:.1f}\n' for p in v))
    uv = np.stack([(east - x0) / (x1 - x0), (north - y0) / (y1 - y0)], 1)
    f.write(''.join(f'vt {t[0]:.5f} {t[1]:.5f}\n' for t in uv))
    a00, a01, a10, a11 = vid[:-1, :-1], vid[:-1, 1:], vid[1:, :-1], vid[1:, 1:]
    quad = (a00 > 0) & (a01 > 0) & (a10 > 0) & (a11 > 0)
    q = np.stack([a00[quad], a01[quad], a10[quad], a11[quad]], 1)
    # Sens anti-horaire vu du dessus (est, nord)
    f.write(''.join(f'f {s[0]}/{s[0]} {s[2]}/{s[2]} {s[1]}/{s[1]}\nf {s[1]}/{s[1]} {s[2]}/{s[2]} {s[3]}/{s[3]}\n' for s in q))
with open(str(out) + '.mtl', 'w', newline='\n') as f:
    f.write(f'newmtl {out.name}\nKd 1 1 1\n' + (f'map_Kd {out.name}.png\n' if has_rgb else ''))
print(f'{len(v):,} sommets, {2 * len(q):,} triangles -> {out.with_suffix(".obj")}')
