#pragma once
#include "Entrada.h"
#include "Astronauta.h"
#include "Particula.h"

namespace JuegoEspacial {

	// Clase base de los 8 minijuegos (uno por planeta).
	// Cada minijuego define como iniciar, actualizar y dibujarse.
	public ref class Minijuego abstract {
	protected:
		Astronauta^ jugador;
		int estadoInterno;       // 0 = jugando, 1 = ganado, 2 = perdido
		int ticks;
		String^ titulo;
		String^ instrucciones;
		int puntajeLocal;        // puntos conseguidos en este intento
		int planeta;
		bool usaVidas;           // si el HUD muestra los corazones
		List<Particula^>^ particulas;

	public:
		Minijuego(Astronauta^ pjugador, int pplaneta, String^ ptitulo, String^ pinstrucciones);

		virtual void iniciar() = 0;
		virtual void actualizar(Entrada^ entrada) = 0;
		virtual void dibujar(Graphics^ g) = 0;
		virtual void clic(int mx, int my);
		virtual String^ progreso() = 0;          // texto del objetivo para el HUD
		virtual void dibujarHUD(Graphics^ g);

		bool jugando();
		bool ganado();
		bool perdido();
		String^ getTitulo();
		String^ getInstrucciones();
		int getPuntajeLocal();
		int getPlaneta();

	protected:
		void ganar();
		void perder();
		void sumar(int cantidad);                // suma puntos al intento y al astronauta
		void tictac();                           // avanza el reloj y las particulas
		void dibujarCorazones(Graphics^ g);
	};
}
