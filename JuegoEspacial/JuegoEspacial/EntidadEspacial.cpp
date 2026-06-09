#include "EntidadEspacial.h"

namespace JuegoEspacial {

	EntidadEspacial::EntidadEspacial(float px, float py) {
		x = px;
		y = py;
	}

	float EntidadEspacial::getX() {
		return x;
	}

	float EntidadEspacial::getY() {
		return y;
	}

	void EntidadEspacial::setPosicion(float px, float py) {
		x = px;
		y = py;
	}
}
