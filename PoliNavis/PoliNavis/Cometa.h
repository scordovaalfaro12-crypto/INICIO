#pragma once
#include "Obstaculo.h"

namespace PoliNavis {

	// Cometa del Nivel 1: nucleo brillante + cola que apunta al lado
	// contrario de su movimiento. Hecho con circulos y poligonos.
	public ref class Cometa : public Obstaculo {
	public:
		Cometa(float px, float py, float pdx, float pdy, float pradio);
		virtual void dibujar(Graphics^ g) override;
	private:
		Color color;
	};
}
