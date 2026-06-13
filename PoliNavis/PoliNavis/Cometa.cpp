#include "Cometa.h"
#include "Azar.h"

namespace PoliNavis {

	Cometa::Cometa(float px, float py, float pdx, float pdy, float pradio)
		: Obstaculo(px, py, pdx, pdy, pradio) {
		color = Azar::moneda() ? Color::FromArgb(120, 220, 255) : Color::FromArgb(180, 230, 255);
	}

	void Cometa::dibujar(Graphics^ g) {
		// direccion contraria al movimiento = hacia donde apunta la cola
		float largo = (float)Math::Sqrt(dx * dx + dy * dy);
		if (largo < 0.1f) largo = 0.1f;
		float nx = -dx / largo, ny = -dy / largo;
		float perpX = -ny, perpY = nx;

		// cola: triangulo largo que se desvanece
		array<PointF>^ cola = gcnew array<PointF>(3);
		cola[0] = PointF(x + perpX * radio, y + perpY * radio);
		cola[1] = PointF(x - perpX * radio, y - perpY * radio);
		cola[2] = PointF(x + nx * radio * 7.0f, y + ny * radio * 7.0f);
		SolidBrush brushCola(Color::FromArgb(70, color));
		g->FillPolygon(%brushCola, cola);
		// cola interior mas brillante
		array<PointF>^ cola2 = gcnew array<PointF>(3);
		cola2[0] = PointF(x + perpX * radio * 0.5f, y + perpY * radio * 0.5f);
		cola2[1] = PointF(x - perpX * radio * 0.5f, y - perpY * radio * 0.5f);
		cola2[2] = PointF(x + nx * radio * 4.5f, y + ny * radio * 4.5f);
		SolidBrush brushCola2(Color::FromArgb(130, 255, 255, 255));
		g->FillPolygon(%brushCola2, cola2);

		// halo y nucleo
		SolidBrush halo(Color::FromArgb(90, color));
		g->FillEllipse(%halo, x - radio * 1.5f, y - radio * 1.5f, radio * 3.0f, radio * 3.0f);
		SolidBrush nucleo(Color::FromArgb(255, 255, 255));
		g->FillEllipse(%nucleo, x - radio * 0.7f, y - radio * 0.7f, radio * 1.4f, radio * 1.4f);
		SolidBrush brillo(color);
		g->FillEllipse(%brillo, x - radio, y - radio, radio * 2.0f, radio * 2.0f);
		SolidBrush centro(Color::FromArgb(255, 255, 255, 255));
		g->FillEllipse(%centro, x - radio * 0.4f, y - radio * 0.4f, radio * 0.8f, radio * 0.8f);
	}
}
