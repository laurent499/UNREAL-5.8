"""Masque de l'emprise du socle dans l'espace de texture d'un mesh issu de laz_to_mesh.py (meme emprise que son nuage).

Blanc sur le socle, fondu sur --flou m : le materiau de la glace y devient translucide pendant le decalottage.
Usage : python -I masque_socle.py socle.obj nuage.laz masque.png [--taille 1024] [--flou 6]
(socle.obj en coordonnees Lambert-93 absolues, comme livre par les geometres)
"""
import argparse

import laspy
from PIL import Image, ImageDraw, ImageFilter

p = argparse.ArgumentParser()
p.add_argument('socle')
p.add_argument('nuage')
p.add_argument('sortie')
p.add_argument('--taille', type=int, default=1024)
p.add_argument('--flou', type=float, default=6.0)
a = p.parse_args()

h = laspy.open(a.nuage).header
x0, y0, x1, y1 = h.mins[0], h.mins[1], h.maxs[0], h.maxs[1]
s = a.taille
verts, faces = [], []
for l in open(a.socle, encoding='latin-1'):
    t = l.split()
    if t and t[0] == 'v':
        verts.append((float(t[1]), float(t[2])))
    elif t and t[0] == 'f':
        faces.append([int(k.split('/')[0]) - 1 for k in t[1:]])
img = Image.new('L', (s, s), 0)
d = ImageDraw.Draw(img)
px = lambda e, n: ((e - x0) / (x1 - x0) * (s - 1), (y1 - n) / (y1 - y0) * (s - 1))
for f in faces:
    d.polygon([px(*verts[i]) for i in f], fill=255)
img = img.filter(ImageFilter.GaussianBlur(a.flou / (x1 - x0) * s))
img.save(a.sortie)
print('masque', a.sortie, img.getbbox())
