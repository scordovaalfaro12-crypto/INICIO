#pragma once
//==============================================================================
//  Sprite.h  -  Declaracion de la clase BASE de la jerarquia (POO)
//------------------------------------------------------------------------------
//  Encapsula los atributos y comportamientos comunes a cualquier objeto que se
//  dibuja y anima: posicion, tamanio, hoja de sprites, recorte del cuadro de
//  animacion y deteccion de colisiones.
//==============================================================================
using namespace System;
using namespace System::Drawing;

namespace LB2_Bathero {

	ref class Sprite {
	protected:
		float x, y;          // Posicion (float para movimientos suaves)
		int ancho, alto;     // Tamanio con el que se dibuja
		Image^ hoja;         // Hoja de sprites (spritesheet)
		int colsHoja;        // Columnas totales de la hoja
		int filasHoja;       // Filas totales de la hoja
		int colOffset;       // Columna inicial de ESTE personaje dentro de la hoja
		int numFrames;       // Cantidad de cuadros de animacion (columnas a usar)
		int frame;           // Cuadro actual (0..numFrames-1)
		int fila;            // Fila actual = direccion (0 abajo,1 izq,2 der,3 arriba)
		int animTick;        // Contador interno para temporizar la animacion
		bool vivo;           // Indica si el sprite sigue activo

	public:
		Sprite();

		// Propiedades de acceso (accesores triviales en linea).
		property float X { float get() { return x; } void set(float v) { x = v; } }
		property float Y { float get() { return y; } void set(float v) { y = v; } }
		property int Ancho { int get() { return ancho; } }
		property int Alto { int get() { return alto; } }
		property bool Vivo { bool get() { return vivo; } void set(bool v) { vivo = v; } }

		Rectangle Caja();                 // Caja de colision
		bool Colisiona(Sprite^ otro);     // Colision con otro sprite
		void Animar(int cada);            // Avanza el cuadro de animacion
		virtual void Actualizar();        // Movimiento (polimorfico)
		virtual void Dibujar(Graphics^ g);// Dibuja el cuadro actual
	};
}
