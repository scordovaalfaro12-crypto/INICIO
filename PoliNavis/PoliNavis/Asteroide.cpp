#include "Asteroide.h"
#include "Azar.h"
#include "Estilos.h"

namespace PoliNavis {

	Asteroide::Asteroide(float px, float py, float pdx, float pdy, float pradio)
		: Obstaculo(px, py, pdx, pdy, pradio) {
		forma = gcnew array<float>(9);
		for (int i = 0; i < 9; i++) forma[i] = Azar::entreF(0.68f, 1.0f);
		int tono = Azar::entre(0, 3);
		if (tono == 0) color = Color::FromArgb(150, 130, 110);
		else if (tono == 1) color = Color::FromArgb(140, 110, 95);
		else color = Color::FromArgb(120, 105, 100);
	}

	void Asteroide::dibujar(Graphics^ g) {
		array<PointF>^ p = gcnew array<PointF>(9);
		for (int i = 0; i < 9; i++) {
			double a = (giro + i * 40.0) * Math::PI / 180.0;
			float rr = radio * forma[i];
			p[i] = PointF(x + (float)Math::Cos(a) * rr, y + (float)Math::Sin(a) * rr);
		}
		SolidBrush cuerpo(color);
		g->FillPolygon(%cuerpo, p);
		Pen borde(Estilos::oscurecer(color, 0.55f), 2.0f);
		g->DrawPolygon(%borde, p);
		// crateres
		SolidBrush crater(Color::FromArgb(120, Estilos::oscurecer(color, 0.5f)));
		g->FillEllipse(%crater, x - radio * 0.4f, y - radio * 0.25f, radio * 0.45f, radio * 0.38f);
		g->FillEllipse(%crater, x + radio * 0.1f, y + radio * 0.2f, radio * 0.32f, radio * 0.28f);
		// brillo
		SolidBrush brillo(Color::FromArgb(45, 255, 255, 255));
		g->FillEllipse(%brillo, x - radio * 0.5f, y - radio * 0.55f, radio * 0.6f, radio * 0.4f);
	}
}
