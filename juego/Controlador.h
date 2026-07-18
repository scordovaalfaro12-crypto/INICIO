#pragma once
#include "Submarino.h"
#include "Tesoro.h"
#include "PezEnemigo.h"
#include <ctime>
using namespace System::Windows::Forms;

class Controlador {
private:
	Bitmap^ imgSubmarino;
	Submarino* submarino;

	Bitmap^ imgTesoro;
	TesoroList* tesoros;

	Bitmap^ imgPez;
	PezEnemigoList* pezEnemigo;

public:
	Controlador();
	void moverSubmarino(bool value, Keys tecla);
	void mover(Graphics^ g);
	void mostrar(Graphics^ g);
};
