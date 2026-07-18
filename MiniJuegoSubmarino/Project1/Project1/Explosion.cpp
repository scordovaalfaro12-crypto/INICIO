#include "Explosion.h"
// ============================================================
//  Explosion.cpp
// ============================================================

namespace Project1 {

	Explosion::Explosion(Bitmap^ hoja, float centroX, float centroY)
		: Entidad(hoja, 6, 1,
			0.0f, 0.0f,
			84, 92,     /* tamanio del efecto */
			1, 0.0f)
	{
		x = centroX - ancho / 2.0f;
		y = centroY - alto / 2.0f;
		retardoAnim = 3;
	}

	void Explosion::Mover(int anchoVentana, int altoVentana)
	{
		if (!activo) return;
		Animar();
	}

	void Explosion::Animar()
	{
		contadorAnim++;
		if (contadorAnim >= retardoAnim)
		{
			contadorAnim = 0;
			frameActual++;
			if (frameActual >= columnas)
			{
				activo = false;   // ya se reprodujo completa
			}
		}
	}
}
