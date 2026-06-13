#pragma once
using namespace System;

namespace PoliNavis {

	// Generador central de numeros aleatorios para todo el juego.
	// La implementacion vive en Azar.cpp.
	public ref class Azar abstract sealed {
	public:
		static Random^ generador;

		static int entre(int minimo, int maximoExcluido);  // entero en [min, maxExcluido)
		static float real();                                // real en [0, 1)
		static float entreF(float a, float b);              // real en [a, b)
		static bool moneda();                               // true/false al 50%
	};
}
