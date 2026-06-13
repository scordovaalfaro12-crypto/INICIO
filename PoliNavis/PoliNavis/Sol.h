#pragma once
#include "Entidad.h"

namespace PoliNavis {

	// El Sol: centro del sistema solar y de todas las orbitas del Nivel 1.
	// Dibujado con circulos y lineas (rayos giratorios). No es obstaculo.
	public ref class Sol : public Entidad {
	public:
		Sol(float px, float py, float pradio);

		virtual void mover() override;
		virtual void dibujar(Graphics^ g) override;

		float getRadio();

	private:
		float radio;
		float giro;
	};
}
