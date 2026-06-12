#include "HuallalloJefe.h"

HuallalloJefe::HuallalloJefe(int px, int py) : LlamaDigital(px, py, 3, 9999) {
	ancho = 250;
	alto = 290;
	vida = vidaMax = 24;
	corrompiendo = false;
}

int HuallalloJefe::getVidaMax() { return vidaMax; }
bool HuallalloJefe::getCorrompiendo() { return corrompiendo; }
void HuallalloJefe::setCorrompiendo(bool value) { corrompiendo = value; }

void HuallalloJefe::recibirRayo() {
	if (!activo) return;
	vida--;
	if (vida <= 0) {
		vida = 0;
		activo = false;
	}
}

void HuallalloJefe::perseguir(int objetivoX, int objetivoY) {
	if (!activo) return;
	irHacia(objetivoX - ancho / 2, objetivoY - alto / 2);
	delay++;
	if (delay >= 3) { delay = 0; IDx = (IDx + 1) % 12; }
}

Rectangle HuallalloJefe::hitbox() {
	return Rectangle(x + ancho / 8, y + alto / 6, ancho * 3 / 4, alto * 3 / 4);
}

void HuallalloJefe::mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY) {
	if (!activo) return;
	int px = x - camaraX;
	int py = y - camaraY;
	int flama = (IDx + semilla) % 12;

	// Sombra
	g->FillEllipse(Brushes::Black, px + ancho / 8, py + alto - 22, ancho * 3 / 4, 26);

	// Gran cuerpo de fuego con cuernos
	array<Point>^ cuerpo = {
		Point(px + ancho / 2, py + (flama % 3) * 6),
		Point(px + ancho - 30, py + alto / 4),
		Point(px + ancho - 8, py + alto * 2 / 3),
		Point(px + ancho - 40 + (flama % 2) * 10, py + alto - 16),
		Point(px + 40 - (flama % 2) * 10, py + alto - 16),
		Point(px + 8, py + alto * 2 / 3),
		Point(px + 30, py + alto / 4)
	};
	g->FillPolygon(Brushes::DarkRed, cuerpo);

	array<Point>^ nucleo = {
		Point(px + ancho / 2, py + 30 + (flama % 4) * 4),
		Point(px + ancho - 60, py + alto / 2),
		Point(px + ancho - 70, py + alto - 26),
		Point(px + 70, py + alto - 26),
		Point(px + 60, py + alto / 2)
	};
	g->FillPolygon(Brushes::OrangeRed, nucleo);
	g->FillEllipse(Brushes::Orange, px + ancho / 2 - 45, py + alto / 2, 90, alto / 2 - 30);

	// Cuernos del devorador
	array<Point>^ cuernoIzq = {
		Point(px + 36, py + 44), Point(px + 6, py - 6), Point(px + 56, py + 26)
	};
	array<Point>^ cuernoDer = {
		Point(px + ancho - 36, py + 44), Point(px + ancho - 6, py - 6), Point(px + ancho - 56, py + 26)
	};
	g->FillPolygon(Brushes::Black, cuernoIzq);
	g->FillPolygon(Brushes::Black, cuernoDer);

	// Ojos
	g->FillRectangle(Brushes::Black, px + ancho / 2 - 42, py + alto / 2 - 6, 26, 22);
	g->FillRectangle(Brushes::Black, px + ancho / 2 + 16, py + alto / 2 - 6, 26, 22);
	Brush^ ojo = corrompiendo ? Brushes::Magenta : Brushes::Yellow;
	g->FillRectangle(ojo, px + ancho / 2 - 36, py + alto / 2 - 2, 12, 12);
	g->FillRectangle(ojo, px + ancho / 2 + 22, py + alto / 2 - 2, 12, 12);

	// Chispas glitch
	for (int i = 0; i < 7; i++) {
		int gx = px + ((semilla * 17 + i * 37 + IDx * 13) % ancho);
		int gy = py + ((semilla * 11 + i * 29 + IDx * 19) % (alto * 2 / 3));
		g->FillRectangle((i % 2 == 0) ? Brushes::Red : Brushes::Gold, gx, gy, 9, 9);
	}

	// Barra de vida sobre el jefe
	int barraAncho = ancho;
	int vidaAncho = (int)((double)vida / vidaMax * barraAncho);
	g->FillRectangle(Brushes::Black, px - 2, py - 24, barraAncho + 4, 14);
	g->FillRectangle(Brushes::Red, px, py - 22, vidaAncho, 10);
	g->DrawRectangle(Pens::White, px - 2, py - 24, barraAncho + 4, 14);
}
