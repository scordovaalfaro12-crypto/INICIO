#include "Estrella.h"
#include "Astronauta.h"
#include "Sonido.h"

namespace JuegoEspacial {

	Estrella::Estrella(float px, float py, float pvy, float ptam, int pvalor) : Item(px, py, pvalor) {
		vy = pvy;
		tam = ptam;
		angulo = 0.0f;
	}

	void Estrella::mover() {
		y += vy;
		angulo += 1.8f;
	}

	void Estrella::aplicarEfecto(Astronauta^ astronauta) {
		// efecto: suma su valor al puntaje del astronauta
		astronauta->sumarPuntaje(valor);
		Sonido::item();
	}

	RectangleF Estrella::zona() {
		return RectangleF(x - tam, y - tam, tam * 2.0f, tam * 2.0f);
	}

	void Estrella::dibujarForma(Graphics^ g, float cx, float cy, float radio, float anguloGrados, Color color) {
		array<PointF>^ puntos = gcnew array<PointF>(10);
		for (int i = 0; i < 10; i++) {
			double a = (anguloGrados - 90.0 + i * 36.0) * Math::PI / 180.0;
			float r = (i % 2 == 0) ? radio : radio * 0.45f;
			puntos[i] = PointF(cx + (float)Math::Cos(a) * r, cy + (float)Math::Sin(a) * r);
		}
		SolidBrush pincel(color);
		g->FillPolygon(%pincel, puntos);
	}

	void Estrella::dibujar(Graphics^ g) {
		// halo pulsante
		int alfa = 60 + (int)(35.0 * Math::Sin(angulo * 0.1));
		SolidBrush halo(Color::FromArgb(alfa, 255, 220, 90));
		g->FillEllipse(%halo, x - tam * 1.6f, y - tam * 1.6f, tam * 3.2f, tam * 3.2f);

		dibujarForma(g, x, y, tam, angulo, Color::FromArgb(255, 210, 70));
		dibujarForma(g, x, y, tam * 0.55f, angulo, Color::FromArgb(255, 240, 160));
	}
}
