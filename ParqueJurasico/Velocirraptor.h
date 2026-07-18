#pragma once
#include "Dinosaurio.h"

// Velocirraptor: aparece al iniciar la partida (entre 6 y 9).
// Algoritmo propio: se desplaza en linea recta HORIZONTAL o VERTICAL
// (elegida al azar al nacer) y cambia de direccion al chocar con los bordes.
// Sprite: _dinosaurio2.png (hoja 4x4, frames de 100x100).
class Velocirraptor : public Dinosaurio
{
public:
	Velocirraptor(int x, int y);
	void mover(Rectangle zona) override;
	int getTipo() override;

private:
	void actualizarFila();
};
