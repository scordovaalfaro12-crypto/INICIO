#include "FiguraKhaos.h"
#include "Config.h"
#include "Azar.h"

namespace PoliNavis {

	FiguraKhaos::FiguraKhaos(float px, float py, float pdx, float pdy, float pradio, int plados)
		: Obstaculo(px, py, pdx, pdy, pradio) {
		lados = plados;
		velGiro = Azar::entreF(-7.0f, 7.0f);   // giran mas rapido que el Nivel 1
		caos = Azar::entre(20, 70);
		// paleta neon totalmente distinta al Nivel 1
		array<Color>^ paleta = gcnew array<Color> {
			Color::FromArgb(255, 60, 200), Color::FromArgb(60, 255, 220),
			Color::FromArgb(180, 90, 255), Color::FromArgb(255, 230, 70),
			Color::FromArgb(80, 255, 120), Color::FromArgb(255, 120, 60)
		};
		color = paleta[Azar::entre(0, paleta->Length)];
	}

	void FiguraKhaos::mover() {
		x += dx;
		y += dy;
		giro += velGiro;

		// rebote en los bordes (la figura no abandona la galaxia)
		if (x < radio && dx < 0) dx = -dx;
		if (x > Config::ANCHO - radio && dx > 0) dx = -dx;
		if (y < radio && dy < 0) dy = -dy;
		if (y > Config::ALTO - radio && dy > 0) dy = -dy;

		// cambios bruscos de rumbo: caos geometrico impredecible
		caos--;
		if (caos <= 0) {
			caos = Azar::entre(20, 70);
			dx += Azar::entreF(-1.6f, 1.6f);
			dy += Azar::entreF(-1.6f, 1.6f);
			// limitar la rapidez para que no se descontrole
			float v = (float)Math::Sqrt(dx * dx + dy * dy);
			float vmax = 7.5f;
			if (v > vmax) { dx = dx / v * vmax; dy = dy / v * vmax; }
		}
	}

	void FiguraKhaos::dibujar(Graphics^ g) {
		array<PointF>^ p = gcnew array<PointF>(lados);
		for (int i = 0; i < lados; i++) {
			double a = (giro + i * (360.0 / lados)) * Math::PI / 180.0;
			p[i] = PointF(x + (float)Math::Cos(a) * radio, y + (float)Math::Sin(a) * radio);
		}
		// resplandor neon
		SolidBrush halo(Color::FromArgb(60, color));
		g->FillEllipse(%halo, x - radio * 1.4f, y - radio * 1.4f, radio * 2.8f, radio * 2.8f);
		// relleno translucido + borde neon brillante
		SolidBrush relleno(Color::FromArgb(110, color));
		g->FillPolygon(%relleno, p);
		Pen borde(color, 2.5f);
		g->DrawPolygon(%borde, p);
		// nucleo
		SolidBrush nucleo(Color::FromArgb(220, 255, 255, 255));
		g->FillEllipse(%nucleo, x - radio * 0.22f, y - radio * 0.22f, radio * 0.44f, radio * 0.44f);
	}
}
