#pragma once

namespace JuegoEspacial {

	// Constantes globales del juego (la ventana tiene un tamano fijo)
	public ref class Config abstract sealed {
	public:
		literal int ANCHO = 1000;          // ancho del area de juego en pixeles
		literal int ALTO = 700;            // alto del area de juego en pixeles
		literal float SUELO = 560.0f;      // altura (y) del suelo en los planetas
		literal int TICKS_SEGUNDO = 60;    // el temporizador corre a ~60 ticks por segundo
	};
}
