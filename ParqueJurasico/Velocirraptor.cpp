#include "Velocirraptor.h"

Velocirraptor::Velocirraptor(int x, int y)
	: Dinosaurio(x, y, 60, 60, 2 + rand() % 3, 100, 100, 4)   // velocidad aleatoria 2..4
{
	// Eje de desplazamiento aleatorio: horizontal o vertical
	if (rand() % 2 == 0)
	{
		dx = (rand() % 2 == 0) ? velocidad : -velocidad;
		dy = 0;
	}
	else
	{
		dx = 0;
		dy = (rand() % 2 == 0) ? velocidad : -velocidad;
	}
	actualizarFila();
}

void Velocirraptor::mover(Rectangle zona)
{
	x += dx;
	y += dy;
	if (dx != 0)
	{
		if (x <= zona.X)                  { x = zona.X; dx = velocidad; }
		else if (x + ancho >= zona.Right) { x = zona.Right - ancho; dx = -velocidad; }
	}
	if (dy != 0)
	{
		if (y <= zona.Y)                  { y = zona.Y; dy = velocidad; }
		else if (y + alto >= zona.Bottom) { y = zona.Bottom - alto; dy = -velocidad; }
	}
	actualizarFila();
	avanzarAnimacion();
}

int Velocirraptor::getTipo()
{
	return 0;
}

void Velocirraptor::actualizarFila()
{
	// Filas de _dinosaurio2.png: 0 espalda (arriba), 1 izquierda, 2 derecha, 3 frente (abajo)
	if (dx < 0)      fila = 1;
	else if (dx > 0) fila = 2;
	else if (dy < 0) fila = 0;
	else if (dy > 0) fila = 3;
}
