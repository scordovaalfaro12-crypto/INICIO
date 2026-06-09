#pragma once
using namespace System;
using namespace System::Drawing;

namespace JuegoEspacial {

	// Clase base abstracta de todos los objetos del juego (segun el UML):
	// tiene una posicion (x, y) y sabe moverse y dibujarse.
	public ref class EntidadEspacial abstract {
	protected:
		float x;
		float y;

	public:
		EntidadEspacial(float px, float py);

		virtual void mover() = 0;
		virtual void dibujar(Graphics^ g) = 0;

		float getX();
		float getY();
		void setPosicion(float px, float py);
	};
}
