"""Dalle LiDAR HD (LAS/LAZ, Lambert-93) -> terrain OBJ texture avec l'orthophoto IGN, pour remplacer Google.

- Altitude : mediane des points par cellule (--pas m), trous bouches par diffusion.
- Sous un mesh plus detaille deja pose (--dessous : OBJ issu de laz_to_mesh.py, meme origine), le terrain
  passe sous ce mesh (min des deux - --retrait m) pour ne pas le traverser.
- Zones sans donnees (dalle manquante) : hors du mesh.
- Bord : le terrain plonge de --plongee m sur --bande m pour passer sous Google, decoupe --decoupe m plus a
  l'interieur (contour Lambert-93 ecrit a cote : <sortie>_decoupe.json, pour ClipOutlineLambert93).
- Orthophoto deja telechargee (<sortie>.png) : reutilisee.
- Texture : orthophoto IGN (Geoplateforme, WMS ORTHOIMAGERY.ORTHOPHOTOS) en mosaique, 8192 x 4096 px.
Sortie : OBJ (cm, Z haut, recentre sur --origine), MTL, PNG.

Usage : python -I terrain_lidar.py dalle.laz sortie --origine 999911 6533232 4779 --dessous glace_sommet.obj
Dependances : pip install "laspy[lazrs]" numpy pillow opencv-python-headless
"""
import argparse
import io
import json
import pathlib
import urllib.request

import cv2
import laspy
import numpy as np
from PIL import Image

Image.MAX_IMAGE_PIXELS = None
p = argparse.ArgumentParser()
p.add_argument('source')
p.add_argument('sortie')
p.add_argument('--origine', type=float, nargs=3, required=True)
p.add_argument('--pas', type=float, default=2.0)
p.add_argument('--dessous', default=None)
p.add_argument('--retrait', type=float, default=1.0)
p.add_argument('--socle', default=None, help='OBJ du socle rocheux (meme origine) : le terrain passe dessous')
p.add_argument('--marge_socle', type=float, default=8.0)
p.add_argument('--bande', type=float, default=40.0)
p.add_argument('--plongee', type=float, default=6.0)
p.add_argument('--decoupe', type=float, default=25.0, help='retrait du contour de decoupe Google (m)')
p.add_argument('--texture', type=int, nargs=2, default=[8192, 4096])
a = p.parse_args()
origin = np.array(a.origine)


def fill(grid, mask, radius):
    g, m = grid.copy(), mask.copy()
    for _ in range(radius):
        known = m.astype(np.float32)
        acc = np.zeros_like(g)
        cnt = np.zeros(m.shape, np.float32)
        for dy in (-1, 0, 1):
            for dx in (-1, 0, 1):
                if dx or dy:
                    acc += np.roll(np.roll(g * known, dy, 0), dx, 1)
                    cnt += np.roll(np.roll(known, dy, 0), dx, 1)
        new = (~m) & (cnt >= 3)
        if not new.any():
            break
        g[new] = acc[new] / cnt[new]
        m |= new
    return g, m


# --- Grille d'altitude
las = laspy.read(a.source)
x, y, z = np.asarray(las.x), np.asarray(las.y), np.asarray(las.z)
x0, y0, x1, y1 = np.floor(x.min()), np.floor(y.min()), np.ceil(x.max()), np.ceil(y.max())
nx, ny = int((x1 - x0) / a.pas) + 1, int((y1 - y0) / a.pas) + 1
ix = np.clip(((x - x0) / a.pas).round().astype(np.int64), 0, nx - 1)
iy = np.clip(((y1 - y) / a.pas).round().astype(np.int64), 0, ny - 1)
cell = iy * nx + ix
order = np.lexsort((z, cell))
cs, zs = cell[order], z[order]
starts = np.flatnonzero(np.r_[True, cs[1:] != cs[:-1]])
ends = np.r_[starts[1:], len(cs)]
h = np.zeros(nx * ny, np.float32)
h[cs[starts]] = zs[(starts + ends) // 2]
m = np.zeros(nx * ny, bool)
m[cs[starts]] = True
h, m = fill(h.reshape(ny, nx), m.reshape(ny, nx), 30)
# Zone utile : plus grande zone couverte, trous interieurs bouches
n_lab, lab, stats, _ = cv2.connectedComponentsWithStats(m.astype(np.uint8))
m = lab == 1 + np.argmax(stats[1:, cv2.CC_STAT_AREA])
# petits trous interieurs bouches (< 1 ha) ; une dalle manquante reste hors du mesh
n_hole, hole, hstats, _ = cv2.connectedComponentsWithStats((~m).astype(np.uint8), connectivity=4)
small = np.isin(hole, 1 + np.flatnonzero(hstats[1:, cv2.CC_STAT_AREA] * a.pas * a.pas < 10000))
m |= small
h, _ = fill(h, ~small & m, 60)
# ouverture de 100 m : les bandes etroites (autour d'une dalle manquante) sortent du mesh, sinon elles traversent Google
o = int(round(50 / a.pas))
m = cv2.morphologyEx(m.astype(np.uint8), cv2.MORPH_OPEN, cv2.getStructuringElement(cv2.MORPH_RECT, (2 * o + 1, 2 * o + 1)),
                     borderType=cv2.BORDER_CONSTANT, borderValue=0).astype(bool)
print(f'{len(x):,} points, grille {nx} x {ny} au pas de {a.pas} m, {m.mean() * 100:.1f} % couverte')

# Points aberrants (oiseaux, bruit en bord de dalle) : cellule a plus de 20 m de la mediane de ses voisines
med = cv2.medianBlur(h.astype(np.float32), 5)
spikes = np.abs(h - med) > 20.0
h = np.where(spikes, med, h)
print(f'{int(spikes.sum())} cellules aberrantes corrigees')

east = x0 + np.arange(nx) * a.pas
north = y1 - np.arange(ny) * a.pas
E, N = np.meshgrid(east, north)

# --- Passage sous le mesh detaille
if a.dessous:
    v = np.array([list(map(float, l.split()[1:4])) for l in open(a.dessous) if l.startswith('v ')])
    ve, vn, vh = origin[0] + v[:, 0] / 100, origin[1] + v[:, 1] / 100, origin[2] + v[:, 2] / 100
    jx = ((ve - x0) / a.pas).round().astype(np.int64)
    jy = ((y1 - vn) / a.pas).round().astype(np.int64)
    ok = (jx >= 0) & (jx < nx) & (jy >= 0) & (jy < ny)
    hd = np.full((ny, nx), np.inf, np.float32)
    np.minimum.at(hd, (jy[ok], jx[ok]), vh[ok])
    inside = np.isfinite(hd)
    # erosion d'une cellule : au bord du mesh detaille, le terrain reprend son altitude LiDAR
    core = inside.copy()
    for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        core &= np.roll(np.roll(inside, dy, 0), dx, 1)
    h = np.where(core, np.minimum(h, hd) - a.retrait, h)
    print(f'{core.sum():,} cellules sous le mesh detaille')

# --- Sous le socle rocheux (OBJ recentre, meme origine) : le terrain descend sous la roche, visible a travers la glace
if a.socle:
    v = np.array([list(map(float, l.split()[1:4])) for l in open(a.socle) if l.startswith('v ')])
    se, sn, sh = origin[0] + v[:, 0] / 100, origin[1] + v[:, 1] / 100, origin[2] + v[:, 2] / 100
    near = np.zeros((ny, nx), bool)
    jx = ((se - x0) / a.pas).round().astype(np.int64)
    jy = ((y1 - sn) / a.pas).round().astype(np.int64)
    r = int(np.ceil(a.marge_socle / a.pas))
    for dy in range(-r, r + 1):
        for dx in range(-r, r + 1):
            if dx * dx + dy * dy <= r * r:
                ok = (jx + dx >= 0) & (jx + dx < nx) & (jy + dy >= 0) & (jy + dy < ny)
                near[jy[ok] + dy, jx[ok] + dx] = True
    h = np.where(near, np.minimum(h, sh.min() - 3.0), h)
    print(f'{near.sum():,} cellules sous le socle')

# --- Plongee du bord sous Google (distance au bord de la zone utile)
border = np.pad(m, 1).astype(np.uint8)
dist = cv2.distanceTransform(border, cv2.DIST_L2, 5)[1:-1, 1:-1] * a.pas
w = np.clip(1 - dist / a.bande, 0, 1)
h = h - a.plongee * w * w

# --- Orthophoto IGN (WMS, tuiles de 2048 px)
tw, th = a.texture
tiles_x, tiles_y = int(np.ceil(tw / 2048)), int(np.ceil(th / 2048))
mosaic = Image.new('RGB', (tiles_x * 2048, tiles_y * 2048))
sx, sy = (x1 - x0) / tw, (y1 - y0) / th
for ty in range(tiles_y if not pathlib.Path(a.sortie + '.png').exists() else 0):
    for tx in range(tiles_x):
        bx0, by1 = x0 + tx * 2048 * sx, y1 - ty * 2048 * sy
        bbox = f'{bx0},{by1 - 2048 * sy},{bx0 + 2048 * sx},{by1}'
        url = ('https://data.geopf.fr/wms-r?SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap&LAYERS=ORTHOIMAGERY.ORTHOPHOTOS'
               f'&STYLES=&CRS=EPSG:2154&BBOX={bbox}&WIDTH=2048&HEIGHT=2048&FORMAT=image/jpeg')
        for attempt in range(4):
            try:
                data = urllib.request.urlopen(url, timeout=120).read()
                break
            except Exception as ex:
                print('nouvel essai', tx, ty, ex)
        mosaic.paste(Image.open(io.BytesIO(data)).convert('RGB'), (tx * 2048, ty * 2048))
        print('tuile', tx, ty)
if not pathlib.Path(a.sortie + '.png').exists():
    mosaic.crop((0, 0, tw, th)).save(a.sortie + '.png')

# --- OBJ
out = pathlib.Path(a.sortie)
with open(out.with_suffix('.obj'), 'w', newline='\n') as f:
    f.write(f'# {pathlib.Path(a.source).name} : origine Lambert-93 {origin[0]:.0f} {origin[1]:.0f} {origin[2]:.0f} (m)\n')
    f.write('# Unites cm, Z haut (x = est, y = nord, z = altitude)\n')
    f.write(f'mtllib {out.name}.mtl\nusemtl {out.name}\n')
    vx = ((E - origin[0]) * 100).ravel()
    vy = ((N - origin[1]) * 100).ravel()
    vz = ((h - origin[2]) * 100).ravel()
    f.write(''.join(f'v {a_:.1f} {b_:.1f} {c_:.1f}\n' for a_, b_, c_ in zip(vx, vy, vz)))
    u = ((E - x0) / (x1 - x0)).ravel()
    t = ((N - y0) / (y1 - y0)).ravel()
    f.write(''.join(f'vt {a_:.6f} {b_:.6f}\n' for a_, b_ in zip(u, t)))
    idx = np.arange(nx * ny).reshape(ny, nx) + 1
    # seuls les carres entierement dans la zone utile
    quad = (m[:-1, :-1] & m[:-1, 1:] & m[1:, :-1] & m[1:, 1:]).ravel()
    a00, a01, a10, a11 = (q.ravel()[quad] for q in (idx[:-1, :-1], idx[:-1, 1:], idx[1:, :-1], idx[1:, 1:]))
    f.write(''.join(f'f {q0}/{q0} {q2}/{q2} {q1}/{q1}\nf {q1}/{q1} {q2}/{q2} {q3}/{q3}\n'
                    for q0, q1, q2, q3 in zip(a00, a01, a10, a11)))
with open(str(out) + '.mtl', 'w', newline='\n') as f:
    f.write(f'newmtl {out.name}\nKd 1 1 1\nmap_Kd {out.name}.png\n')

# --- Contour de decoupe Google : zone utile rentree de --decoupe m, pour ClipOutlineLambert93
k = int(round(a.decoupe / a.pas))
# ouverture de 100 m : les bandes etroites autour d'une dalle manquante sortent de la decoupe (un polygone sans trou)
o = int(round(50 / a.pas))
clip = cv2.morphologyEx(m.astype(np.uint8), cv2.MORPH_OPEN, cv2.getStructuringElement(cv2.MORPH_RECT, (2 * o + 1, 2 * o + 1)),
                        borderType=cv2.BORDER_CONSTANT, borderValue=0)
clip = np.pad(clip, 1)
inner = cv2.erode(clip, cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (2 * k + 1, 2 * k + 1)))[1:-1, 1:-1]
cs_, _ = cv2.findContours(inner, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
c = cv2.approxPolyDP(max(cs_, key=cv2.contourArea), 1.0, True)[:, 0, :]
json.dump([{'X': float(x0 + px * a.pas), 'Y': float(y1 - py * a.pas)} for px, py in c], open(a.sortie + '_decoupe.json', 'w'))
print(f'{nx * ny:,} sommets, {2 * int(quad.sum()):,} triangles, contour de decoupe {len(c)} points -> {out.with_suffix(".obj")}')
