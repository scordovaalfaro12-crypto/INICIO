#include "Tesoro.h"
// ============================================================
//  Tesoro.cpp
// ============================================================

namespace Project1 {

	Tesoro::Tesoro(Bitmap^ hoja, float x, float y)
		: Entidad(hoja, 4, 1, x, y,
			76, 88,     /* tamanio del cofre */
			1,
			0.0f)       /* velocidad 0: el tesoro no se mueve */
	{
		rescatado = false;
		retardoAnim = 8;   // animacion mas lenta para el cofre
	}

	void Tesoro::Mover(int anchoVentana, int altoVentana)
	{
		if (!activo) return;
		Animar();
	}

	void Tesoro::Rescatar()
	{
		rescatado = true;
		activo = false;   // desaparece del juego al ser rescatado
	}

	bool Tesoro::Rescatado::get()
	{
		return rescatado;
	}
}
