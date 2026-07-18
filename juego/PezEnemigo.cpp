#include "PezEnemigo.h"

PezEnemigo::PezEnemigo(Bitmap^ img) {

	ancho = img->Width / 4;
	alto = img->Height / 2;

	a = 1 + rand() % 2;

	if (a == 1)
		dx = 5;
	else
		dx = -5;

	if (dx < 0) {
		accion = izquierda;
	}
	else {
		accion = derecha;
	}

	x = 300 + rand() % 300;
	y = 150 + rand() % 200;

}

void PezEnemigo::mover(Graphics^ g) {

	// Rebota solo cuando el siguiente paso sale de los limites de la pantalla
	if (x + dx <= 0 || x + dx + ancho >= g->VisibleClipBounds.Width) {
		dx *= -1;
	}

	x += dx;

	// Actualiza el sprite segun la direccion actual
	if (dx < 0) {
		accion = izquierda;
	}
	else {
		accion = derecha;
	}

}

void PezEnemigo::mostrar(Graphics^ g, Bitmap^ img) {
	Rectangle corte = Rectangle(ancho * IDx, alto * accion, ancho, alto);
	g->DrawImage(img, area(), corte, GraphicsUnit::Pixel);

	delay++;
	if (delay >= 6)
	{
		delay = 0;
		IDx = (IDx + 1) % 4;
	}

}

PezEnemigoList::PezEnemigoList(int cant, Bitmap^ img) {
	for (int i = 0; i < cant; i++) {
		PezEnemigo* p = new PezEnemigo(img);
		pezEnemigo.push_back(p);
	}
}

PezEnemigoList::~PezEnemigoList() {
	for (PezEnemigo* p : pezEnemigo) {
		delete p;
	}
	pezEnemigo.clear();
}

void PezEnemigoList::mover(Graphics^ g) {
	for (PezEnemigo* p : pezEnemigo) {
		p->mover(g);
	}
}

void PezEnemigoList::mostrar(Graphics^ g, Bitmap^ img) {
	for (PezEnemigo* p : pezEnemigo) {
		p->mostrar(g, img);
	}
}

void PezEnemigoList::agregar(int cant, Bitmap^ img) {
	for (int i = 0; i < cant; i++) {
		PezEnemigo* p = new PezEnemigo(img);
		pezEnemigo.push_back(p);
	}
}
