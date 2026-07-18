#pragma once
// ============================================================
//  Tesoro.h
//  Clase HIJA de Entidad. Cofre del tesoro fijo en el fondo.
//  Hay 4 tesoros repartidos al azar; al tocarlos el submarino
//  se muestra "Tesoro rescatado" y con los 4 "Mision completa".
// ============================================================
#include "Entidad.h"

namespace Project1 {

	public ref class Tesoro : public Entidad
	{
	private:
		bool rescatado;

	public:
		Tesoro(Bitmap^ hoja, float x, float y);

		// el tesoro no se desplaza, solo se anima
		virtual void Mover(int anchoVentana, int altoVentana) override;

		void Rescatar();

		property bool Rescatado { bool get(); }
	};
}
