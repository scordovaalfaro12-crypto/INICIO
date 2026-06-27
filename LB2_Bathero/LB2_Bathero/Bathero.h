#pragma once
//==============================================================================
//  Bathero.h  -  El heroe controlado por el jugador (hereda de Sprite)
//------------------------------------------------------------------------------
//  Se mueve SOLO por la mitad izquierda de la pantalla usando las flechas.
//  Hoja "Bathero.png": 3 columnas (cuadros) x 4 filas (direcciones).
//==============================================================================
#include "Sprite.h"

namespace LB2_Bathero {

	ref class Bathero : public Sprite {
	private:
		float xIni, yIni;    // Posicion inicial
		int velocidad;       // Pixeles por movimiento
		bool enMovimiento;

	public:
		Bathero(int areaAlto) {
			hoja = Util::CargarImagen("Bathero.png");
			colsHoja = 3; filasHoja = 4; colOffset = 0; numFrames = 3;
			ancho = 54; alto = 64;
			velocidad = 7;
			xIni = 40.0f;
			yIni = (float)(areaAlto / 2 - alto / 2);
			x = xIni; y = yIni;
			fila = 0; frame = 1; enMovimiento = false;
		}

		property float XIni { float get() { return xIni; } }
		property float YIni { float get() { return yIni; } }

		// Mueve a Bathero segun las teclas mantenidas, sin salir de su zona.
		void Mover(bool arriba, bool abajo, bool izq, bool der, int limX, int limY) {
			enMovimiento = false;
			if (izq) { x -= velocidad; fila = 1; enMovimiento = true; }
			if (der) { x += velocidad; fila = 2; enMovimiento = true; }
			if (arriba) { y -= velocidad; fila = 3; enMovimiento = true; }
			if (abajo) { y += velocidad; fila = 0; enMovimiento = true; }

			// Limites: queda confinado a la mitad izquierda de la ventana.
			if (x < 0) x = 0;
			if (x > limX) x = (float)limX;
			if (y < 0) y = 0;
			if (y > limY) y = (float)limY;

			if (enMovimiento) Animar(6);
			else frame = 1; // cuadro "quieto"
		}
	};
}
