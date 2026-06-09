#pragma once
#include "EntidadEspacial.h"

namespace JuegoEspacial {

	// Enemigo generico (abstracto en el UML): tiene vida y sabe atacar.
	// De aqui heredan Asteroide, BasuraEspacial y JefeFinal.
	public ref class Enemigo abstract : public EntidadEspacial {
	protected:
		int vida;
		int destello;     // ticks de resplandor al atacar o recibir un golpe

	public:
		Enemigo(float px, float py, int pvida);

		virtual void atacar() = 0;
		virtual RectangleF zona() = 0;     // rectangulo de colision

		void recibirGolpe(int danio);
		int getVida();
		bool destruido();
	};
}
