#pragma once
//==============================================================================
//  Sprite.h  -  Clase BASE de toda la jerarquia (POO: herencia y polimorfismo)
//------------------------------------------------------------------------------
//  Encapsula los atributos y comportamientos comunes a cualquier objeto que se
//  dibuja y anima en pantalla: posicion, tamanio, hoja de sprites, recorte del
//  cuadro de animacion, deteccion de colisiones, etc.
//==============================================================================
#include "Util.h"

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
		Sprite() {
			x = 0; y = 0; ancho = 48; alto = 48;
			hoja = nullptr; colsHoja = 1; filasHoja = 1; colOffset = 0; numFrames = 1;
			frame = 0; fila = 0; animTick = 0; vivo = true;
		}

		// ---- Propiedades de acceso ----
		property float X { float get() { return x; } void set(float v) { x = v; } }
		property float Y { float get() { return y; } void set(float v) { y = v; } }
		property int Ancho { int get() { return ancho; } }
		property int Alto { int get() { return alto; } }
		property bool Vivo { bool get() { return vivo; } void set(bool v) { vivo = v; } }

		// Caja de colision (un poco mas pequenia que el sprite para ser justos).
		Rectangle Caja() {
			int mx = ancho / 5, my = alto / 5;
			return Rectangle((int)x + mx, (int)y + my, ancho - 2 * mx, alto - 2 * my);
		}

		// Detecta colision con otro sprite.
		bool Colisiona(Sprite^ otro) {
			Rectangle a = Caja();
			Rectangle b = otro->Caja();
			return a.IntersectsWith(b);
		}

		// Avanza el cuadro de animacion cada "cada" ticks.
		void Animar(int cada) {
			if (numFrames <= 1) return;
			if (++animTick >= cada) { animTick = 0; frame = (frame + 1) % numFrames; }
		}

		// Metodo polimorfico: cada subclase define su movimiento.
		virtual void Actualizar() {}

		// Dibuja el cuadro actual recortandolo de la hoja de sprites.
		virtual void Dibujar(Graphics^ g) {
			if (hoja == nullptr) {
				// Si la imagen no se encontro, se dibuja un marcador para no fallar.
				g->FillRectangle(Brushes::Magenta, (int)x, (int)y, ancho, alto);
				return;
			}
			int c = colOffset + frame;
			int sx = c * hoja->Width / colsHoja;
			int sw = (c + 1) * hoja->Width / colsHoja - sx;
			int sy = fila * hoja->Height / filasHoja;
			int sh = (fila + 1) * hoja->Height / filasHoja - sy;
			g->DrawImage(hoja, Rectangle((int)x, (int)y, ancho, alto),
				sx, sy, sw, sh, GraphicsUnit::Pixel);
		}
	};
}
