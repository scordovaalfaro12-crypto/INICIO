#pragma once
#include "Menu.h"
#include "Nave.h"
#include "Sol.h"
#include "Planeta.h"
#include "Obstaculo.h"
#include "Estacion.h"
#include "Entrada.h"

using namespace System::Collections::Generic;
using namespace System::Windows::Forms;

namespace PoliNavis {

	// ===== CLASE CONTROLADORA =====
	// Integra TODAS las clases del juego (Menu, Nave, Sol, Planetas,
	// Obstaculos y Estacion), administra los estados (menu, niveles,
	// mensajes) y coordina el bucle actualizar()/dibujar() que llama MyForm.
	public ref class Juego {
	public:
		Juego();

		void actualizar();              // un tick de logica
		void dibujar(Graphics^ g);      // dibuja la pantalla activa

		void teclaAbajo(Keys tecla);
		void teclaArriba(Keys tecla);
		void clic(int mx, int my);
		void ratonMovido(int mx, int my);

		String^ getEstado();

	private:
		String^ estado;
		int tick, tickEstado;

		Menu^ menu;
		Entrada^ entrada;

		Nave^ nave;
		Sol^ sol;
		List<Planeta^>^ planetas;
		List<Obstaculo^>^ obstaculos;
		Estacion^ meta;

		int nivel;            // 1 o 2
		int colisiones;
		int ticksNivel;       // tiempo del nivel actual (en ticks)
		int tiempoNivel1;     // segundos logrados en el Nivel 1
		int spawnTimer;
		String^ datoLeyenda;  // dato educativo del planeta resaltado

		void cambiar(String^ nuevo);
		void iniciarNivel(int n);
		void actualizarNivel();
		void aplicarInput();
		void spawnObstaculoNivel1();
		void spawnObstaculoNivel2();
		void revisarColisiones();
		float inicioX();
		float inicioY();

		void dibujarFondoNivel(Graphics^ g);
		void dibujarPanel(Graphics^ g);
		void dibujarLeyenda(Graphics^ g);
		void dibujarPuntos(Graphics^ g);
		void dibujarAgujeroNegro(Graphics^ g);
		void dibujarMensaje(Graphics^ g, String^ titulo, String^ sub, String^ pie, Color color);
	};
}
