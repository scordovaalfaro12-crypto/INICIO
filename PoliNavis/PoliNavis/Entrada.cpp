#include "Entrada.h"

namespace PoliNavis {

	Entrada::Entrada() {
		reiniciar();
	}

	void Entrada::reiniciar() {
		izquierda = false;
		derecha = false;
		arriba = false;
		abajo = false;
		disparo = false;
		ratonX = 0;
		ratonY = 0;
	}
}
