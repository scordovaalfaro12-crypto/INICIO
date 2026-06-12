#include "Entidad.h"

Entidad::Entidad() {
	x = y = 0;
	dx = dy = 0;
	ancho = alto = 0;
	vida = 3; // vida por defecto
	IDx = 0;
	delay = 0;
	activo = true;
}

Entidad::~Entidad() {}

int Entidad::getX() { return x; }
int Entidad::getY() { return y; }
int Entidad::getDX() { return dx; }
int Entidad::getDY() { return dy; }
int Entidad::getAncho() { return ancho; }
int Entidad::getAlto() { return alto; }
int Entidad::getVida() { return vida; }
bool Entidad::getActivo() { return activo; }

void Entidad::setX(int value) { x = value; }
void Entidad::setY(int value) { y = value; }
void Entidad::setDX(int value) { dx = value; }
void Entidad::setDY(int value) { dy = value; }
void Entidad::setAncho(int value) { ancho = value; }
void Entidad::setAlto(int value) { alto = value; }
void Entidad::setVida(int value) { vida = value; }
void Entidad::setActivo(bool value) { activo = value; }

int Entidad::centroX() { return x + ancho / 2; }
int Entidad::centroY() { return y + alto / 2; }

double Entidad::distanciaA(Entidad* otra) {
	double difX = (double)(centroX() - otra->centroX());
	double difY = (double)(centroY() - otra->centroY());
	return sqrt(difX * difX + difY * difY);
}

Rectangle Entidad::area() { return Rectangle(x, y, ancho, alto); }

Rectangle Entidad::hitbox() {
	// Por defecto reduce un poco el area para colisiones mas justas
	return Rectangle(x + ancho / 6, y + alto / 6, ancho * 2 / 3, alto * 2 / 3);
}

void Entidad::mover(Graphics^ g) {
	x += dx;
	y += dy;
}

void Entidad::mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY) {
	g->DrawRectangle(Pens::Black, Rectangle(x - camaraX, y - camaraY, ancho, alto));
}
