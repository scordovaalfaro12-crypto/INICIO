#include "LlamaDigital.h"

LlamaDigital::LlamaDigital(int px, int py, int vel, int radioPers) {
	x = baseX = objetivoX = px;
	y = baseY = objetivoY = py;
	velocidad = vel;
	radioPersecucion = radioPers;
	radioPatrulla = 260;
	pausa = 0;
	ancho = 110;
	alto = 130;
	vida = 1;
	semilla = rand() % 97;
}

void LlamaDigital::irHacia(int px, int py) {
	double difX = (double)(px - x);
	double difY = (double)(py - y);
	double dist = sqrt(difX * difX + difY * difY);
	if (dist < 2) return;
	x += (int)(difX / dist * velocidad);
	y += (int)(difY / dist * velocidad);
}

void LlamaDigital::perseguir(int jugadorX, int jugadorY) {
	if (!activo) return;
	double difX = (double)(jugadorX - centroX());
	double difY = (double)(jugadorY - centroY());
	double dist = sqrt(difX * difX + difY * difY);

	if (dist < radioPersecucion) {
		// Persigue al jugador
		irHacia(jugadorX - ancho / 2, jugadorY - alto / 2);
	}
	else {
		// Patrulla: camina hacia un punto aleatorio cerca de su base
		if (pausa > 0) { pausa--; }
		else {
			double dox = (double)(objetivoX - x);
			double doy = (double)(objetivoY - y);
			if (sqrt(dox * dox + doy * doy) < velocidad * 2) {
				objetivoX = baseX + (rand() % (radioPatrulla * 2)) - radioPatrulla;
				objetivoY = baseY + (rand() % (radioPatrulla * 2)) - radioPatrulla;
				pausa = 20 + rand() % 50;
			}
			else {
				double dist2 = sqrt(dox * dox + doy * doy);
				x += (int)(dox / dist2 * (velocidad - 1 > 0 ? velocidad - 1 : 1));
				y += (int)(doy / dist2 * (velocidad - 1 > 0 ? velocidad - 1 : 1));
			}
		}
	}

	delay++;
	if (delay >= 3) { delay = 0; IDx = (IDx + 1) % 12; }
}

Rectangle LlamaDigital::hitbox() {
	return Rectangle(x + ancho / 6, y + alto / 5, ancho * 2 / 3, alto * 3 / 4);
}

void LlamaDigital::mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY) {
	if (!activo) return;
	int px = x - camaraX;
	int py = y - camaraY;
	int flama = (IDx + semilla) % 12;

	// Sombra
	g->FillEllipse(Brushes::Black, px + ancho / 6, py + alto - 14, ancho * 2 / 3, 16);

	// Cuerpo de la flama: tres capas glitch (rojo oscuro, naranja, amarillo)
	array<Point>^ capa1 = {
		Point(px + ancho / 2, py + (flama % 3) * 4),
		Point(px + ancho - 6, py + alto / 3),
		Point(px + ancho - 14 + (flama % 2) * 6, py + alto - 8),
		Point(px + 14 - (flama % 2) * 6, py + alto - 8),
		Point(px + 6, py + alto / 3)
	};
	g->FillPolygon(Brushes::Firebrick, capa1);

	array<Point>^ capa2 = {
		Point(px + ancho / 2, py + 14 + (flama % 4) * 3),
		Point(px + ancho - 24, py + alto / 2),
		Point(px + ancho - 28, py + alto - 12),
		Point(px + 28, py + alto - 12),
		Point(px + 24, py + alto / 2)
	};
	g->FillPolygon(Brushes::OrangeRed, capa2);

	int nucleo = 8 + (flama % 3) * 3;
	g->FillEllipse(Brushes::Orange, px + ancho / 2 - nucleo - 6, py + alto / 2 - 4, (nucleo + 6) * 2, alto / 2);
	g->FillEllipse(Brushes::Gold, px + ancho / 2 - nucleo / 2 - 4, py + alto / 2 + 6, nucleo + 8, alto / 3);

	// Pixeles glitch que se desprenden
	for (int i = 0; i < 4; i++) {
		int gx = px + ((semilla * 17 + i * 31 + IDx * 13) % ancho);
		int gy = py + ((semilla * 11 + i * 23 + IDx * 19) % (alto / 2));
		g->FillRectangle((i % 2 == 0) ? Brushes::OrangeRed : Brushes::Yellow, gx, gy, 7, 7);
	}

	// Ojos del virus
	g->FillRectangle(Brushes::Black, px + ancho / 2 - 18, py + alto / 2 + 8, 10, 12);
	g->FillRectangle(Brushes::Black, px + ancho / 2 + 8, py + alto / 2 + 8, 10, 12);
	g->FillRectangle(Brushes::White, px + ancho / 2 - 16, py + alto / 2 + 10, 4, 5);
	g->FillRectangle(Brushes::White, px + ancho / 2 + 10, py + alto / 2 + 10, 4, 5);
}
