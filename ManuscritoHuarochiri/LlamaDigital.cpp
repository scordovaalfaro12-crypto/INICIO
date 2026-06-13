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

bool LlamaDigital::recibirImpacto() {
	vida--;
	if (vida <= 0) {
		vida = 0;
		activo = false;
		return true;
	}
	return false;
}

int LlamaDigital::colorParticula() { return 0; } // fuego

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

// =========================== GolemLava ===========================

GolemLava::GolemLava(int px, int py) : LlamaDigital(px, py, 2, 9999) {
	ancho = 150;
	alto = 168;
	vida = 3; // tanque: aguanta 3 rayos
}

void GolemLava::perseguir(int jugadorX, int jugadorY) {
	if (!activo) return;
	// Marcha lenta e implacable hacia el jugador
	irHacia(jugadorX - ancho / 2, jugadorY - alto / 2);
	delay++;
	if (delay >= 4) { delay = 0; IDx = (IDx + 1) % 10; }
}

int GolemLava::colorParticula() { return 0; }

Rectangle GolemLava::hitbox() {
	return Rectangle(x + ancho / 5, y + alto / 6, ancho * 3 / 5, alto * 3 / 4);
}

void GolemLava::mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY) {
	if (!activo) return;
	int px = x - camaraX;
	int py = y - camaraY;
	int latido = (IDx + semilla) % 10;

	// Sombra
	g->FillEllipse(Brushes::Black, px + ancho / 6, py + alto - 16, ancho * 2 / 3, 18);

	// Resplandor interno que palpita
	SolidBrush^ brillo = gcnew SolidBrush(Color::FromArgb(40 + latido * 6, 255, 90, 0));
	g->FillEllipse(brillo, px + 10, py + 10, ancho - 20, alto - 20);
	delete brillo;

	// Cuerpo rocoso (bloques de piedra gris)
	g->FillRectangle(Brushes::DimGray, px + ancho / 4, py + alto / 4, ancho / 2, alto / 2 + 10);   // torso
	g->FillRectangle(Brushes::Gray, px + ancho / 3, py + 8, ancho / 3, alto / 4);                   // cabeza
	g->FillRectangle(Brushes::DimGray, px + 6, py + alto / 3, ancho / 5, alto / 3);                 // brazo izq
	g->FillRectangle(Brushes::DimGray, px + ancho - ancho / 5 - 6, py + alto / 3, ancho / 5, alto / 3); // brazo der
	g->FillRectangle(Brushes::Gray, px + ancho / 3, py + alto - alto / 5, ancho / 8, alto / 5);     // pierna izq
	g->FillRectangle(Brushes::Gray, px + ancho / 2, py + alto - alto / 5, ancho / 8, alto / 5);     // pierna der

	// Grietas de lava (lineas que brillan segun el latido)
	Pen^ grieta = gcnew Pen(Color::FromArgb(255, 255, 120 + latido * 10, 0), 3.0f);
	g->DrawLine(grieta, px + ancho / 2, py + alto / 4, px + ancho / 2 - 10, py + alto / 2);
	g->DrawLine(grieta, px + ancho / 2 - 10, py + alto / 2, px + ancho / 2 + 14, py + alto * 2 / 3);
	g->DrawLine(grieta, px + ancho / 3 + 6, py + alto / 3, px + ancho / 2, py + alto / 2);
	delete grieta;

	// Ojos incandescentes
	g->FillRectangle(Brushes::Gold, px + ancho / 3 + 8, py + alto / 8 + 6, 10, 8);
	g->FillRectangle(Brushes::Gold, px + ancho / 2 + 6, py + alto / 8 + 6, 10, 8);
}

// =========================== LoboLava ============================

LoboLava::LoboLava(int px, int py) : LlamaDigital(px, py, 4, 460) {
	ancho = 132;
	alto = 86;
	vida = 1;
	embestida = 0;
	recarga = 0;
}

void LoboLava::perseguir(int jugadorX, int jugadorY) {
	if (!activo) return;
	double difX = (double)(jugadorX - centroX());
	double difY = (double)(jugadorY - centroY());
	double dist = sqrt(difX * difX + difY * difY);

	if (embestida > 0) {
		// En plena embestida: corre rapido en linea recta hacia el jugador
		embestida--;
		x += (int)(difX / (dist < 1 ? 1 : dist) * (velocidad + 6));
		y += (int)(difY / (dist < 1 ? 1 : dist) * (velocidad + 6));
		recarga = 60;
	}
	else {
		if (recarga > 0) recarga--;
		if (dist < radioPersecucion && recarga == 0) {
			embestida = 14; // inicia el salto/embestida
		}
		else if (dist < radioPersecucion) {
			irHacia(jugadorX - ancho / 2, jugadorY - alto / 2); // acecha
		}
		else {
			// Patrulla suave
			if (pausa > 0) pausa--;
			else {
				double dox = (double)(objetivoX - x);
				double doy = (double)(objetivoY - y);
				if (sqrt(dox * dox + doy * doy) < velocidad * 2) {
					objetivoX = baseX + (rand() % (radioPatrulla * 2)) - radioPatrulla;
					objetivoY = baseY + (rand() % (radioPatrulla * 2)) - radioPatrulla;
					pausa = 20 + rand() % 40;
				}
				else irHacia(objetivoX, objetivoY);
			}
		}
	}

	delay++;
	if (delay >= 3) { delay = 0; IDx = (IDx + 1) % 12; }
}

Rectangle LoboLava::hitbox() {
	return Rectangle(x + ancho / 8, y + alto / 4, ancho * 3 / 4, alto * 3 / 5);
}

void LoboLava::mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY) {
	if (!activo) return;
	int px = x - camaraX;
	int py = y - camaraY;
	int fuego = (IDx + semilla) % 12;
	bool saltando = embestida > 0;

	// Sombra
	g->FillEllipse(Brushes::Black, px + ancho / 6, py + alto - 10, ancho * 2 / 3, 12);

	// Cuerpo cuadrupedo (rocas oscuras)
	g->FillRectangle(Brushes::DimGray, px + ancho / 5, py + alto / 3, ancho * 3 / 5, alto / 3);
	// Cabeza
	g->FillRectangle(Brushes::DimGray, px + ancho - ancho / 4, py + alto / 4, ancho / 4, alto / 3);
	// Patas (se estiran al saltar)
	int pata = saltando ? alto / 2 : alto / 3;
	g->FillRectangle(Brushes::Gray, px + ancho / 4, py + alto - pata, 10, pata);
	g->FillRectangle(Brushes::Gray, px + ancho / 2, py + alto - pata, 10, pata);

	// Melena y cola de fuego (se agitan)
	array<Point>^ melena = {
		Point(px + ancho - ancho / 5, py + alto / 5),
		Point(px + ancho - ancho / 3 - (fuego % 4) * 2, py - 6),
		Point(px + ancho / 2, py + alto / 4),
		Point(px + ancho / 2 + 8, py + alto / 6 - (fuego % 3) * 2)
	};
	g->FillPolygon(Brushes::OrangeRed, melena);
	array<Point>^ cola = {
		Point(px + ancho / 5, py + alto / 3),
		Point(px - 6 - (fuego % 4) * 3, py + alto / 6),
		Point(px + ancho / 8, py + alto / 2)
	};
	g->FillPolygon(Brushes::Orange, cola);

	// Grietas de lava en el lomo
	Pen^ grieta = gcnew Pen(Color::FromArgb(255, 255, 110, 0), 2.0f);
	g->DrawLine(grieta, px + ancho / 4, py + alto / 2, px + ancho - ancho / 4, py + alto / 2);
	delete grieta;

	// Ojo
	g->FillRectangle(saltando ? Brushes::Gold : Brushes::OrangeRed,
		px + ancho - ancho / 8, py + alto / 3 + 4, 9, 8);
}
