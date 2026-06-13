#pragma once
using namespace System::Drawing;

namespace PoliNavis {

	// Constantes globales del juego (ventana fija). Todo ocurre dentro de
	// una sola ventana de ANCHO x ALTO pixeles. Los metodos auxiliares
	// (centro y puntos A/B) estan implementados en Config.cpp.
	public ref class Config abstract sealed {
	public:
		literal int ANCHO = 1000;          // ancho del area de juego
		literal int ALTO = 700;            // alto del area de juego
		literal int TICKS_SEGUNDO = 60;    // el temporizador corre a ~60 ticks/seg

		literal int VIDAS_INICIALES = 3;
		literal int ENERGIA_INICIAL = 100;

		// Punto A (origen) y Punto B (estacion / salida)
		literal int AX = 70;
		literal int AY = 620;
		literal int BX = 905;
		literal int BY = 95;

		static int centroX();      // ANCHO / 2
		static int centroY();      // ALTO / 2
		static Point puntoA();     // origen de la nave
		static Point puntoB();     // estacion / salida
	};
}
