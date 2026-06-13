#include "Estacion.h"

namespace PoliNavis {

	Estacion::Estacion(float px, float py, int ptipo) : Entidad(px, py) {
		tipo = ptipo;
		fase = 0.0f;
	}

	void Estacion::mover() {
		fase += 0.08f;
	}

	RectangleF Estacion::area() {
		return RectangleF(x - 34.0f, y - 34.0f, 68.0f, 68.0f);
	}

	void Estacion::dibujar(Graphics^ g) {
		// anillo de luz pulsante (atrae al jugador a la meta)
		int alfa = 90 + (int)(60.0 * Math::Sin(fase));
		Color guia = (tipo == 0) ? Color::FromArgb(alfa, 110, 230, 255) : Color::FromArgb(alfa, 200, 110, 255);
		Pen aura(guia, 3.0f);
		float rp = 42.0f + (float)Math::Sin(fase) * 5.0f;
		g->DrawEllipse(%aura, x - rp, y - rp, rp * 2.0f, rp * 2.0f);

		if (tipo == 0) {
			// ----- ESTACION ESPACIAL (Nivel 1) -----
			// paneles solares
			SolidBrush panel(Color::FromArgb(40, 80, 180));
			g->FillRectangle(%panel, x - 60.0f, y - 8.0f, 26.0f, 16.0f);
			g->FillRectangle(%panel, x + 34.0f, y - 8.0f, 26.0f, 16.0f);
			Pen marco(Color::FromArgb(150, 160, 175), 1.0f);
			g->DrawRectangle(%marco, x - 60.0f, y - 8.0f, 26.0f, 16.0f);
			g->DrawRectangle(%marco, x + 34.0f, y - 8.0f, 26.0f, 16.0f);
			Pen eje(Color::FromArgb(180, 185, 195), 3.0f);
			g->DrawLine(%eje, x - 34.0f, y, x + 34.0f, y);
			// anillo toroidal
			Pen anillo(Color::FromArgb(200, 205, 220), 6.0f);
			g->DrawEllipse(%anillo, x - 28.0f, y - 28.0f, 56.0f, 56.0f);
			// nucleo central
			SolidBrush nucleo(Color::FromArgb(210, 215, 230));
			g->FillEllipse(%nucleo, x - 14.0f, y - 14.0f, 28.0f, 28.0f);
			SolidBrush ventana(Color::FromArgb(120, 220, 255));
			g->FillEllipse(%ventana, x - 8.0f, y - 8.0f, 16.0f, 16.0f);
			// luces de atraque
			for (int i = 0; i < 4; i++) {
				bool on = ((int)(fase * 2) + i) % 2 == 0;
				SolidBrush luz(on ? Color::FromArgb(255, 90, 90) : Color::FromArgb(90, 60, 60));
				double a = i * 90.0 * Math::PI / 180.0;
				g->FillEllipse(%luz, x + (float)Math::Cos(a) * 28.0f - 3.0f,
					y + (float)Math::Sin(a) * 28.0f - 3.0f, 6.0f, 6.0f);
			}
		}
		else {
			// ----- PORTAL DE SALIDA (Nivel 2) -----
			for (int k = 4; k >= 1; k--) {
				int a = 40 + k * 20;
				SolidBrush aro(Color::FromArgb(a, 200, 110, 255));
				float r = 10.0f + k * 7.0f + (float)Math::Sin(fase + k) * 2.0f;
				g->FillEllipse(%aro, x - r, y - r, r * 2.0f, r * 2.0f);
			}
			SolidBrush centro(Color::FromArgb(255, 255, 255));
			g->FillEllipse(%centro, x - 8.0f, y - 8.0f, 16.0f, 16.0f);
			// destellos giratorios
			Pen chispa(Color::FromArgb(200, 230, 180, 255), 2.0f);
			for (int i = 0; i < 6; i++) {
				double a = (fase * 40.0 + i * 60.0) * Math::PI / 180.0;
				g->DrawLine(%chispa, x, y,
					x + (float)Math::Cos(a) * 30.0f, y + (float)Math::Sin(a) * 30.0f);
			}
		}
	}
}
