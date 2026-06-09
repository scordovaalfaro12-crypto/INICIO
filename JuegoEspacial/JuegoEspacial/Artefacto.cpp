#include "Artefacto.h"
#include "Astronauta.h"
#include "Sonido.h"
#include "Azar.h"

namespace JuegoEspacial {

	Artefacto::Artefacto(float px, float py, int pvalor) : Item(px, py, pvalor) {
		fase = Azar::entreF(0.0f, 6.2832f);
	}

	void Artefacto::mover() {
		fase += 0.06f;
	}

	float Artefacto::desplaceY() {
		return (float)Math::Sin(fase) * 5.0f;
	}

	void Artefacto::aplicarEfecto(Astronauta^ astronauta) {
		astronauta->sumarPuntaje(valor);
		Sonido::item();
	}

	RectangleF Artefacto::zona() {
		return RectangleF(x - 16.0f, y + desplaceY() - 16.0f, 32.0f, 32.0f);
	}

	void Artefacto::dibujar(Graphics^ g) {
		float yv = y + desplaceY();

		// aura misteriosa
		int alfa = 50 + (int)(30.0 * Math::Sin(fase * 2.0));
		SolidBrush aura(Color::FromArgb(alfa, 90, 230, 255));
		g->FillEllipse(%aura, x - 24.0f, yv - 24.0f, 48.0f, 48.0f);

		// piramide dorada
		array<PointF>^ piramide = gcnew array<PointF>(3);
		piramide[0] = PointF(x - 14.0f, yv + 11.0f);
		piramide[1] = PointF(x + 14.0f, yv + 11.0f);
		piramide[2] = PointF(x, yv - 13.0f);
		SolidBrush oro(Color::FromArgb(225, 185, 80));
		g->FillPolygon(%oro, piramide);
		Pen filo(Color::FromArgb(140, 110, 40), 2.0f);
		g->DrawPolygon(%filo, piramide);
		Pen detalle(Color::FromArgb(255, 230, 150), 1.0f);
		g->DrawLine(%detalle, x - 7.0f, yv + 5.0f, x + 7.0f, yv + 5.0f);

		// gema central que late
		float pulso = 1.0f + 0.25f * (float)Math::Sin(fase * 3.0);
		SolidBrush gema(Color::FromArgb(80, 230, 255));
		g->FillEllipse(%gema, x - 4.0f * pulso, yv + 1.0f - 4.0f * pulso, 8.0f * pulso, 8.0f * pulso);

		// anillo orbital
		Pen anillo(Color::FromArgb(150, 120, 240, 255), 1.5f);
		g->DrawEllipse(%anillo, x - 19.0f, yv - 3.0f, 38.0f, 11.0f);
	}
}
