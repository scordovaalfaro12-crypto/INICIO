#pragma once
using namespace System;
using namespace System::Drawing;

namespace JuegoEspacial {

	// Dibuja el escenario de cada planeta: cielo con degradado,
	// decoracion caracteristica y el suelo con rocas y crateres.
	public ref class Superficie abstract sealed {
	public:
		static void dibujarFondo(Graphics^ g, int planeta, int tick);

		static Color colorCieloArriba(int planeta);
		static Color colorCieloAbajo(int planeta);
		static Color colorSuelo(int planeta);
		static Color colorSueloOscuro(int planeta);

	private:
		static array<float>^ rocasX;
		static array<float>^ rocasTam;
		static void asegurarDetalles();
		static void dibujarDecoracion(Graphics^ g, int planeta, int tick);
		static void dibujarSuelo(Graphics^ g, int planeta, int tick);
	};
}
