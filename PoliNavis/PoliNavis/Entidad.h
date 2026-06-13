#pragma once
using namespace System;
using namespace System::Drawing;

namespace PoliNavis {

	// ===== CLASE BASE ABSTRACTA (raiz de toda la jerarquia POO) =====
	// Demuestra ABSTRACCION y ENCAPSULAMIENTO: todo objeto del juego tiene
	// una posicion (x, y) y una velocidad (dx, dy), sabe moverse y dibujarse.
	// De aqui heredan Nave, Sol, Planeta, Obstaculo y Estacion.
	public ref class Entidad abstract {
	protected:
		float x, y;       // posicion (encapsuladas: solo se tocan con get/set)
		float dx, dy;     // velocidad por tick

	public:
		Entidad(float px, float py);

		// metodos abstractos: cada hija decide COMO se mueve y se dibuja
		virtual void mover() = 0;
		virtual void dibujar(Graphics^ g) = 0;

		// rectangulo de colision (las hijas lo ajustan a su forma)
		virtual RectangleF area();

		float getX();
		float getY();
		void setPos(float px, float py);
		void setVel(float pdx, float pdy);
	};
}
