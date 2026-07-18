#include "Helicoptero.h"

Helicoptero::Helicoptero(Rectangle zona)
{
	ancho = 90;
	alto = 90;
	velocidad = 3;
	x = zona.X + 10;
	y = zona.Y + 5;
	dx = velocidad;
	frame = 0;
	contadorAnim = 0;
	activo = true;
	ticksReaparecer = 0;
}

Helicoptero::~Helicoptero()
{
}

void Helicoptero::mover(Rectangle zona)
{
	if (!activo)
	{
		ticksReaparecer--;
		if (ticksReaparecer <= 0)
		{
			activo = true;
			x = zona.X + 10;
			dx = velocidad;
		}
		return;
	}
	x += dx;
	if (x <= zona.X)                  { x = zona.X; dx = velocidad; }
	else if (x + ancho >= zona.Right) { x = zona.Right - ancho; dx = -velocidad; }

	// Las helices giran siempre
	contadorAnim++;
	if (contadorAnim >= 3)
	{
		contadorAnim = 0;
		frame = (frame + 1) % TOTAL_FRAMES;
	}
}

void Helicoptero::recoger()
{
	activo = false;
	ticksReaparecer = 400;   // regresa despues de ~12 segundos (400 ticks x 30 ms)
}

bool Helicoptero::estaActivo()
{
	return activo;
}

void Helicoptero::dibujar(Graphics^ g, Bitmap^ bmp)
{
	if (!activo) return;
	// Filas de _helicoptero.png: 1 mira a la izquierda, 2 mira a la derecha
	int fila = (dx < 0) ? 1 : 2;
	Rectangle destino(x, y, ancho, alto);
	Rectangle origen(frame * FRAME_ANCHO, fila * FRAME_ALTO, FRAME_ANCHO, FRAME_ALTO);
	g->DrawImage(bmp, destino, origen, GraphicsUnit::Pixel);
}

Rectangle Helicoptero::getRectangulo()
{
	return Rectangle(x, y, ancho, alto);
}
