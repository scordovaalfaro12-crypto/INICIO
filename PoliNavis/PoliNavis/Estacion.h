#pragma once
#include "Entidad.h"

namespace PoliNavis {

	// Meta del nivel. tipo 0 = estacion espacial (Nivel 1, Punto B);
	// tipo 1 = portal de salida (Nivel 2, Lugar 2). Hecha con figuras.
	public ref class Estacion : public Entidad {
	public:
		Estacion(float px, float py, int ptipo);

		virtual void mover() override;
		virtual void dibujar(Graphics^ g) override;
		virtual RectangleF area() override;

	private:
		int tipo;
		float fase;
	};
}
