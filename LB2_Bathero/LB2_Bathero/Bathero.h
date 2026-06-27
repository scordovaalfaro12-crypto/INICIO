#pragma once
//==============================================================================
//  Bathero.h  -  Declaracion del heroe controlado por el jugador
//------------------------------------------------------------------------------
//  Se mueve SOLO por la mitad izquierda usando las flechas. Hoja "Bathero.png":
//  3 columnas (cuadros) x 4 filas (direcciones).
//==============================================================================
#include "Sprite.h"

namespace LB2_Bathero {

	ref class Bathero : public Sprite {
	private:
		float xIni, yIni;    // Posicion inicial
		int velocidad;       // Pixeles por movimiento
		bool enMovimiento;

	public:
		Bathero(int areaAlto);

		property float XIni { float get() { return xIni; } }
		property float YIni { float get() { return yIni; } }

		// Mueve a Bathero segun las teclas mantenidas, sin salir de su zona.
		void Mover(bool arriba, bool abajo, bool izq, bool der, int limX, int limY);
	};
}
