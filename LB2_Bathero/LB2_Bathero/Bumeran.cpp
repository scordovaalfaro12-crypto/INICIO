//==============================================================================
//  Bumeran.cpp  -  Implementacion del bumeran
//==============================================================================
#include "Bumeran.h"
#include "Util.h"

namespace LB2_Bathero {

	Bumeran::Bumeran() {
		hoja = Util::CargarImagen("bumeran.png");
		colsHoja = 1; filasHoja = 1; numFrames = 1;
		ancho = 44; alto = 44;
		estado = 0; vel = 15.0f; angulo = 0.0f;
		ox = 0; oy = 0; alcanceMax = 0;
	}

	// Lanza el bumeran desde (bx,by). Ignora si ya esta en vuelo.
	void Bumeran::Lanzar(float bx, float by, int areaAncho) {
		if (estado != 0) return;
		ox = bx; oy = by; x = bx; y = by;
		alcanceMax = (float)(areaAncho - ancho - 5);
		estado = 1;
		angulo = 0;
	}

	// Ordena el regreso (al atrapar un enemigo).
	void Bumeran::Regresar() { if (estado == 1) estado = 2; }

	// Mientras esta quieto, acompania a Bathero.
	void Bumeran::SeguirA(float bx, float by) {
		if (estado == 0) { x = bx; y = by; ox = bx; oy = by; }
	}

	void Bumeran::Actualizar() {
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
	void Bumeran::Dibujar(Graphics^ g) {
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
}
