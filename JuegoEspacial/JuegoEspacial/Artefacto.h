#pragma once
#include "Item.h"

namespace JuegoEspacial {

	// Reliquia alienigena que flota: vale muchos puntos
	public ref class Artefacto : public Item {
	public:
		Artefacto(float px, float py, int pvalor);

		virtual void mover() override;       // flota suavemente
		virtual void aplicarEfecto(Astronauta^ astronauta) override;
		virtual void dibujar(Graphics^ g) override;
		virtual RectangleF zona() override;

	private:
		float fase;
		float desplaceY();
	};
}
