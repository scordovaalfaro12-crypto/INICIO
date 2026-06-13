#include "Entidad.h"

namespace PoliNavis {

	Entidad::Entidad(float px, float py) {
		x = px; y = py;
		dx = 0; dy = 0;
	}

	RectangleF Entidad::area() {
		return RectangleF(x - 10.0f, y - 10.0f, 20.0f, 20.0f);
	}

	float Entidad::getX() { return x; }
	float Entidad::getY() { return y; }

	void Entidad::setPos(float px, float py) { x = px; y = py; }
	void Entidad::setVel(float pdx, float pdy) { dx = pdx; dy = pdy; }
}
