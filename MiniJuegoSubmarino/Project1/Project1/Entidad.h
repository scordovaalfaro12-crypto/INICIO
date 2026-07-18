#pragma once
// ============================================================
//  Entidad.h
//  Clase PADRE de todas las entidades del juego.
//  Aporta las propiedades comunes que heredan las clases hijas:
//    - Tamanio   (ancho, alto)
//    - Vida      (vida)
//    - Movimiento(x, y, velocidad, metodo virtual Mover)
// ============================================================

namespace Project1 {

	using namespace System;
	using namespace System::Drawing;

	// Direcciones posibles de una entidad dentro del juego
	public enum class Direccion : int
	{
		Izquierda = 0,
		Derecha   = 1,
		Arriba    = 2,
		Abajo     = 3
	};

	public ref class Entidad abstract
	{
	protected:
		// ----- Movimiento -----
		float x;             // posicion horizontal
		float y;             // posicion vertical
		float velocidad;     // pixeles que avanza por ciclo

		// ----- Tamanio -----
		int ancho;           // ancho con el que se dibuja
		int alto;            // alto con el que se dibuja

		// ----- Vida -----
		int vida;            // cantidad de vidas de la entidad
		bool activo;         // false = ya no participa en el juego

		// ----- Sprite (hoja de imagenes) -----
		Bitmap^ hoja;        // hoja de sprites completa
		int columnas;        // columnas de la hoja
		int filas;           // filas de la hoja
		int frameAncho;      // ancho de un fotograma dentro de la hoja
		int frameAlto;       // alto de un fotograma dentro de la hoja
		int frameActual;     // fotograma que se esta mostrando
		int fila;            // fila de la hoja que se usa (segun direccion)
		int contadorAnim;    // contador para regular la animacion
		int retardoAnim;     // cada cuantos ciclos cambia de fotograma

	public:
		Entidad(Bitmap^ hoja, int columnas, int filas,
			float x, float y, int ancho, int alto,
			int vida, float velocidad);

		// Cada clase hija define SU propia forma de moverse
		virtual void Mover(int anchoVentana, int altoVentana) abstract;

		// Dibujo y animacion comunes (las hijas pueden redefinirlos)
		virtual void Dibujar(Graphics^ g);
		virtual void Animar();

		// Colisiones
		Rectangle ObtenerRectangulo();
		bool ColisionaCon(Entidad^ otra);

		// Vida
		void PerderVida();
		bool EstaVivo();

		// ----- Propiedades de acceso -----
		property float X { float get(); void set(float valor); }
		property float Y { float get(); void set(float valor); }
		property int Ancho { int get(); }
		property int Alto { int get(); }
		property int Vida { int get(); void set(int valor); }
		property float Velocidad { float get(); void set(float valor); }
		property bool Activo { bool get(); void set(bool valor); }
		property float CentroX { float get(); }
		property float CentroY { float get(); }
	};
}
