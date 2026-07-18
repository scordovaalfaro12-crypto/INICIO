#pragma once
using namespace System;
using namespace System::Drawing;

// Caja de suministros: cuadrado de color rojo creado al iniciar la animacion,
// distribuido de forma aleatoria en la zona de juego (entre N y 2N cajas,
// donde N es el total de dinosaurios generados). Al colisionar el
// guardabosques con una caja, esta desaparece y el inventario suma 1 dardo.
class Caja
{
private:
	int x, y;
	int tam;

public:
	Caja(int x, int y);
	~Caja();

	void dibujar(Graphics^ g);
	Rectangle getRectangulo();
};
