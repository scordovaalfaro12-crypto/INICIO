#include "Azar.h"

namespace PoliNavis {

	// inicializacion del generador estatico
	Random^ Azar::generador = gcnew Random();

	int Azar::entre(int minimo, int maximoExcluido) {
		return generador->Next(minimo, maximoExcluido);
	}

	float Azar::real() {
		return (float)generador->NextDouble();
	}

	float Azar::entreF(float a, float b) {
		return a + (b - a) * real();
	}

	bool Azar::moneda() {
		return generador->Next(2) == 0;
	}
}
