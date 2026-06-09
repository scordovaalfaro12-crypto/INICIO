#pragma once
using namespace System;
using namespace System::Drawing;

namespace JuegoEspacial {

	// Pantalla del mapa: el Sol y los 8 planetas, cada uno con su mision.
	// Tambien dibuja cualquier planeta en miniatura (lo reusan otras pantallas).
	public ref class SistemaSolar {
	public:
		SistemaSolar();

		void dibujar(Graphics^ g, int tick, array<bool>^ completados, int puntaje, String^ nombreJugador, Color colorJugador);
		int planetaEn(int mx, int my);              // indice del planeta bajo el raton (-1 si nada)
		bool moverRaton(int mx, int my);            // true si cambio el planeta marcado
		int getMarcado();
		void marcarConTeclado(int direccion);       // izquierda (-1) / derecha (+1)
		bool desbloqueado(int indice, array<bool>^ completados);

		static String^ nombre(int indice);
		static String^ nombreMision(int indice);
		static void dibujarPlaneta(Graphics^ g, int indice, float cx, float cy, float radio, int tick);

	private:
		array<float>^ px;
		array<float>^ py;
		array<float>^ radios;
		int marcado;
	};
}
