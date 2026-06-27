//==============================================================================
//  Enemigo2.cpp  -  Implementacion del villano vertical
//==============================================================================
#include "Enemigo2.h"

namespace LB2_Bathero {

	Enemigo2::Enemigo2(int w, int h, int mitad, Random^ azar) : Enemigo(w, h, mitad, azar) {
		colOffset = 3; // segundo personaje de la hoja
		// Posicion aleatoria dentro de la mitad derecha.
		x = (float)(mitad + 10 + azar->Next(0, Math::Max(1, w - mitad - ancho - 20)));
		y = (float)(10 + azar->Next(0, Math::Max(1, h - alto - 20)));
		int v = 3 + azar->Next(0, 6); // velocidad aleatoria 3..8
		vy = (azar->Next(0, 2) == 0) ? -v : v;
		fila = (vy > 0) ? 0 : 3; // baja (abajo) / sube (arriba)
	}

	// Movimiento vertical con rebote (polimorfismo: sobreescribe Actualizar).
	void Enemigo2::Actualizar() {
		y += vy;
		if (y < 5) { y = 5; vy = -vy; }
		if (y > areaH - alto - 5) { y = (float)(areaH - alto - 5); vy = -vy; }
		fila = (vy > 0) ? 0 : 3;
		Animar(7);
	}
}
