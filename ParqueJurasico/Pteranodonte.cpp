#include "Pteranodonte.h"

Pteranodonte::Pteranodonte(int x, int y)
	: Dinosaurio(x, y, 60, 60, 5 + rand() % 2, 256, 256, 4)   // velocidad 5..6 (> velocirraptor)
{
	dx = (rand() % 2 == 0) ? velocidad : -velocidad;
	dy = (rand() % 2 == 0) ? velocidad : -velocidad;
	actualizarFila();
}

void Pteranodonte::mover(Rectangle zona)
{
	x += dx;
	y += dy;
	if (x <= zona.X)                  { x = zona.X; dx = velocidad; }
	else if (x + ancho >= zona.Right) { x = zona.Right - ancho; dx = -velocidad; }
	if (y <= zona.Y)                  { y = zona.Y; dy = velocidad; }
	else if (y + alto >= zona.Bottom) { y = zona.Bottom - alto; dy = -velocidad; }
	actualizarFila();
	avanzarAnimacion();
}

int Pteranodonte::getTipo()
{
	return 1;
}

void Pteranodonte::actualizarFila()
{
	// Filas de _dinosaurio.png: 0 frente, 1 izquierda, 2 derecha, 3 espalda.
	// En diagonal se usa la componente horizontal para elegir la fila.
	fila = (dx < 0) ? 1 : 2;
}
