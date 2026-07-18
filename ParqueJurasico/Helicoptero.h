#pragma once
using namespace System;
using namespace System::Drawing;

// Helicoptero de apoyo: patrulla la parte superior de la zona de juego.
// Cuando el guardabosques colisiona con el (rubrica: colision
// Guardabosques-helicoptero -> vida y dardos), recibe +1 vida y +3 dardos;
// el helicoptero se retira y regresa despues de unos segundos.
// Sprite: _helicoptero.png (hoja 4x4, frames de 256x256).
class Helicoptero
{
private:
	int x, y;
	int ancho, alto;
	int velocidad;
	int dx;
	int frame;
	int contadorAnim;
	bool activo;
	int ticksReaparecer;

	static const int FRAME_ANCHO = 256;
	static const int FRAME_ALTO = 256;
	static const int TOTAL_FRAMES = 4;

public:
	Helicoptero(Rectangle zona);
	~Helicoptero();

	void mover(Rectangle zona);
	void recoger();          // se retira temporalmente tras entregar el apoyo
	bool estaActivo();
	void dibujar(Graphics^ g, Bitmap^ bmp);
	Rectangle getRectangulo();
};
