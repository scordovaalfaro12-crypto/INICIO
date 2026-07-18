#pragma once
// ============================================================
//  Torpedo.h
//  Clase HIJA de Entidad. Proyectil que dispara el submarino
//  con la tecla ESPACIO, en el sentido en el que este mirando.
// ============================================================
#include "Entidad.h"

namespace Project1 {

	public ref class Torpedo : public Entidad
	{
	private:
		Direccion direccion;

	public:
		// centroX / centroY: punto desde donde sale el torpedo
		Torpedo(Bitmap^ hojaHorizontal, Bitmap^ hojaVertical,
			float centroX, float centroY, Direccion direccion);

		virtual void Mover(int anchoVentana, int altoVentana) override;
	};
}
