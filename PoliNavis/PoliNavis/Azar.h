#pragma once
using namespace System;

namespace PoliNavis {

	// Generador central de numeros aleatorios para todo el juego.
	// El handle estatico se inicializa AQUI (obligatorio en C++/CLI);
	// los metodos estan implementados en Azar.cpp.
	public ref class Azar abstract sealed {
	public:
		static Random^ generador = gcnew Random();

		static int entre(int minimo, int maximoExcluido);  // entero en [min, maxExcluido)
		static float real();                                // real en [0, 1)
		static float entreF(float a, float b);              // real en [a, b)
		static bool moneda();                               // true/false al 50%
	};
}
