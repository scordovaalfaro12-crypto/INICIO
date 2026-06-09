#pragma once
#include "EntidadEspacial.h"

namespace JuegoEspacial {

	ref class Astronauta;     // declaracion adelantada (el efecto se aplica al astronauta)

	// Item recolectable (abstracto en el UML): tiene un valor y aplica un efecto.
	// De aqui heredan Estrella y Artefacto.
	public ref class Item abstract : public EntidadEspacial {
	protected:
		int valor;

	public:
		Item(float px, float py, int pvalor);

		virtual void aplicarEfecto(Astronauta^ astronauta) = 0;
		virtual RectangleF zona() = 0;     // rectangulo de colision

		int getValor();
	};
}
