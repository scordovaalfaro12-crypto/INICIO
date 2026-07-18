#include "Controlador.h"

Controlador::Controlador() {
	srand((unsigned int)time(NULL));

	imgSubmarino = gcnew Bitmap("img/submarino.png");
	submarino = new Submarino(imgSubmarino);

	imgTesoro = gcnew Bitmap("img/Tesoros.png");
	tesoros = new TesoroList(5, imgTesoro);

	imgPez = gcnew Bitmap("img/PezEnemigo.png");
	pezEnemigo = new PezEnemigoList(5, imgPez);
}

void Controlador::moverSubmarino(bool value, Keys tecla) {
	int velocidad = 5;
	if (value == true) {
		if (tecla == Keys::W) {
			submarino->setDY(-velocidad);
			submarino->setAccion(Submarino::arriba);
		}
		else if (tecla == Keys::S) {
			submarino->setDY(velocidad);
			submarino->setAccion(Submarino::abajo);
		}
		else if (tecla == Keys::A) {
			submarino->setDX(-velocidad);
			submarino->setAccion(Submarino::izquierda);
		}
		else if (tecla == Keys::D) {
			submarino->setDX(velocidad);
			submarino->setAccion(Submarino::derecha);
		}
		else if (tecla == Keys::P) {
			pezEnemigo->agregar(3, imgPez);
		}
	}
	else {
		if (tecla == Keys::W) {
			submarino->setDY(0);
			submarino->setAccion(Submarino::arriba);
		}
		else if (tecla == Keys::S) {
			submarino->setDY(0);
			submarino->setAccion(Submarino::abajo);
		}
		else if (tecla == Keys::A) {
			submarino->setDX(0);
			submarino->setAccion(Submarino::izquierda);
		}
		else if (tecla == Keys::D) {
			submarino->setDX(0);
			submarino->setAccion(Submarino::derecha);
		}
	}
}

void Controlador::mover(Graphics^ g) {
	submarino->mover(g);
	pezEnemigo->mover(g);
}

void Controlador::mostrar(Graphics^ g) {
	pezEnemigo->mostrar(g, imgPez);
	submarino->mostrar(g, imgSubmarino);
	tesoros->mostrar(g, imgTesoro);
}
