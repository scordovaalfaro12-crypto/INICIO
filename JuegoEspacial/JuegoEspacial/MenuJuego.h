#pragma once
using namespace System;
using namespace System::Drawing;

namespace JuegoEspacial {

	// Acciones que puede devolver el menu al hacer clic o presionar ENTER
	public enum class AccionMenu {
		Ninguna,
		Iniciar,
		Personajes,
		Controles,
		Creditos,
		Salir,
		Volver,
		ElegirPersonaje
	};

	// Todas las pantallas del menu en una sola ventana:
	// principal, personajes, controles y creditos.
	public ref class MenuJuego {
	public:
		MenuJuego();

		// ---- pantalla principal ----
		void dibujarPrincipal(Graphics^ g, int tick);
		AccionMenu clicPrincipal(int mx, int my);
		bool ratonPrincipal(int mx, int my);          // true si cambio la opcion marcada
		void moverSeleccion(int direccion);           // teclado: -1 arriba, +1 abajo
		AccionMenu accionSeleccionada();

		// ---- personajes ----
		void dibujarPersonajes(Graphics^ g, int tick, int seleccionado);
		AccionMenu clicPersonajes(int mx, int my);
		bool ratonPersonajes(int mx, int my);
		void moverPersonaje(int direccion);
		int getTarjetaMarcada();
		int getPersonajeClicado();

		// ---- controles y creditos ----
		void dibujarControles(Graphics^ g, int tick);
		void dibujarCreditos(Graphics^ g, int tick);
		AccionMenu clicVolver(int mx, int my);
		bool ratonVolver(int mx, int my);

		void reiniciarAnimacion();                    // al entrar a una pantalla

		// datos de los 4 personajes (tambien los usa la clase Juego)
		static array<String^>^ nombres = gcnew array<String^> {
			L"BRYAN", L"LENIN", L"GUSTAVO", L"MUSTAFA"
		};
		static array<String^>^ titulos = gcnew array<String^> {
			L"Comandante", L"Ingeniero", L"Científico", L"Explorador"
		};
		static array<String^>^ descripciones = gcnew array<String^> {
			L"Comandante de la misión. Piloto valiente que dirige a toda la tripulación.",
			L"Ingeniero de vuelo. Repara la nave y diseña todos sus sistemas.",
			L"Científico de a bordo. Estudia los planetas y los artefactos alienígenas.",
			L"Explorador estelar. Experto en terrenos desconocidos y hostiles."
		};
		static Color colorPersonaje(int indice);

	private:
		int opcion;              // boton marcado en el menu principal
		int tarjeta;             // tarjeta marcada en personajes
		int personajeClicado;
		bool volverMarcado;
		float animacion;         // 0..1 entrada suave de cada pantalla

		RectangleF rectBoton(int indice);
		RectangleF rectTarjeta(int indice);
		RectangleF rectVolver();
		void avanzarAnimacion();
		void dibujarBoton(Graphics^ g, int indice, String^ texto, int tick);
		void dibujarVolver(Graphics^ g);
		void dibujarTituloPantalla(Graphics^ g, String^ texto);
		void dibujarTecla(Graphics^ g, String^ letra, float x, float y, float ancho);
		void dibujarRaton(Graphics^ g, float x, float y);
		void dibujarCascoMini(Graphics^ g, float cx, float cy, float r, Color color);
	};
}
