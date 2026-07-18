#include "Dardo.h"

Dardo::Dardo(int centroX, int centroY, int direccion)
{
	ancho = 22;
	alto = 22;
	velocidad = 12;
	this->direccion = direccion;
	x = centroX - ancho / 2;
	y = centroY - alto / 2;
	frame = 0;
	contadorAnim = 0;
	activo = true;
}

Dardo::~Dardo()
{
}

void Dardo::mover()
{
	switch (direccion)
	{
	case 0: y += velocidad; break;   // abajo
	case 1: x -= velocidad; break;   // izquierda
	case 2: x += velocidad; break;   // derecha
	case 3: y -= velocidad; break;   // arriba
	}
	contadorAnim++;
	if (contadorAnim >= 4)
	{
		contadorAnim = 0;
		frame = (frame + 1) % TOTAL_FRAMES;
	}
}

bool Dardo::estaFuera(Rectangle zona)
{
	return (x + ancho < zona.X || x > zona.Right ||
	        y + alto < zona.Y || y > zona.Bottom);
}

void Dardo::dibujar(Graphics^ g, Bitmap^ bmp)
{
	// Filas de _dardo.png: 0 apunta abajo, 1 apunta derecha, 2 apunta izquierda, 3 apunta arriba
	int filaSprite;
	switch (direccion)
	{
	case 0:  filaSprite = 0; break;
	case 1:  filaSprite = 2; break;
	case 2:  filaSprite = 1; break;
	default: filaSprite = 3; break;
	}
	Rectangle destino(x, y, ancho, alto);
	Rectangle origen(frame * FRAME_ANCHO, filaSprite * FRAME_ALTO, FRAME_ANCHO, FRAME_ALTO);
	g->DrawImage(bmp, destino, origen, GraphicsUnit::Pixel);
}

Rectangle Dardo::getRectangulo()
{
	return Rectangle(x, y, ancho, alto);
}

void Dardo::desactivar()
{
	activo = false;
}

bool Dardo::estaActivo()
{
	return activo;
}
