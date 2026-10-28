"""Recentre un OBJ en Lambert-93 (m, Z haut) pour l'import Unreal.

Les coordonnees absolues (~1 000 000 m) perdent leur precision une fois en flottants Unreal :
on retranche une origine ronde et on passe en centimetres. L'import OBJ d'Unreal 5.8 garde Z haut
et inverse Y : on ecrit (est, nord, altitude) pour obtenir X est, Y sud, Z haut dans Unreal.
Usage : python recentre_obj.py socle.obj socle_local.obj
L'origine retranchee est ecrite dans l'en-tete du fichier produit, a reporter dans
ManualLambert93Origin du Calage Lambert-93.
"""
import sys

src, dst = sys.argv[1], sys.argv[2]
lines = open(src, encoding='latin-1').read().splitlines()
verts = [tuple(map(float, l.split()[1:4])) for l in lines if l.startswith('v ')]
origin = tuple(round((min(v[i] for v in verts) + max(v[i] for v in verts)) / 2) for i in range(3))

with open(dst, 'w', encoding='latin-1', newline='\n') as out:
    out.write(f'# Recentre depuis {src} : origine Lambert-93 {origin[0]} {origin[1]} {origin[2]} (m)\n')
    out.write('# Unites cm, Z haut (x = est, y = nord, z = altitude)\n')
    for l in lines:
        if l.startswith('v '):
            e, n, h = (float(c) - o for c, o in zip(l.split()[1:4], origin))
            out.write(f'v {e * 100:.2f} {n * 100:.2f} {h * 100:.2f}\n')
        elif l.startswith('f '):
            # L'inversion de Y a l'import retourne les faces : on inverse l'ordre des sommets
            out.write('f ' + ' '.join(reversed(l.split()[1:])) + '\n')
        else:
            out.write(l + '\n')
print('origine', origin, 'sommets', len(verts))
