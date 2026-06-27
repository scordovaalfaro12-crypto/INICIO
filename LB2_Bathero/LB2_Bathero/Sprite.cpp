//==============================================================================
//  Sprite.cpp  -  Implementacion de la clase base Sprite
//==============================================================================
#include "Sprite.h"

namespace LB2_Bathero {

	Sprite::Sprite() {
		x = 0; y = 0; ancho = 48; alto = 48;
		hoja = nullptr; colsHoja = 1; filasHoja = 1; colOffset = 0; numFrames = 1;
		frame = 0; fila = 0; animTick = 0; vivo = true;
	}

	// Caja de colision (un poco mas pequenia que el sprite para ser justos).
	Rectangle Sprite::Caja() {
		int mx = ancho / 5, my = alto / 5;
		return Rectangle((int)x + mx, (int)y + my, ancho - 2 * mx, alto - 2 * my);
	}

	bool Sprite::Colisiona(Sprite^ otro) {
		Rectangle a = Caja();
		Rectangle b = otro->Caja();
		return a.IntersectsWith(b);
	}

	// Avanza el cuadro de animacion cada "cada" ticks.
	void Sprite::Animar(int cada) {
		if (numFrames <= 1) return;
		if (++animTick >= cada) { animTick = 0; frame = (frame + 1) % numFrames; }
	}

	// Metodo virtual base (cada subclase define su movimiento).
	void Sprite::Actualizar() {}

	// Dibuja el cuadro actual recortandolo de la hoja de sprites.
	void Sprite::Dibujar(Graphics^ g) {
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
}
