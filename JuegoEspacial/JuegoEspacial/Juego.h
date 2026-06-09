#pragma once
#include "MenuJuego.h"
#include "SistemaSolar.h"
#include "Aterrizaje.h"
#include "Minijuego.h"
#include "Astronauta.h"
#include "Entrada.h"
#include "Particula.h"

namespace JuegoEspacial {

	// Clase principal (segun el UML): controla el nivel, el estado actual
	// y coordina los menus, el mapa, las cinematicas y los 8 minijuegos.
	// Todo ocurre dentro de la misma ventana.
	public ref class Juego {
	public:
		Juego();

		void iniciar();       // comienza la aventura (va al sistema solar)
		void actualizar();    // un tick del juego (lo llama el temporizador)
		void finalizar();     // se completo la galaxia (pantalla final)
		void dibujar(Graphics^ g);

		// eventos que reenvia el formulario
		void teclaAbajo(System::Windows::Forms::Keys tecla);
		void teclaArriba(System::Windows::Forms::Keys tecla);
		void ratonMovido(int mx, int my);
		void clic(int mx, int my);

		String^ getEstado();
		int getNivel();

	private:
		int nivel;                // misiones completadas (0..8)
		String^ estado;           // MENU, PERSONAJES, CONTROLES, CREDITOS, SISTEMA,
		                          // ATERRIZAJE, MINIJUEGO, DESPEGUE, FINAL
		int tick;
		int tickEstado;           // ticks desde el ultimo cambio de pantalla

		MenuJuego^ menu;
		SistemaSolar^ sistema;
		Aterrizaje^ transicion;
		Minijuego^ minijuego;
		Astronauta^ jugador;
		Entrada^ entrada;
		array<bool>^ completados;
		List<Particula^>^ fiesta;     // fuegos artificiales del final

		int personajeSel;
		int planetaActual;
		int subMini;              // 0 = intro, 1 = jugando, 2 = resultado
		bool ganadoUltimo;
		bool bonusDado;

		void cambiarEstado(String^ nuevo);
		void volverAlMenu();
		void ejecutarAccion(AccionMenu accion);
		void elegirPersonaje(int indice);
		void entrarPlaneta(int indice);
		void irADespegue();
		void terminarMinijuego();
		Minijuego^ crearMinijuego(int indice);

		void dibujarIntro(Graphics^ g);
		void dibujarResultado(Graphics^ g);
		void dibujarFinal(Graphics^ g);
	};
}
