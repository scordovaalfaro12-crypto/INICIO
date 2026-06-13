#include "Azar.h"

namespace PoliNavis {

	// El campo estatico 'generador' se inicializa en Azar.h (handle
	// administrado). Aqui van solo las implementaciones de los metodos.

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
