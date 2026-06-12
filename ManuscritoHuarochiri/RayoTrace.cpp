#include "RayoTrace.h"

RayoTrace::RayoTrace(int px, int py, int dirX, int dirY) {
	ancho = 46;
	alto = 46;
	x = px - ancho / 2;
	y = py - alto / 2;
	int velocidad = 16;
	// Si no hay direccion (no deberia pasar), dispara hacia abajo
	if (dirX == 0 && dirY == 0) dirY = 1;
	dx = dirX * velocidad;
	dy = dirY * velocidad;
	vidaUtil = 36; // ~1 segundo de alcance
}

bool RayoTrace::terminado() { return vidaUtil <= 0 || !activo; }

void RayoTrace::mover(Graphics^ g) {
	x += dx;
	y += dy;
	vidaUtil--;
	delay++;
	if (delay >= 2) { delay = 0; IDx = (IDx + 1) % 8; }
}

Rectangle RayoTrace::hitbox() { return Rectangle(x, y, ancho, alto); }

void RayoTrace::mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY) {
	if (terminado()) return;
	int px = x - camaraX + ancho / 2;
	int py = y - camaraY + alto / 2;

	// Rayo en zigzag perpendicular a su direccion de vuelo
	bool horizontal = (dx != 0);
	Pen^ brillo = gcnew Pen(Color::Cyan, 7.0f);
	Pen^ nucleo = gcnew Pen(Color::White, 3.0f);
	array<Point>^ pts = gcnew array<Point>(5);
	for (int i = 0; i < 5; i++) {
		int paso = (i - 2) * 12;
		int zig = ((i + IDx) % 2 == 0) ? 9 : -9;
		if (horizontal) pts[i] = Point(px + paso, py + zig);
		else pts[i] = Point(px + zig, py + paso);
	}
	g->DrawLines(brillo, pts);
	g->DrawLines(nucleo, pts);
	delete brillo;
	delete nucleo;

	// Chispas
	g->FillEllipse(Brushes::Cyan, px - 7, py - 7, 14, 14);
	g->FillEllipse(Brushes::White, px - 3, py - 3, 6, 6);
}
