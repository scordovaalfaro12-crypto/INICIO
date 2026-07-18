#pragma once
// ============================================================
//  Explosion.h
//  Clase HIJA de Entidad. Efecto visual que se reproduce una
//  sola vez cuando un torpedo elimina a un enemigo.
// ============================================================
#include "Entidad.h"

namespace Project1 {

	public ref class Explosion : public Entidad
	{
	public:
		// centroX / centroY: punto donde ocurre la explosion
		Explosion(Bitmap^ hoja, float centroX, float centroY);

		// la explosion no se desplaza, solo reproduce su animacion
		virtual void Mover(int anchoVentana, int altoVentana) override;

		// al terminar los fotogramas la explosion desaparece
		virtual void Animar() override;
	};
}
