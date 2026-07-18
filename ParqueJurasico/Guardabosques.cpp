#include "Guardabosques.h"

Guardabosques::Guardabosques(int x, int y)
{
	this->x = x;
	this->y = y;
	ancho = 52;
	alto = 52;
	velocidad = 5;
	fila = 0;          // inicia mirando hacia abajo
	frame = 0;
	contadorAnim = 0;
	vidas = 3;
	dardos = 0;        // los dardos se consiguen recogiendo cajas
}

Guardabosques::~Guardabosques()
{
}

void Guardabosques::mover(bool arriba, bool abajo, bool izquierda, bool derecha, Rectangle zona)
{
	int dx = 0, dy = 0;
	if (izquierda)      { dx = -velocidad; fila = 1; }
	else if (derecha)   { dx = velocidad;  fila = 2; }
	if (arriba)         { dy = -velocidad; fila = 3; }
	else if (abajo)     { dy = velocidad;  fila = 0; }

	x += dx;
	y += dy;
	if (x < zona.X)              x = zona.X;
	if (x + ancho > zona.Right)  x = zona.Right - ancho;
	if (y < zona.Y)              y = zona.Y;
	if (y + alto > zona.Bottom)  y = zona.Bottom - alto;

	if (dx != 0 || dy != 0)
	{
		contadorAnim++;
		if (contadorAnim >= 5)
		{
			contadorAnim = 0;
			frame = (frame + 1) % TOTAL_FRAMES;
		}
	}
	else
	{
		frame = 0;   // quieto: pose de reposo
	}
}

void Guardabosques::dibujar(Graphics^ g, Bitmap^ bmp)
{
	Rectangle destino(x, y, ancho, alto);
	Rectangle origen(frame * FRAME_ANCHO, fila * FRAME_ALTO, FRAME_ANCHO, FRAME_ALTO);
	g->DrawImage(bmp, destino, origen, GraphicsUnit::Pixel);
}

Rectangle Guardabosques::getRectangulo()
{
	return Rectangle(x, y, ancho, alto);
}

int Guardabosques::getDireccion()
{
	return fila;
}

int Guardabosques::getCentroX()
{
	return x + ancho / 2;
}

int Guardabosques::getCentroY()
{
	return y + alto / 2;
}

bool Guardabosques::usarDardo()
{
	if (dardos > 0)
	{
		dardos--;
		return true;
	}
	return false;
}

void Guardabosques::agregarDardos(int cantidad)
{
	dardos += cantidad;
}

void Guardabosques::quitarVida()
{
	if (vidas > 0) vidas--;
}

void Guardabosques::agregarVida()
{
	if (vidas < VIDAS_MAXIMAS) vidas++;
}

int Guardabosques::getVidas()
{
	return vidas;
}

int Guardabosques::getDardos()
{
	return dardos;
}
