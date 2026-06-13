#pragma once

namespace PoliNavis {

	// Estado del teclado y del raton que se comparte con los minijuegos.
	// Las teclas se mantienen en true mientras esten presionadas.
	public ref class Entrada {
	public:
		bool izquierda;
		bool derecha;
		bool arriba;
		bool abajo;
		bool disparo;
		int ratonX;
		int ratonY;

		Entrada() {
			izquierda = false;
			derecha = false;
			arriba = false;
			abajo = false;
			disparo = false;
			ratonX = 0;
			ratonY = 0;
		}
	};
}
