#include "Caja.h"

Caja::Caja(int x, int y)
{
	this->x = x;
	this->y = y;
	tam = 22;
}

Caja::~Caja()
{
}

void Caja::dibujar(Graphics^ g)
{
	// Cuadrado de color rojo (segun el enunciado)
	g->FillRectangle(Brushes::Red, x, y, tam, tam);
	Pen^ borde = gcnew Pen(Color::DarkRed, 2);
	g->DrawRectangle(borde, x, y, tam, tam);
	delete borde;
}

Rectangle Caja::getRectangulo()
{
	return Rectangle(x, y, tam, tam);
}
