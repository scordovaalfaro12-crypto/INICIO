#pragma once
//==============================================================================
//  Enemigo2.h  -  Villano de movimiento VERTICAL (hereda de Enemigo)
//------------------------------------------------------------------------------
//  Aparece en la zona DERECHA con posicion y velocidad aleatorias. Rebota en
//  los bordes superior/inferior de la pantalla.
//==============================================================================
#include "Enemigo.h"

namespace LB2_Bathero {

	ref class Enemigo2 : public Enemigo {
	private:
		int vy; // Velocidad vertical (con signo)

	public:
		Enemigo2(int w, int h, int mitad, Random^ azar);
		virtual void Actualizar() override;
	};
}
