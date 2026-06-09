#include "Asteroide.h"
#include "Azar.h"
#include "Estilos.h"

namespace JuegoEspacial {

	Asteroide::Asteroide(float px, float py, float pvx, float pvy, float pradio, Color pcolor)
		: Enemigo(px, py, 2) {
		vx = pvx;
		vy = pvy;
		radio = pradio;
		colorBase = pcolor;
		angulo = Azar::entreF(0.0f, 360.0f);
		velocidadGiro = Azar::entreF(1.0f, 3.2f) * (Azar::moneda() ? 1.0f : -1.0f);
		forma = gcnew array<float>(8);
		for (int i = 0; i < 8; i++) {
			forma[i] = Azar::entreF(0.72f, 1.0f);
		}
	}

	void Asteroide::mover() {
		x += vx;
		y += vy;
		angulo += velocidadGiro;
		if (destello > 0) destello--;
	}

	void Asteroide::atacar() {
		// el asteroide ataca al chocar: se enciende un instante
		destello = 14;
	}

	void Asteroide::rebotar(float minX, float minY, float maxX, float maxY) {
		if (x - radio < minX && vx < 0) vx = -vx;
		if (x + radio > maxX && vx > 0) vx = -vx;
		if (y - radio < minY && vy < 0) vy = -vy;
		if (y + radio > maxY && vy > 0) vy = -vy;
	}

	float Asteroide::getRadio() {
		return radio;
	}

	RectangleF Asteroide::zona() {
		return RectangleF(x - radio * 0.8f, y - radio * 0.8f, radio * 1.6f, radio * 1.6f);
	}

	void Asteroide::dibujar(Graphics^ g) {
		// polilgono irregular de 8 vertices que rota
		array<PointF>^ puntos = gcnew array<PointF>(8);
		for (int i = 0; i < 8; i++) {
			double a = (angulo + i * 45.0) * Math::PI / 180.0;
			float r = radio * forma[i];
			puntos[i] = PointF(x + (float)Math::Cos(a) * r, y + (float)Math::Sin(a) * r);
		}

		SolidBrush cuerpo(colorBase);
		g->FillPolygon(%cuerpo, puntos);
		Pen borde(Estilos::oscurecer(colorBase, 0.55f), 2.0f);
		g->DrawPolygon(%borde, puntos);

		// crateres
		SolidBrush crater(Color::FromArgb(120, Estilos::oscurecer(colorBase, 0.45f)));
		g->FillEllipse(%crater, x - radio * 0.45f, y - radio * 0.3f, radio * 0.5f, radio * 0.4f);
		g->FillEllipse(%crater, x + radio * 0.1f, y + radio * 0.2f, radio * 0.36f, radio * 0.3f);

		// brillo superior
		SolidBrush brillo(Color::FromArgb(50, 255, 255, 255));
		g->FillEllipse(%brillo, x - radio * 0.5f, y - radio * 0.6f, radio * 0.7f, radio * 0.45f);

		// resplandor al atacar o ser golpeado
		if (destello > 0) {
			Pen aura(Color::FromArgb(Math::Min(255, destello * 18), 255, 255, 255), 3.0f);
			g->DrawPolygon(%aura, puntos);
		}
	}
}
