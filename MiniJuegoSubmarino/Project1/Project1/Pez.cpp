#include "Pez.h"
// ============================================================
//  Pez.cpp
//  El pez siempre nada de derecha a izquierda, por eso su
//  velocidad se guarda en negativo.
// ============================================================

namespace Project1 {

	Pez::Pez(Bitmap^ hoja, float x, float y, float velocidad)
		: Enemigo(hoja, 4, 2, x, y,
			92, 80,               /* tamanio del pez */
			1,                    /* una colision lo elimina */
			-velocidad)           /* derecha a izquierda */
	{
		fila = 0;                 // fotogramas mirando a la izquierda
	}
}
