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
		Enemigo1(int w, int h, int mitad, Random^ azar) : Enemigo(w, h, mitad, azar) {
			colOffset = 0; // primer personaje de la hoja
			// Posicion aleatoria dentro de la mitad derecha.
			x = (float)(mitad + 10 + azar->Next(0, Math::Max(1, w - mitad - ancho - 20)));
			y = (float)(10 + azar->Next(0, Math::Max(1, h - alto - 20)));
			int v = 3 + azar->Next(0, 6); // velocidad aleatoria 3..8
			vx = (azar->Next(0, 2) == 0) ? -v : v;
			fila = (vx > 0) ? 2 : 1; // mira a la derecha / izquierda
		}

		// Movimiento horizontal con rebote (polimorfismo: sobreescribe Actualizar).
		virtual void Actualizar() override {
			x += vx;
			if (x < mitad + 5) { x = (float)(mitad + 5); vx = -vx; }
			if (x > areaW - ancho - 5) { x = (float)(areaW - ancho - 5); vx = -vx; }
			fila = (vx > 0) ? 2 : 1;
			Animar(7);
		}
	};
}
