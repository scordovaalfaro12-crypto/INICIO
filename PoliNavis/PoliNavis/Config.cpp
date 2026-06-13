#include "Config.h"

namespace PoliNavis {

	int Config::centroX() {
		return ANCHO / 2;
	}

	int Config::centroY() {
		return ALTO / 2;
	}

	Point Config::puntoA() {
		return Point(AX, AY);
	}

	Point Config::puntoB() {
		return Point(BX, BY);
	}
}
