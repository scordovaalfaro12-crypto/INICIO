#pragma once
#include "Minijuego.h"
#include "Nave.h"

namespace JuegoEspacial {

	// URANO - "Cristales de memoria": un juego de Simon dentro de una
	// caverna helada. Observa la secuencia de luces y repitela con el raton.
	public ref class MinijuegoUrano : public Minijuego {
	public:
		MinijuegoUrano(Astronauta^ pjugador);

		virtual void iniciar() override;
		virtual void actualizar(Entrada^ entrada) override;
		virtual void dibujar(Graphics^ g) override;
		virtual void clic(int mx, int my) override;
		virtual String^ progreso() override;

	private:
		literal int RONDA_FINAL = 6;

		List<int>^ secuencia;
		Nave^ naveEstacionada;
		int faseSimon;        // 0 = mostrando, 1 = esperando al jugador, 2 = pausa
		int temporizador;
		int mostrando;        // que paso de la secuencia se esta ensenando
		int paso;             // que paso espera del jugador
		int errores;
		int iluminado;        // cristal encendido (-1 = ninguno)
		int iluminadoTicks;

		array<float>^ cx;     // centros de los 4 cristales
		array<float>^ cy;

		void encender(int indice);
		int cristalEn(int mx, int my);
		Color colorCristal(int indice);
		void dibujarCristal(Graphics^ g, int indice, bool encendido);
	};
}
