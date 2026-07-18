#include "Tiburon.h"
// ============================================================
//  Tiburon.cpp
// ============================================================

namespace Project1 {

	Tiburon::Tiburon(Bitmap^ hoja, float x, float y, float velocidad,
		bool haciaLaDerecha)
		: Enemigo(hoja, 4, 2, x, y,
			112, 82,              /* tamanio del tiburon */
			1,                    /* una colision lo elimina */
			haciaLaDerecha ? velocidad : -velocidad)
	{
		// fila 0: fotogramas mirando a la izquierda
		// fila 1: fotogramas mirando a la derecha
		fila = haciaLaDerecha ? 1 : 0;
	}
}
