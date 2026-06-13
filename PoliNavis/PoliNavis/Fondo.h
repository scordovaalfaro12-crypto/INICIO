#pragma once
using namespace System;
using namespace System::Drawing;

namespace PoliNavis {

	// Cielo espacial animado: degradado, estrellas que parpadean y estrellas fugaces
	public ref class Fondo abstract sealed {
	public:
		static void dibujar(Graphics^ g, int tick);                              // fondo completo
		static void dibujarEstrellas(Graphics^ g, int tick, int alfaMaximo);     // solo las estrellas

	private:
		literal int CANTIDAD = 130;
		static array<float>^ ex;       // posiciones x de las estrellas
		static array<float>^ ey;       // posiciones y
		static array<float>^ etam;     // tamano de cada estrella
		static array<float>^ efase;    // fase del parpadeo
		static float fugazX = 0.0f;
		static float fugazY = 0.0f;
		static float fugazVX = 0.0f;
		static float fugazVY = 0.0f;
		static int fugazVida = 0;
		static void crear();
	};
}
