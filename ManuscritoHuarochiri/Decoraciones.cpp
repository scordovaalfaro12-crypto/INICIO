#include "Decoraciones.h"

// ------------------------- Decoracion -------------------------

Decoracion::Decoracion(int px, int py) {
	x = px;
	y = py;
	ancho = alto = 60;
	semilla = rand() % 97;
}

void Decoracion::animar() {
	delay++;
	if (delay >= 3) {
		delay = 0;
		IDx = (IDx + 1) % 24;
	}
}

// --------------------- AntorchaDecorativa ---------------------

AntorchaDecorativa::AntorchaDecorativa(int px, int py) : Decoracion(px, py) {
	ancho = 44;
	alto = 110;
}

void AntorchaDecorativa::mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY) {
	int px = x - camaraX;
	int py = y - camaraY;
	int parpadeo = (IDx + semilla) % 6;

	// Halo de luz que late
	SolidBrush^ halo = gcnew SolidBrush(Color::FromArgb(55 + parpadeo * 8, 255, 150, 40));
	g->FillEllipse(halo, px - 40, py - 36, ancho + 80, 90);
	delete halo;

	// Poste de piedra
	g->FillRectangle(Brushes::DimGray, px + ancho / 2 - 7, py + 34, 14, alto - 34);
	g->FillRectangle(Brushes::DarkSlateGray, px + ancho / 2 - 12, py + 26, 24, 12);

	// Llama en capas (se agita con el parpadeo)
	int lengua = parpadeo - 3; // -3..2
	array<Point>^ llama = {
		Point(px + ancho / 2 + lengua * 2, py - 12 + (parpadeo % 3) * 2),
		Point(px + ancho / 2 + 14, py + 18),
		Point(px + ancho / 2, py + 34),
		Point(px + ancho / 2 - 14, py + 18)
	};
	g->FillPolygon(Brushes::OrangeRed, llama);
	array<Point>^ nucleo = {
		Point(px + ancho / 2 - lengua, py + 2 + (parpadeo % 2) * 3),
		Point(px + ancho / 2 + 8, py + 22),
		Point(px + ancho / 2, py + 32),
		Point(px + ancho / 2 - 8, py + 22)
	};
	g->FillPolygon(Brushes::Gold, nucleo);

	// Chispa que sube
	int chispaY = py - 6 - ((IDx * 3 + semilla) % 26);
	g->FillRectangle(Brushes::Yellow, px + ancho / 2 + lengua * 3, chispaY, 4, 4);
}

// -------------------------- SolAndino --------------------------

SolAndino::SolAndino(int px, int py) : Decoracion(px, py) {
	ancho = alto = 200;
}

void SolAndino::mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY) {
	int cx = x - camaraX + ancho / 2;
	int cy = y - camaraY + alto / 2;
	double angulo = IDx * 0.13 + semilla;
	int pulso = (IDx % 12 < 6) ? IDx % 12 : 12 - IDx % 12;

	// Halo
	SolidBrush^ halo = gcnew SolidBrush(Color::FromArgb(50, 255, 215, 0));
	g->FillEllipse(halo, cx - 90 - pulso, cy - 90 - pulso, 180 + pulso * 2, 180 + pulso * 2);
	delete halo;

	// Rayos que giran (alternando largo y corto, como el sol de Echenique)
	Pen^ rayo = gcnew Pen(Color::Gold, 6.0f);
	for (int i = 0; i < 12; i++) {
		double a = angulo + i * 3.1416 / 6.0;
		int largo = (i % 2 == 0) ? 86 : 64;
		g->DrawLine(rayo,
			cx + (int)(46 * cos(a)), cy + (int)(46 * sin(a)),
			cx + (int)(largo * cos(a)), cy + (int)(largo * sin(a)));
	}
	delete rayo;

	// Disco central con cara estilizada
	g->FillEllipse(Brushes::Gold, cx - 44, cy - 44, 88, 88);
	g->FillEllipse(Brushes::Orange, cx - 34, cy - 34, 68, 68);
	g->FillRectangle(Brushes::SaddleBrown, cx - 16, cy - 10, 9, 12);  // ojo izq
	g->FillRectangle(Brushes::SaddleBrown, cx + 7, cy - 10, 9, 12);   // ojo der
	g->FillRectangle(Brushes::SaddleBrown, cx - 10, cy + 12, 20, 6);  // boca
}

// ------------------------- BurbujaLava -------------------------

BurbujaLava::BurbujaLava(int px, int py) : Decoracion(px, py) {
	ancho = alto = 70;
}

void BurbujaLava::mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY) {
	int px = x - camaraX;
	int py = y - camaraY;
	int fase = (IDx + semilla) % 24;

	// Resplandor de la lava que late siempre
	SolidBrush^ brillo = gcnew SolidBrush(Color::FromArgb(40 + (fase % 8) * 6, 255, 90, 0));
	g->FillEllipse(brillo, px - 22, py - 12, ancho + 44, alto + 24);
	delete brillo;

	if (fase < 18) {
		// La burbuja crece
		int radio = 6 + fase;
		g->FillEllipse(Brushes::OrangeRed, px + ancho / 2 - radio, py + alto / 2 - radio, radio * 2, radio * 2);
		g->FillEllipse(Brushes::Orange, px + ancho / 2 - radio / 2, py + alto / 2 - radio / 2 - 3, radio, radio);
		g->FillEllipse(Brushes::Yellow, px + ancho / 2 - 4, py + alto / 2 - radio / 2 - 2, 7, 7);
	}
	else {
		// Revienta: salpicaduras
		int s = (fase - 18) * 5;
		g->FillEllipse(Brushes::Orange, px + ancho / 2 - 24 - s / 2, py + alto / 2 - 6, 12, 12);
		g->FillEllipse(Brushes::Orange, px + ancho / 2 + 12 + s / 2, py + alto / 2 - 8, 12, 12);
		g->FillEllipse(Brushes::Gold, px + ancho / 2 - 6, py + alto / 2 - 18 - s, 12, 12);
		g->FillEllipse(Brushes::OrangeRed, px + ancho / 2 - 10, py + alto / 2 - 2, 20, 12);
	}
}

// ------------------------- DestelloAgua -------------------------

DestelloAgua::DestelloAgua(int px, int py) : Decoracion(px, py) {
	ancho = alto = 50;
}

void DestelloAgua::mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY) {
	int fase = (IDx + semilla) % 24;
	if (fase > 12) return; // la mitad del tiempo descansa

	int px = x - camaraX + ancho / 2;
	int py = y - camaraY + alto / 2;
	int brillo = (fase < 7) ? fase : 12 - fase; // 0..6..0
	int largo = 4 + brillo * 2;

	Pen^ blanco = gcnew Pen(Color::White, 3.0f);
	Pen^ celeste = gcnew Pen(Color::Cyan, 2.0f);
	g->DrawLine(blanco, px - largo, py, px + largo, py);
	g->DrawLine(blanco, px, py - largo, px, py + largo);
	g->DrawLine(celeste, px - largo / 2, py - largo / 2, px + largo / 2, py + largo / 2);
	g->DrawLine(celeste, px - largo / 2, py + largo / 2, px + largo / 2, py - largo / 2);
	delete blanco;
	delete celeste;
}

// ---------------------- PersonajeSecundario ----------------------

PersonajeSecundario::PersonajeSecundario(int px, int py, int anchoFrame, int altoFrame) {
	x = px;
	y = py;
	ancho = anchoFrame;
	alto = altoFrame;
	frameAnim = 0;
}

void PersonajeSecundario::animar() {
	delay++;
	if (delay >= 8) {
		delay = 0;
		frameAnim = (frameAnim + 1) % 4; // frames de respiracion
		IDx = (IDx + 1) % 16;            // ciclo del flote
	}
}

void PersonajeSecundario::mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY) {
	int flote = (int)(7.0 * sin(IDx * 3.1416 / 8.0));
	int px = x - camaraX;
	int py = y - camaraY + flote;

	// Aura espiritual en el suelo
	SolidBrush^ aura = gcnew SolidBrush(Color::FromArgb(70, 120, 230, 255));
	g->FillEllipse(aura, px + ancho / 4, y - camaraY + alto - 16, ancho / 2, 20);
	delete aura;

	// Sprite de la deidad en reposo, dibujado translucido y calido (fantasma)
	System::Drawing::Imaging::ColorMatrix^ matriz = gcnew System::Drawing::Imaging::ColorMatrix();
	matriz->Matrix33 = 0.62f; // transparencia
	matriz->Matrix40 = 0.20f; // un toque dorado (suma rojo)
	matriz->Matrix41 = 0.14f; // y verde
	System::Drawing::Imaging::ImageAttributes^ atributos = gcnew System::Drawing::Imaging::ImageAttributes();
	atributos->SetColorMatrix(matriz);

	Rectangle destino = Rectangle(px, py, ancho, alto);
	g->DrawImage(img, destino, frameAnim * ancho, 4 * alto, ancho, alto,
		GraphicsUnit::Pixel, atributos);
	delete atributos;

	// Rombo dorado flotante: senal de que tiene algo que contar
	int rx = px + ancho / 2;
	int ry = py - 30 + (int)(3.0 * sin(IDx * 3.1416 / 4.0));
	array<Point>^ rombo = {
		Point(rx, ry - 10), Point(rx + 8, ry), Point(rx, ry + 10), Point(rx - 8, ry)
	};
	g->FillPolygon(Brushes::Gold, rombo);
	g->FillRectangle(Brushes::OrangeRed, rx - 2, ry - 4, 4, 5);
	g->FillRectangle(Brushes::OrangeRed, rx - 2, ry + 3, 4, 3);
}

// --------------------------- BolaFuego ---------------------------

BolaFuego::BolaFuego(int px, int py, int objetivoX, int objetivoY) {
	ancho = alto = 52;
	x = px - ancho / 2;
	y = py - alto / 2;
	double difX = (double)(objetivoX - px);
	double difY = (double)(objetivoY - py);
	double dist = sqrt(difX * difX + difY * difY);
	if (dist < 1) dist = 1;
	dx = (int)(difX / dist * 10);
	dy = (int)(difY / dist * 10);
	if (dx == 0 && dy == 0) dy = 10;
	vidaUtil = 130;
}

bool BolaFuego::terminado() { return vidaUtil <= 0 || !activo; }

void BolaFuego::mover(Graphics^ g) {
	x += dx;
	y += dy;
	vidaUtil--;
	delay++;
	if (delay >= 2) { delay = 0; IDx = (IDx + 1) % 8; }
}

Rectangle BolaFuego::hitbox() {
	return Rectangle(x + 8, y + 8, ancho - 16, alto - 16);
}

void BolaFuego::mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY) {
	if (terminado()) return;
	int px = x - camaraX;
	int py = y - camaraY;

	// Estela detras de la bola
	g->FillEllipse(Brushes::Firebrick, px - dx, py - dy, ancho, alto);
	g->FillEllipse(Brushes::OrangeRed, px - dx / 2, py - dy / 2, ancho, alto);

	// Nucleo
	int pulso = (IDx % 4 < 2) ? 4 : 0;
	g->FillEllipse(Brushes::Orange, px - pulso / 2, py - pulso / 2, ancho + pulso, alto + pulso);
	g->FillEllipse(Brushes::Gold, px + 9, py + 9, ancho - 18, alto - 18);
	g->FillEllipse(Brushes::White, px + 18, py + 14, 12, 10);
}
