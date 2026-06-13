#include "Fondo.h"
#include "Azar.h"
#include "Config.h"

using namespace System::Drawing::Drawing2D;

namespace PoliNavis {

	void Fondo::crear() {
		ex = gcnew array<float>(CANTIDAD);
		ey = gcnew array<float>(CANTIDAD);
		etam = gcnew array<float>(CANTIDAD);
		efase = gcnew array<float>(CANTIDAD);
		for (int i = 0; i < CANTIDAD; i++) {
			ex[i] = Azar::entreF(0.0f, (float)Config::ANCHO);
			ey[i] = Azar::entreF(0.0f, (float)Config::ALTO);
			etam[i] = Azar::entreF(1.0f, 2.8f);
			efase[i] = Azar::entreF(0.0f, 6.2832f);
		}
	}

	void Fondo::dibujarEstrellas(Graphics^ g, int tick, int alfaMaximo) {
		if (ex == nullptr) crear();
		for (int i = 0; i < CANTIDAD; i++) {
			float brillo = 0.45f + 0.55f * (float)Math::Abs(Math::Sin(tick * 0.04 + efase[i]));
			int alfa = (int)(alfaMaximo * brillo);
			if (alfa < 0) alfa = 0;
			if (alfa > 255) alfa = 255;
			SolidBrush pincel(Color::FromArgb(alfa, 255, 255, 255));
			g->FillEllipse(%pincel, ex[i], ey[i], etam[i], etam[i]);
		}
	}

	void Fondo::dibujar(Graphics^ g, int tick) {
		if (ex == nullptr) crear();

		// degradado oscuro del espacio
		LinearGradientBrush cielo(
			RectangleF(0.0f, 0.0f, (float)Config::ANCHO, (float)Config::ALTO),
			Color::FromArgb(6, 6, 18), Color::FromArgb(24, 14, 48), 90.0f);
		g->FillRectangle(%cielo, 0.0f, 0.0f, (float)Config::ANCHO, (float)Config::ALTO);

		dibujarEstrellas(g, tick, 255);

		// de vez en cuando pasa una estrella fugaz
		if (fugazVida <= 0 && Azar::entre(0, 300) == 7) {
			fugazX = Azar::entreF(100.0f, 900.0f);
			fugazY = Azar::entreF(20.0f, 200.0f);
			fugazVX = Azar::entreF(5.0f, 9.0f) * (Azar::moneda() ? 1.0f : -1.0f);
			fugazVY = Azar::entreF(1.5f, 3.5f);
			fugazVida = 40;
		}
		if (fugazVida > 0) {
			fugazVida--;
			fugazX += fugazVX;
			fugazY += fugazVY;
			int alfa = Math::Min(255, fugazVida * 8);
			Pen pluma(Color::FromArgb(alfa, 255, 255, 220), 2.0f);
			g->DrawLine(%pluma, fugazX, fugazY, fugazX - fugazVX * 5.0f, fugazY - fugazVY * 5.0f);
		}
	}
}
