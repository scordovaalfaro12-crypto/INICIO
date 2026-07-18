#pragma once
// ============================================================
//  Pez.h
//  Clase HIJA de Enemigo (nieta de Entidad).
//  Aparece con la tecla P (entre 3 y 5 peces) y se mueve de
//  derecha a izquierda mas rapido que el submarino.
// ============================================================
#include "Enemigo.h"

namespace Project1 {

	public ref class Pez : public Enemigo
	{
	public:
		Pez(Bitmap^ hoja, float x, float y, float velocidad);
	};
}
