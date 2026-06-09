#pragma once
#include "Enemigo.h"

namespace JuegoEspacial {

	// Chatarra que flota en orbita: paneles solares rotos, satelites y tuercas gigantes
	public ref class BasuraEspacial : public Enemigo {
	public:
		BasuraEspacial(float px, float py, int ptipoPieza);

		virtual void mover() override;       // gira y se balancea en su sitio
		virtual void atacar() override;
		virtual void dibujar(Graphics^ g) override;
		virtual RectangleF zona() override;

	private:
		float angulo;
		float velocidadGiro;
		float fase;          // balanceo vertical
		int tipoPieza;       // 0 = panel solar, 1 = satelite, 2 = tuerca
		float desplaceY();   // desplazamiento del balanceo
	};
}
