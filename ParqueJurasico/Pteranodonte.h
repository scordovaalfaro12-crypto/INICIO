#pragma once
#include "Dinosaurio.h"

// Pteranodonte: aparece al presionar la tecla "P" (entre 3 y 5).
// Algoritmo propio: velocidad mayor a la de los velocirraptores y
// trayectoria DIAGONAL que rebota al chocar con los bordes de la zona.
// Sprite: _dinosaurio.png (hoja 4x4, frames de 256x256).
class Pteranodonte : public Dinosaurio
{
public:
	Pteranodonte(int x, int y);
	void mover(Rectangle zona) override;
	int getTipo() override;

private:
	void actualizarFila();
};
