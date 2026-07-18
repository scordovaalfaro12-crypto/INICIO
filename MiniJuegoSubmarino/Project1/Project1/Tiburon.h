#pragma once
// ============================================================
//  Tiburon.h
//  Clase HIJA de Enemigo (nieta de Entidad).
//  Aparece con la tecla T (entre 4 y 6 tiburones), se mueve de
//  derecha a izquierda o viceversa y es mas rapido que los
//  peces y que el submarino.
// ============================================================
#include "Enemigo.h"

namespace Project1 {

	public ref class Tiburon : public Enemigo
	{
	public:
		// haciaLaDerecha = true  -> nada de izquierda a derecha
		// haciaLaDerecha = false -> nada de derecha a izquierda
		Tiburon(Bitmap^ hoja, float x, float y, float velocidad,
			bool haciaLaDerecha);
	};
}
