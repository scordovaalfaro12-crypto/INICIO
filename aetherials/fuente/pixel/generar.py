"""Genera los iconos nuevos con la estructura de Arte/ y los valida.

Uso: python3 generar.py [carpeta_salida]   (por defecto ./salida)
"""
import os
import sys

import dock
import objetos
import pixel
import revisar
from pixel import CONTORNO

SALIDA = sys.argv[1] if len(sys.argv) > 1 else 'salida'

GRUPOS = [
    # (carpeta, hoja del MANIFIESTO, módulo, ids, tamaño)
    ('Arte/iconos_objeto', 'objetos', objetos, objetos.ORDEN, 24),
    ('Arte/iconos_dock', 'dock', dock, dock.ORDEN_DOCK, 24),
    ('Arte/ui', 'tipos', dock, dock.ORDEN_AVISOS, 16),
]


def comprobar_contorno(im):
    """Todo píxel opaco que toca el exterior (vecino transparente o borde) debe ser del color de contorno."""
    n = im.width
    px = im.load()
    malos = []
    for y in range(n):
        for x in range(n):
            if px[x, y][3] == 0:
                continue
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                xx, yy = x + dx, y + dy
                fuera = not (0 <= xx < n and 0 <= yy < n) or px[xx, yy][3] == 0
                if fuera and px[x, y][:3] != CONTORNO:
                    malos.append((x, y))
                    break
    return malos


def main():
    todos = []
    for carpeta, hoja, mod, ids, n in GRUPOS:
        destino = os.path.join(SALIDA, carpeta)
        os.makedirs(destino, exist_ok=True)
        lista = []
        for id_ in ids:
            assert id_ == id_.lower() and id_.isascii(), id_
            L = getattr(mod, id_)()
            L.contorno()
            im = L.imagen()
            pixel.validar(im, n)
            malos = comprobar_contorno(im)
            assert not malos, '%s: borde sin contorno en %s' % (id_, malos[:5])
            # nada pegado al borde del lienzo: el contorno necesita su píxel
            a = im.getchannel('A')
            assert a.getbbox()[0] >= 0 and a.getbbox()[2] <= n, id_
            im.save(os.path.join(destino, id_ + '.png'))
            lista.append((id_, im))
        revisar.hoja_rejilla(lista, os.path.join(destino, '_rejilla_nuevos.png'), escala=10 if n == 24 else 14,
                             columnas=6 if n == 24 else 5)
        todos.append((hoja, lista))
        print('[OK] %-20s %2d iconos %dx%d -> %s' % (hoja, len(lista), n, n, carpeta))
    revisar.hoja_preview([x for _, l in todos[:1] for x in l], os.path.join(SALIDA, 'preview_objetos.png'))
    revisar.hoja_preview([x for _, l in todos[1:2] for x in l], os.path.join(SALIDA, 'preview_dock.png'), columnas=4)
    revisar.hoja_preview([x for _, l in todos[2:] for x in l], os.path.join(SALIDA, 'preview_avisos.png'), escala=6, columnas=5)
    total = sum(len(l) for _, l in todos)
    print('total: %d iconos nuevos, alfa binario y contorno de 1 px comprobados' % total)


if __name__ == '__main__':
    main()
