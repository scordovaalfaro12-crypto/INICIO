#pragma once

namespace PoliNavis {

	// Estado del teclado y del raton que el controlador comparte con el juego.
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

		Entrada();
		void reiniciar();   // pone todas las teclas en false
	};
}
