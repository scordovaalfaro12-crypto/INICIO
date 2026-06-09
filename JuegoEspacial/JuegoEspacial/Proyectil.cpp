#include "Proyectil.h"
#include "Config.h"

namespace JuegoEspacial {

	Proyectil::Proyectil(float px, float py, float pvx, float pvy, int ptipo) : EntidadEspacial(px, py) {
		vx = pvx;
		vy = pvy;
		tipo = ptipo;
	}

	void Proyectil::mover() {
		x += vx;
		y += vy;
	}

	bool Proyectil::fuera() {
		return x < -40.0f || x > Config::ANCHO + 40.0f || y < -60.0f || y > Config::ALTO + 60.0f;
	}

	int Proyectil::getTipo() {
		return tipo;
	}

	RectangleF Proyectil::zona() {
		if (tipo == LASER) return RectangleF(x - 3.0f, y - 4.0f, 6.0f, 18.0f);
		if (tipo == RAYO)  return RectangleF(x - 8.0f, y - 10.0f, 16.0f, 26.0f);
		return RectangleF(x - 5.0f, y - 6.0f, 10.0f, 14.0f);
	}

	void Proyectil::dibujar(Graphics^ g) {
		if (tipo == LASER) {
			// laser cian con halo
			Pen halo(Color::FromArgb(90, 90, 255, 255), 7.0f);
			g->DrawLine(%halo, x, y, x, y + 14.0f);
			Pen nucleo(Color::FromArgb(240, 170, 255, 255), 3.0f);
			g->DrawLine(%nucleo, x, y, x, y + 14.0f);
		}
		else if (tipo == RAYO) {
			// rayo magenta del jefe (apunta en la direccion del movimiento)
			float colaX = x - vx * 3.5f;
			float colaY = y - vy * 3.5f;
			Pen halo(Color::FromArgb(110, 255, 80, 220), 11.0f);
			g->DrawLine(%halo, x, y, colaX, colaY);
			Pen nucleo(Color::FromArgb(250, 255, 210, 250), 4.0f);
			g->DrawLine(%nucleo, x, y, colaX, colaY);
		}
		else {
			// gota acida verde
			SolidBrush cuerpo(Color::FromArgb(230, 120, 230, 90));
			g->FillEllipse(%cuerpo, x - 5.0f, y - 4.0f, 10.0f, 12.0f);
			array<PointF>^ punta = gcnew array<PointF>(3);
			punta[0] = PointF(x - 4.0f, y);
			punta[1] = PointF(x + 4.0f, y);
			punta[2] = PointF(x, y - 9.0f);
			g->FillPolygon(%cuerpo, punta);
			SolidBrush brillo(Color::FromArgb(170, 230, 255, 200));
			g->FillEllipse(%brillo, x - 2.5f, y - 1.0f, 3.0f, 4.0f);
		}
	}
}
