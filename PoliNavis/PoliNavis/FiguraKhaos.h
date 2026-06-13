#pragma once
#include "Obstaculo.h"

namespace PoliNavis {

	// Cuerpo de la "Galaxia Nebulosa del Khaos" (Nivel 2). Es un poligono de
	// 3 a 8 lados (o rectangulo) con colores neon, que se mueve de forma
	// CAOTICA: rebota en los bordes y cambia de direccion al azar. Mas
	// impredecible y veloz que los obstaculos del Nivel 1.
	public ref class FiguraKhaos : public Obstaculo {
	public:
		FiguraKhaos(float px, float py, float pdx, float pdy, float pradio, int plados);
		virtual void mover() override;       // movimiento caotico con rebote
		virtual void dibujar(Graphics^ g) override;
	private:
		int lados;
		Color color;
		int caos;        // ticks para el proximo cambio brusco de rumbo
	};
}
