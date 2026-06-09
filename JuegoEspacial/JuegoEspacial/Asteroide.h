#pragma once
#include "Enemigo.h"

namespace JuegoEspacial {

	// Roca espacial irregular que gira mientras se mueve
	public ref class Asteroide : public Enemigo {
	public:
		Asteroide(float px, float py, float pvx, float pvy, float pradio, Color pcolor);

		virtual void mover() override;
		virtual void atacar() override;
		virtual void dibujar(Graphics^ g) override;
		virtual RectangleF zona() override;

		void rebotar(float minX, float minY, float maxX, float maxY);   // rebote en los bordes
		float getRadio();

	private:
		float vx, vy;
		float radio;
		float angulo;
		float velocidadGiro;
		Color colorBase;
		array<float>^ forma;     // radio relativo de cada uno de los 8 vertices
	};
}
