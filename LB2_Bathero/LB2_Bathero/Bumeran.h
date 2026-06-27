#pragma once
//==============================================================================
//  Bumeran.h  -  El arma arrojadiza de Bathero (hereda de Sprite)
//------------------------------------------------------------------------------
//  Estados:  0 = Quieto (en la mano de Bathero, no se ve)
//            1 = Lanzado (vuela hacia la derecha girando)
//            2 = Regresando (vuelve a su posicion de lanzamiento)
//  Si atrapa un enemigo o llega al limite, regresa a su posicion inicial.
//==============================================================================
#include "Sprite.h"

namespace LB2_Bathero {

	ref class Bumeran : public Sprite {
	private:
		int estado;        // 0 quieto, 1 lanzado, 2 regresando
		float ox, oy;      // Origen al que debe regresar
		float vel;         // Rapidez de vuelo
		float alcanceMax;  // X maxima que alcanza antes de regresar
		float angulo;      // Angulo de giro (efecto visual)

	public:
		Bumeran() {
			hoja = Util::CargarImagen("bumeran.png");
			colsHoja = 1; filasHoja = 1; numFrames = 1;
			ancho = 44; alto = 44;
			estado = 0; vel = 15.0f; angulo = 0.0f;
			ox = 0; oy = 0; alcanceMax = 0;
		}

		// Esta activo (visible y colisionable) solo cuando esta en vuelo.
		property bool Activo { bool get() { return estado != 0; } }

		// Lanza el bumeran desde (bx,by). Ignora si ya esta en vuelo.
		void Lanzar(float bx, float by, int areaAncho) {
			if (estado != 0) return;
			ox = bx; oy = by; x = bx; y = by;
			alcanceMax = (float)(areaAncho - ancho - 5);
			estado = 1;
			angulo = 0;
		}

		// Ordena el regreso (al atrapar un enemigo).
		void Regresar() { if (estado == 1) estado = 2; }

		// Mientras esta quieto, acompania a Bathero.
		void SeguirA(float bx, float by) {
			if (estado == 0) { x = bx; y = by; ox = bx; oy = by; }
		}

		virtual void Actualizar() override {
			angulo += 22; // gira siempre que esta activo
			if (estado == 1) {                 // yendo hacia la derecha
				x += vel;
				if (x >= alcanceMax) { x = alcanceMax; estado = 2; }
			}
			else if (estado == 2) {            // regresando al origen
				float dx = ox - x, dy = oy - y;
				float dist = (float)Math::Sqrt(dx * dx + dy * dy);
				if (dist <= vel) { x = ox; y = oy; estado = 0; }
				else { x += vel * dx / dist; y += vel * dy / dist; }
			}
		}

		// Se dibuja girando alrededor de su centro.
		virtual void Dibujar(Graphics^ g) override {
			if (estado == 0) return; // invisible cuando no esta en vuelo
			if (hoja == nullptr) {
				g->FillEllipse(Brushes::Orange, (int)x, (int)y, ancho, alto);
				return;
			}
			System::Drawing::Drawing2D::GraphicsState^ st = g->Save();
			g->TranslateTransform(x + ancho / 2.0f, y + alto / 2.0f);
			g->RotateTransform(angulo);
			g->DrawImage(hoja, Rectangle(-ancho / 2, -alto / 2, ancho, alto),
				0, 0, hoja->Width, hoja->Height, GraphicsUnit::Pixel);
			g->Restore(st);
		}
	};
}
