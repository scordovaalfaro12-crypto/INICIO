#pragma once
#include "Entidad.h"

// Proyectil del Poder de los Apus (Ray-Trace): un rayo que desfragmenta
// a los agentes de Oblivion Corp. Se obtiene al entregar la ofrenda.
class RayoTrace : public Entidad {
private:
	int vidaUtil; // ticks restantes antes de disiparse

public:
	RayoTrace(int px, int py, int dirX, int dirY);
	bool terminado();
	void mover(Graphics^ g);
	Rectangle hitbox();
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};
