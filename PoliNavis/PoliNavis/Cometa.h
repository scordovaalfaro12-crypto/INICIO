#pragma once
#include "Obstaculo.h"

namespace PoliNavis {

	// Cometa: nucleo brillante + cola que apunta al lado contrario de su
	// movimiento. Hecho con circulos y poligonos. Con 'neon' = true usa
	// colores fosforescentes para los cometas mas veloces del Nivel 2.
	public ref class Cometa : public Obstaculo {
	public:
		Cometa(float px, float py, float pdx, float pdy, float pradio, bool neon);
		virtual void dibujar(Graphics^ g) override;
	private:
		Color color;
		bool esNeon;
	};
}
