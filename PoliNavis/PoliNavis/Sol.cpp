#include "Sol.h"

namespace PoliNavis {

	Sol::Sol(float px, float py, float pradio) : Entidad(px, py) {
		radio = pradio;
		giro = 0.0f;
	}

	void Sol::mover() {
		giro += 0.4f;   // los rayos giran lentamente
	}

	float Sol::getRadio() { return radio; }

	void Sol::dibujar(Graphics^ g) {
		// halos translucidos
		for (int capa = 3; capa >= 1; capa--) {
			SolidBrush halo(Color::FromArgb(18 * capa, 255, 170, 40));
			float r = radio + capa * 16.0f;
			g->FillEllipse(%halo, x - r, y - r, r * 2.0f, r * 2.0f);
		}
		// rayos giratorios (lineas)
		Pen rayo(Color::FromArgb(80, 255, 200, 80), 2.0f);
		for (int i = 0; i < 12; i++) {
			double a = (giro + i * 30.0) * Math::PI / 180.0;
			g->DrawLine(%rayo,
				x + (float)Math::Cos(a) * (radio + 4.0f), y + (float)Math::Sin(a) * (radio + 4.0f),
				x + (float)Math::Cos(a) * (radio + 18.0f), y + (float)Math::Sin(a) * (radio + 18.0f));
		}
		// cuerpo
		SolidBrush sol(Color::FromArgb(255, 200, 60));
		g->FillEllipse(%sol, x - radio, y - radio, radio * 2.0f, radio * 2.0f);
		SolidBrush nucleo(Color::FromArgb(255, 240, 150));
		g->FillEllipse(%nucleo, x - radio * 0.7f, y - radio * 0.7f, radio * 1.4f, radio * 1.4f);
		// manchas solares
		SolidBrush mancha(Color::FromArgb(90, 230, 120, 20));
		g->FillEllipse(%mancha, x - radio * 0.3f, y - radio * 0.1f, radio * 0.3f, radio * 0.25f);
	}
}
