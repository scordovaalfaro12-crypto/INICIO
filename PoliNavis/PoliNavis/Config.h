#pragma once

namespace PoliNavis {

	// Constantes globales del juego (ventana fija). Todo el juego ocurre
	// dentro de una sola ventana de ANCHO x ALTO pixeles.
	public ref class Config abstract sealed {
	public:
		literal int ANCHO = 1000;          // ancho del area de juego
		literal int ALTO = 700;            // alto del area de juego
		literal int TICKS_SEGUNDO = 60;    // el temporizador corre a ~60 ticks/seg

		// Vidas y energia de la nave
		literal int VIDAS_INICIALES = 3;
		literal int ENERGIA_INICIAL = 100;

		// Punto A (origen) y Punto B (estacion / salida).
		// La nave parte de la esquina inferior izquierda y debe llegar
		// a la estacion en la esquina superior derecha.
		literal int AX = 70;               // Punto A x
		literal int AY = 620;              // Punto A y
		literal int BX = 905;              // Punto B (estacion) x
		literal int BY = 95;               // Punto B (estacion) y
	};
}
