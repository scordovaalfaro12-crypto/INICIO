#include "Obstaculo.h"
#include "Config.h"
#include "Azar.h"

namespace PoliNavis {

	Obstaculo::Obstaculo(float px, float py, float pdx, float pdy, float pradio)
		: Entidad(px, py) {
		dx = pdx; dy = pdy;
		radio = pradio;
		giro = Azar::entreF(0.0f, 360.0f);
		velGiro = Azar::entreF(-4.0f, 4.0f);
	}

	void Obstaculo::mover() {
		x += dx;
		y += dy;
		giro += velGiro;
	}

	bool Obstaculo::fueraDePantalla() {
		float m = radio + 40.0f;
		return x < -m || x > Config::ANCHO + m || y < -m || y > Config::ALTO + m;
	}

	RectangleF Obstaculo::area() {
		return RectangleF(x - radio * 0.8f, y - radio * 0.8f, radio * 1.6f, radio * 1.6f);
	}

	float Obstaculo::getRadio() { return radio; }
}
