#include "Nave.h"
#include "Config.h"
#include "Sonido.h"

using namespace System::Drawing::Drawing2D;

namespace PoliNavis {

	Nave::Nave(float px, float py) : Entidad(px, py) {
		vidas = Config::VIDAS_INICIALES;
		energia = Config::ENERGIA_INICIAL;
		invulnerable = 0;
		angulo = -45.0f;        // mirando arriba-derecha (hacia el Punto B)
		faseLlama = 0.0f;
		colorTrazo = Color::FromArgb(120, 230, 255);
	}

	void Nave::acelerar(float ax, float ay) {
		dx = ax;
		dy = ay;
	}

	void Nave::mover() {
		x += dx;
		y += dy;

		// limites de la ventana
		if (x < 18.0f) x = 18.0f;
		if (x > Config::ANCHO - 18.0f) x = Config::ANCHO - 18.0f;
		if (y < 18.0f) y = 18.0f;
		if (y > Config::ALTO - 18.0f) y = Config::ALTO - 18.0f;

		// si se mueve, la nave apunta hacia su direccion
		if (dx != 0.0f || dy != 0.0f) {
			angulo = (float)(Math::Atan2(dy, dx) * 180.0 / Math::PI) + 90.0f;
			faseLlama += 0.6f;
		}
		if (invulnerable > 0) invulnerable--;
	}

	bool Nave::propulsando() {
		return dx != 0.0f || dy != 0.0f;
	}

	void Nave::reiniciarEn(float px, float py) {
		x = px; y = py;
		dx = dy = 0.0f;
		angulo = -45.0f;
		invulnerable = 120;     // 2 segundos de gracia
	}

	void Nave::perderVida() {
		vidas--;
		Sonido::danio();
	}

	void Nave::gastarEnergia(int cantidad) {
		energia -= cantidad;
		if (energia < 0) energia = 0;
	}

	void Nave::recargarEnergia(int cantidad) {
		energia += cantidad;
		if (energia > 100) energia = 100;
	}

	int Nave::getVidas() { return vidas; }
	int Nave::getEnergia() { return energia; }
	bool Nave::sinVidas() { return vidas <= 0; }
	bool Nave::esInvulnerable() { return invulnerable > 0; }
	void Nave::setColorTrazo(Color c) { colorTrazo = c; }

	RectangleF Nave::area() {
		return RectangleF(x - 13.0f, y - 13.0f, 26.0f, 26.0f);
	}

	void Nave::dibujar(Graphics^ g) {
		// parpadeo de invulnerabilidad
		if (invulnerable > 0 && (invulnerable / 5) % 2 == 0) return;

		g->TranslateTransform(x, y);
		g->RotateTransform(angulo);

		// ----- propulsor (llama animada detras) -----
		if (propulsando()) {
			float fl = (float)Math::Sin(faseLlama) * 5.0f + 14.0f;
			array<PointF>^ fuego = gcnew array<PointF>(3);
			fuego[0] = PointF(-6.0f, 12.0f);
			fuego[1] = PointF(6.0f, 12.0f);
			fuego[2] = PointF(0.0f, 12.0f + fl);
			SolidBrush naranja(Color::FromArgb(230, 255, 150, 40));
			g->FillPolygon(%naranja, fuego);
			array<PointF>^ fuego2 = gcnew array<PointF>(3);
			fuego2[0] = PointF(-3.0f, 12.0f);
			fuego2[1] = PointF(3.0f, 12.0f);
			fuego2[2] = PointF(0.0f, 12.0f + fl * 0.6f);
			SolidBrush amarillo(Color::FromArgb(245, 255, 240, 150));
			g->FillPolygon(%amarillo, fuego2);
		}

		// ----- alas (poligonos) -----
		array<PointF>^ alaIzq = gcnew array<PointF>(3);
		alaIzq[0] = PointF(-4.0f, 2.0f);
		alaIzq[1] = PointF(-16.0f, 12.0f);
		alaIzq[2] = PointF(-4.0f, 12.0f);
		array<PointF>^ alaDer = gcnew array<PointF>(3);
		alaDer[0] = PointF(4.0f, 2.0f);
		alaDer[1] = PointF(16.0f, 12.0f);
		alaDer[2] = PointF(4.0f, 12.0f);
		SolidBrush rojo(Color::FromArgb(210, 70, 70));
		g->FillPolygon(%rojo, alaIzq);
		g->FillPolygon(%rojo, alaDer);

		// ----- cuerpo (rombo / poligono) -----
		array<PointF>^ cuerpo = gcnew array<PointF>(4);
		cuerpo[0] = PointF(0.0f, -18.0f);   // nariz
		cuerpo[1] = PointF(8.0f, 8.0f);
		cuerpo[2] = PointF(0.0f, 14.0f);
		cuerpo[3] = PointF(-8.0f, 8.0f);
		SolidBrush plata(Color::FromArgb(225, 230, 240));
		g->FillPolygon(%plata, cuerpo);
		Pen filo(colorTrazo, 2.0f);
		g->DrawPolygon(%filo, cuerpo);

		// ----- cabina (poligono de vidrio) -----
		array<PointF>^ cabina = gcnew array<PointF>(4);
		cabina[0] = PointF(0.0f, -10.0f);
		cabina[1] = PointF(4.0f, 2.0f);
		cabina[2] = PointF(0.0f, 5.0f);
		cabina[3] = PointF(-4.0f, 2.0f);
		SolidBrush vidrio(colorTrazo);
		g->FillPolygon(%vidrio, cabina);

		// ----- detalle central (linea) -----
		Pen linea(Color::FromArgb(150, 70, 70), 1.5f);
		g->DrawLine(%linea, 0.0f, 6.0f, 0.0f, 12.0f);

		g->ResetTransform();
	}
}
