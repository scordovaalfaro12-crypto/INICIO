#include "BasuraEspacial.h"
#include "Azar.h"

namespace JuegoEspacial {

	BasuraEspacial::BasuraEspacial(float px, float py, int ptipoPieza) : Enemigo(px, py, 1) {
		tipoPieza = ptipoPieza;
		angulo = Azar::entreF(0.0f, 360.0f);
		velocidadGiro = Azar::entreF(0.6f, 2.0f) * (Azar::moneda() ? 1.0f : -1.0f);
		fase = Azar::entreF(0.0f, 6.2832f);
	}

	void BasuraEspacial::mover() {
		angulo += velocidadGiro;
		fase += 0.05f;
		if (destello > 0) destello--;
	}

	void BasuraEspacial::atacar() {
		destello = 14;
	}

	float BasuraEspacial::desplaceY() {
		return (float)Math::Sin(fase) * 4.0f;
	}

	RectangleF BasuraEspacial::zona() {
		return RectangleF(x - 18.0f, y + desplaceY() - 14.0f, 36.0f, 28.0f);
	}

	void BasuraEspacial::dibujar(Graphics^ g) {
		float yv = y + desplaceY();

		g->TranslateTransform(x, yv);
		g->RotateTransform(angulo);

		if (tipoPieza == 0) {
			// panel solar roto
			SolidBrush marco(Color::FromArgb(150, 150, 160));
			g->FillRectangle(%marco, -17.0f, -11.0f, 34.0f, 22.0f);
			SolidBrush celda(Color::FromArgb(40, 80, 180));
			g->FillRectangle(%celda, -15.0f, -9.0f, 30.0f, 18.0f);
			Pen linea(Color::FromArgb(120, 180, 255), 1.0f);
			g->DrawLine(%linea, -15.0f, 0.0f, 15.0f, 0.0f);
			g->DrawLine(%linea, -5.0f, -9.0f, -5.0f, 9.0f);
			g->DrawLine(%linea, 5.0f, -9.0f, 5.0f, 9.0f);
			// esquina rota
			SolidBrush hueco(Color::FromArgb(15, 15, 30));
			array<PointF>^ rotura = gcnew array<PointF>(3);
			rotura[0] = PointF(15.0f, -9.0f);
			rotura[1] = PointF(15.0f, 2.0f);
			rotura[2] = PointF(6.0f, -9.0f);
			g->FillPolygon(%hueco, rotura);
		}
		else if (tipoPieza == 1) {
			// satelite viejo
			SolidBrush cuerpo(Color::FromArgb(170, 170, 180));
			g->FillRectangle(%cuerpo, -10.0f, -8.0f, 20.0f, 16.0f);
			SolidBrush ala(Color::FromArgb(200, 170, 60));
			g->FillRectangle(%ala, -22.0f, -5.0f, 10.0f, 10.0f);
			g->FillRectangle(%ala, 12.0f, -5.0f, 10.0f, 10.0f);
			Pen antena(Color::FromArgb(210, 210, 220), 2.0f);
			g->DrawLine(%antena, 0.0f, -8.0f, 0.0f, -16.0f);
			SolidBrush foco(Color::FromArgb(255, 120, 120));
			g->FillEllipse(%foco, -2.5f, -20.0f, 5.0f, 5.0f);
			// antena parabolica
			Pen plato(Color::FromArgb(220, 220, 230), 2.0f);
			g->DrawArc(%plato, -7.0f, 2.0f, 14.0f, 12.0f, 0.0f, 180.0f);
		}
		else {
			// tuerca gigante
			array<PointF>^ tuerca = gcnew array<PointF>(6);
			for (int i = 0; i < 6; i++) {
				double a = i * 60.0 * Math::PI / 180.0;
				tuerca[i] = PointF((float)Math::Cos(a) * 14.0f, (float)Math::Sin(a) * 14.0f);
			}
			SolidBrush metal(Color::FromArgb(160, 160, 170));
			g->FillPolygon(%metal, tuerca);
			Pen filo(Color::FromArgb(90, 90, 100), 2.0f);
			g->DrawPolygon(%filo, tuerca);
			SolidBrush hueco(Color::FromArgb(40, 40, 55));
			g->FillEllipse(%hueco, -6.0f, -6.0f, 12.0f, 12.0f);
		}

		g->ResetTransform();

		// resplandor al ser golpeada
		if (destello > 0) {
			Pen aura(Color::FromArgb(Math::Min(255, destello * 18), 255, 255, 255), 2.5f);
			g->DrawEllipse(%aura, x - 20.0f, yv - 17.0f, 40.0f, 34.0f);
		}
	}
}
