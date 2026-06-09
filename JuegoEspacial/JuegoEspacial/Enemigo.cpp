#include "Enemigo.h"

namespace JuegoEspacial {

	Enemigo::Enemigo(float px, float py, int pvida) : EntidadEspacial(px, py) {
		vida = pvida;
		destello = 0;
	}

	void Enemigo::recibirGolpe(int danio) {
		vida -= danio;
		destello = 12;
	}

	int Enemigo::getVida() {
		return vida;
	}

	bool Enemigo::destruido() {
		return vida <= 0;
	}
}
