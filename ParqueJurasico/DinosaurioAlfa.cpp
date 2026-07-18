#include "DinosaurioAlfa.h"

DinosaurioAlfa::DinosaurioAlfa(int x, int y)
	: Dinosaurio(x, y, 72, 72, 8, 64, 64, 3)   // velocidad 8: mayor a cualquier otra especie
{
	impactos = 0;
	dx = (rand() % 2 == 0) ? velocidad : -velocidad;  // sentido horizontal aleatorio
	dy = 0;
	fila = (dx < 0) ? 1 : 2;
}

void DinosaurioAlfa::mover(Rectangle zona)
{
	x += dx;
	if (x <= zona.X)                  { x = zona.X; dx = velocidad; }
	else if (x + ancho >= zona.Right) { x = zona.Right - ancho; dx = -velocidad; }
	// Filas de DINOALFA.png: 0 frente, 1 izquierda, 2 derecha, 3 espalda
	fila = (dx < 0) ? 1 : 2;
	avanzarAnimacion();
}

int DinosaurioAlfa::getTipo()
{
	return 2;
}

bool DinosaurioAlfa::recibirImpacto()
{
	impactos++;
	return impactos >= 2;   // requiere 2 impactos de dardo
}
