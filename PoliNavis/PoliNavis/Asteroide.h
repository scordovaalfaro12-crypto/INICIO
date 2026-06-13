#pragma once
#include "Obstaculo.h"

namespace PoliNavis {

	// Asteroide del Nivel 1: roca irregular (poligono de 9 vertices) que gira.
	// Cada uno tiene una silueta unica generada al azar al crearse.
	public ref class Asteroide : public Obstaculo {
	public:
		Asteroide(float px, float py, float pdx, float pdy, float pradio);
		virtual void dibujar(Graphics^ g) override;
	private:
		array<float>^ forma;
		Color color;
	};
}
