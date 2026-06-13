#pragma once
#include "Entidad.h"

namespace PoliNavis {

	// ===== CLASE ABSTRACTA Obstaculo (hereda de Entidad) =====
	// Base de todo lo que la nave debe esquivar. Se mueve en linea recta con
	// la velocidad (dx, dy) que le asigna el spawner (trayectoria aleatoria,
	// mas rapida que la nave). De aqui heredan Cometa, Asteroide y FiguraKhaos.
	public ref class Obstaculo abstract : public Entidad {
	protected:
		float radio;
		float giro;          // rotacion propia (visual)
		float velGiro;

	public:
		Obstaculo(float px, float py, float pdx, float pdy, float pradio);

		virtual void mover() override;
		virtual RectangleF area() override;

		bool fueraDePantalla();   // salio del area con margen => se elimina
		float getRadio();
	};
}
