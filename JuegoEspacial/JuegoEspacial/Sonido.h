#pragma once
using namespace System;

namespace JuegoEspacial {

	// Efectos de sonido hechos con tonos puros (Console::Beep).
	// Se reproducen en un hilo aparte para que la ventana nunca se congele.
	public ref class Sonido abstract sealed {
	public:
		static bool activado = true;

		static void tono(int frecuencia, int duracion);
		static void mover();          // al cambiar de opcion en un menu
		static void seleccionar();    // al elegir una opcion
		static void error();          // opcion bloqueada o fallo
		static void disparo();
		static void explosion();
		static void item();           // recoger una estrella o artefacto
		static void danio();          // el jugador recibe danio
		static void victoria();
		static void derrota();
		static void notaSimon(int indice);   // las 4 notas de los cristales de Urano
		static void anillo();         // cruzar un anillo en Saturno
		static void despegueNave();
		static void aterrizajeNave();

	private:
		static bool sonando = false;
		static void lanzar(array<int>^ datos);
		static void trabajador(Object^ estado);
	};
}
