#pragma once
using namespace System;

namespace JuegoEspacial {

	// Generador central de numeros aleatorios para todo el juego
	public ref class Azar abstract sealed {
	public:
		static Random^ generador = gcnew Random();

		// entero en [minimo, maximoExcluido)
		static int entre(int minimo, int maximoExcluido) {
			return generador->Next(minimo, maximoExcluido);
		}

		// real en [0, 1)
		static float real() {
			return (float)generador->NextDouble();
		}

		// real en [a, b)
		static float entreF(float a, float b) {
			return a + (b - a) * real();
		}

		static bool moneda() {
			return generador->Next(2) == 0;
		}
	};
}
