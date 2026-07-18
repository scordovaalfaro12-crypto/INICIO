#include "Enemigo.h"
// ============================================================
//  Enemigo.cpp
// ============================================================

namespace Project1 {

	Enemigo::Enemigo(Bitmap^ hoja, int columnas, int filas,
		float x, float y, int ancho, int alto,
		int vida, float velocidad)
		: Entidad(hoja, columnas, filas, x, y, ancho, alto, vida, velocidad)
	{
	}

	void Enemigo::Mover(int anchoVentana, int altoVentana)
	{
		if (!activo) return;

		x += velocidad;

		// cuando sale por completo de la pantalla deja de existir
		if (x + ancho < -50 || x > anchoVentana + 50)
		{
			activo = false;
		}

		Animar();
	}
}
