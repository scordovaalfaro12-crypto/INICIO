#include "ObjetosNivel.h"

// ------------------------- NodoDato -------------------------

NodoDato::NodoDato(int px, int py, int id) {
	x = px;
	y = py;
	fragmentoID = id;
	ancho = 90;
	alto = 90;
}

int NodoDato::getFragmentoID() { return fragmentoID; }

void NodoDato::animar() {
	delay++;
	if (delay >= 4) { delay = 0; IDx = (IDx + 1) % 16; }
}

void NodoDato::mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY) {
	if (!activo) return;
	// Flota suavemente arriba y abajo
	int flote = (int)(8.0 * sin(IDx * 3.1416 / 8.0));
	int px = x - camaraX;
	int py = y - camaraY + flote;

	// Halo exterior pulsante
	int halo = (IDx % 8 < 4) ? IDx % 8 : 8 - IDx % 8;
	g->FillEllipse(Brushes::Teal, px - halo, py - halo, ancho + halo * 2, alto + halo * 2);
	g->FillEllipse(Brushes::Cyan, px + 8, py + 8, ancho - 16, alto - 16);
	g->FillEllipse(Brushes::White, px + 26, py + 22, 22, 16);

	// Glifo de datos [O] del manuscrito
	g->FillRectangle(Brushes::DarkSlateGray, px + ancho / 2 - 17, py + alto / 2 - 13, 34, 28);
	g->DrawRectangle(Pens::White, px + ancho / 2 - 17, py + alto / 2 - 13, 34, 28);
	g->DrawEllipse(Pens::Cyan, px + ancho / 2 - 8, py + alto / 2 - 6, 16, 14);
}

// ---------------------- PilarInformacion ----------------------

PilarInformacion::PilarInformacion(int px, int py) {
	x = px;
	y = py;
	progreso = 0;
	corrompiendo = false;
	ancho = 130;
	alto = 210;
}

double PilarInformacion::getProgreso() { return progreso; }

void PilarInformacion::cargar(double cantidad) {
	if (progreso >= 100) return;
	progreso += cantidad;
	if (progreso > 100) progreso = 100;
}

void PilarInformacion::drenar(double cantidad) {
	if (progreso >= 100) return; // un pilar compilado ya es indestructible
	progreso -= cantidad;
	if (progreso < 0) progreso = 0;
}

bool PilarInformacion::estaCompleto() { return progreso >= 100; }
bool PilarInformacion::getCorrompiendo() { return corrompiendo; }
void PilarInformacion::setCorrompiendo(bool value) { corrompiendo = value; }

void PilarInformacion::animar() {
	delay++;
	if (delay >= 5) { delay = 0; IDx = (IDx + 1) % 12; }
}

void PilarInformacion::mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY) {
	int px = x - camaraX;
	int py = y - camaraY;

	// Aura segun estado: verde = completo, rojo = corrompiendose, cian = cargando
	Brush^ aura;
	if (estaCompleto()) aura = Brushes::LimeGreen;
	else if (corrompiendo) aura = Brushes::Red;
	else aura = Brushes::Teal;
	int pulso = (IDx % 6 < 3) ? IDx % 6 : 6 - IDx % 6;
	g->FillEllipse(aura, px - 14 - pulso, py + alto - 36, ancho + 28 + pulso * 2, 44);

	// Cuerpo de piedra del pilar
	g->FillRectangle(Brushes::DimGray, px + 20, py + 24, ancho - 40, alto - 44);
	g->FillRectangle(Brushes::Gray, px + 28, py + 24, 18, alto - 44);
	g->FillRectangle(Brushes::DarkSlateGray, px + 10, py + 8, ancho - 20, 26);
	g->FillRectangle(Brushes::DarkSlateGray, px + 6, py + alto - 28, ancho - 12, 22);
	g->DrawRectangle(Pens::Black, px + 20, py + 24, ancho - 40, alto - 44);

	// Runas de datos que suben cuando carga
	Brush^ runa = estaCompleto() ? Brushes::LimeGreen : Brushes::Cyan;
	for (int i = 0; i < 4; i++) {
		int ry = py + alto - 50 - ((IDx * 6 + i * 40) % (alto - 70));
		if (progreso > i * 25) g->FillRectangle(runa, px + ancho / 2 - 8, ry, 16, 10);
	}

	// Barra de progreso de compilacion
	int barra = ancho + 10;
	int lleno = (int)(progreso / 100.0 * barra);
	g->FillRectangle(Brushes::Black, px - 7, py - 22, barra + 4, 14);
	g->FillRectangle(estaCompleto() ? Brushes::LimeGreen : (corrompiendo ? Brushes::Red : Brushes::Cyan), px - 5, py - 20, lleno, 10);
	g->DrawRectangle(Pens::White, px - 7, py - 22, barra + 4, 14);
}

// --------------------- SantuarioMacahuisa ---------------------

SantuarioMacahuisa::SantuarioMacahuisa(int px, int py) {
	ancho = 300;
	alto = 240;
	x = px - ancho / 2;
	y = py - alto / 2;
}

void SantuarioMacahuisa::animar() {
	delay++;
	if (delay >= 4) { delay = 0; IDx = (IDx + 1) % 16; }
}

void SantuarioMacahuisa::mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY) {
	int px = x - camaraX;
	int py = y - camaraY;
	int cx = px + ancho / 2;

	// Anillo dorado pulsante que marca la zona de entrega
	int pulso = (IDx % 8 < 4) ? (IDx % 8) * 3 : (8 - IDx % 8) * 3;
	Pen^ anillo = gcnew Pen(Color::Gold, 6.0f);
	g->DrawEllipse(anillo, px - pulso, py + alto / 4 - pulso / 2, ancho + pulso * 2, alto * 3 / 4 + pulso);
	delete anillo;

	// Mini piramide escalonada (el santuario)
	g->FillRectangle(Brushes::DarkKhaki, cx - 90, py + alto - 60, 180, 36);
	g->FillRectangle(Brushes::Khaki, cx - 64, py + alto - 92, 128, 36);
	g->FillRectangle(Brushes::DarkKhaki, cx - 40, py + alto - 122, 80, 34);
	g->FillRectangle(Brushes::Gold, cx - 14, py + alto - 152, 28, 32);
	g->DrawRectangle(Pens::Black, cx - 90, py + alto - 60, 180, 36);
	g->DrawRectangle(Pens::Black, cx - 64, py + alto - 92, 128, 36);
	g->DrawRectangle(Pens::Black, cx - 40, py + alto - 122, 80, 34);

	// Idolo dorado en la cima
	g->FillEllipse(Brushes::Gold, cx - 10, py + alto - 170, 20, 20);
}

// ---------------------- AliadoAlgoritmo ----------------------

AliadoAlgoritmo::AliadoAlgoritmo() {
	ancho = 56;
	alto = 56;
}

void AliadoAlgoritmo::seguir(int jugadorX, int jugadorY) {
	// Flota detras del hombro del jugador, con un poco de retraso
	int destinoX = jugadorX - 70;
	int destinoY = jugadorY - 60;
	x += (destinoX - x) / 8;
	y += (destinoY - y) / 8;
	delay++;
	if (delay >= 4) { delay = 0; IDx = (IDx + 1) % 16; }
}

void AliadoAlgoritmo::mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY) {
	int flote = (int)(6.0 * sin(IDx * 3.1416 / 8.0));
	int px = x - camaraX;
	int py = y - camaraY + flote;

	g->FillEllipse(Brushes::MediumSpringGreen, px, py, ancho, alto);
	g->FillEllipse(Brushes::PaleGreen, px + 10, py + 8, ancho - 26, alto - 30);
	// Ojos del espiritu
	g->FillRectangle(Brushes::DarkGreen, px + 16, py + 22, 7, 10);
	g->FillRectangle(Brushes::DarkGreen, px + 32, py + 22, 7, 10);
	// Bits que orbitan
	int bx = (int)(34.0 * cos(IDx * 3.1416 / 8.0));
	int by = (int)(20.0 * sin(IDx * 3.1416 / 8.0));
	g->FillRectangle(Brushes::Lime, px + ancho / 2 + bx - 4, py + alto / 2 + by - 4, 8, 8);
	g->FillRectangle(Brushes::Lime, px + ancho / 2 - bx - 4, py + alto / 2 - by - 4, 8, 8);
}
