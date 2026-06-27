#pragma once
//==============================================================================
//  Enemigo1.h  -  Villano de movimiento HORIZONTAL (hereda de Enemigo)
//------------------------------------------------------------------------------
//  Aparece en la zona DERECHA con posicion y velocidad aleatorias. Rebota en
//  los bordes izquierdo/derecho de su zona.
//==============================================================================
#include "Enemigo.h"

namespace LB2_Bathero {

	ref class Enemigo1 : public Enemigo {
	private:
		int vx; // Velocidad horizontal (con signo)

	public:
		Enemigo1(int w, int h, int mitad, Random^ azar);
		virtual void Actualizar() override;
	};
}
