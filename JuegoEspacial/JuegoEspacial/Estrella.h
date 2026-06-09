#pragma once
#include "Item.h"

namespace JuegoEspacial {

	// Estrella dorada que cae girando: da puntos al recogerla
	public ref class Estrella : public Item {
	public:
		Estrella(float px, float py, float pvy, float ptam, int pvalor);

		virtual void mover() override;
		virtual void aplicarEfecto(Astronauta^ astronauta) override;
		virtual void dibujar(Graphics^ g) override;
		virtual RectangleF zona() override;

		// dibuja una estrella de 5 puntas en cualquier lugar (tambien la usan los menus)
		static void dibujarForma(Graphics^ g, float cx, float cy, float radio, float anguloGrados, Color color);

	private:
		float vy;
		float tam;
		float angulo;
	};
}
