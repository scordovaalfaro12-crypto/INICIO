#include "Cometa.h"
#include "Azar.h"

namespace PoliNavis {

	Cometa::Cometa(float px, float py, float pdx, float pdy, float pradio, bool neon)
		: Obstaculo(px, py, pdx, pdy, pradio) {
		esNeon = neon;
		if (neon) {
			// cometas fosforescentes del Nivel 2
			array<Color>^ paleta = gcnew array<Color> {
				Color::FromArgb(255, 60, 200), Color::FromArgb(60, 255, 220),
				Color::FromArgb(180, 90, 255), Color::FromArgb(255, 230, 70)
			};
			color = paleta[Azar::entre(0, paleta->Length)];
		}
		else {
			color = Azar::moneda() ? Color::FromArgb(120, 220, 255) : Color::FromArgb(180, 230, 255);
		}
	}

	void Cometa::dibujar(Graphics^ g) {
		// direccion contraria al movimiento = hacia donde apunta la cola
		float largo = (float)Math::Sqrt(dx * dx + dy * dy);
		if (largo < 0.1f) largo = 0.1f;
		float nx = -dx / largo, ny = -dy / largo;
		float perpX = -ny, perpY = nx;

		// cola: triangulo largo que se desvanece (mas larga si es neon)
		float largoCola = esNeon ? 9.0f : 7.0f;
		array<PointF>^ cola = gcnew array<PointF>(3);
		cola[0] = PointF(x + perpX * radio, y + perpY * radio);
		cola[1] = PointF(x - perpX * radio, y - perpY * radio);
		cola[2] = PointF(x + nx * radio * largoCola, y + ny * radio * largoCola);
		SolidBrush brushCola(Color::FromArgb(70, color));
		g->FillPolygon(%brushCola, cola);
		// cola interior mas brillante
		array<PointF>^ cola2 = gcnew array<PointF>(3);
		cola2[0] = PointF(x + perpX * radio * 0.5f, y + perpY * radio * 0.5f);
		cola2[1] = PointF(x - perpX * radio * 0.5f, y - perpY * radio * 0.5f);
		cola2[2] = PointF(x + nx * radio * largoCola * 0.65f, y + ny * radio * largoCola * 0.65f);
		SolidBrush brushCola2(Color::FromArgb(140, esNeon ? color : Color::FromArgb(255, 255, 255)));
		g->FillPolygon(%brushCola2, cola2);

		// halo y nucleo
		SolidBrush halo(Color::FromArgb(esNeon ? 120 : 90, color));
		g->FillEllipse(%halo, x - radio * 1.6f, y - radio * 1.6f, radio * 3.2f, radio * 3.2f);
		SolidBrush brillo(color);
		g->FillEllipse(%brillo, x - radio, y - radio, radio * 2.0f, radio * 2.0f);
		SolidBrush centro(Color::FromArgb(255, 255, 255, 255));
		g->FillEllipse(%centro, x - radio * 0.45f, y - radio * 0.45f, radio * 0.9f, radio * 0.9f);
	}
}
