#pragma once
using namespace System;
using namespace System::Drawing;

namespace PoliNavis {

	// Acciones que el menu devuelve al controlador
	public enum class AccionMenu {
		Ninguna, Nivel1, Nivel2, Instrucciones, Creadores, Salir, Volver
	};

	// Menu principal + pantallas de Instrucciones y Creadores (obligatorias).
	// Todo se dibuja en la misma ventana. Devuelve una AccionMenu al hacer
	// clic; el controlador Juego decide que hacer.
	public ref class Menu {
	public:
		Menu();

		void dibujarPrincipal(Graphics^ g, int tick);
		void dibujarInstrucciones(Graphics^ g, int tick);
		void dibujarCreadores(Graphics^ g, int tick);

		AccionMenu clicPrincipal(int mx, int my);
		AccionMenu clicVolver(int mx, int my);     // boton volver en sub-pantallas
		bool hoverPrincipal(int mx, int my);       // resalta boton; true si cambio
		void moverSeleccion(int d);
		AccionMenu seleccionActual();

	private:
		int opcion;
		RectangleF rectBoton(int i);
		RectangleF rectVolver();
		void dibujarFondo(Graphics^ g, int tick, bool menu);
		void boton(Graphics^ g, int i, String^ texto, int tick);
		void botonVolver(Graphics^ g);
	};
}
