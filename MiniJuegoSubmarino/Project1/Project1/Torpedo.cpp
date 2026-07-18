#include "Torpedo.h"
// ============================================================
//  Torpedo.cpp
//  El torpedo es el proyectil mas rapido del juego.
// ============================================================

namespace Project1 {

	static const int TORP_ANCHO_H = 64;
	static const int TORP_ALTO_H = 28;
	static const int TORP_ANCHO_V = 28;
	static const int TORP_ALTO_V = 64;
	static const float TORP_VELOCIDAD = 16.0f;

	Torpedo::Torpedo(Bitmap^ hojaHorizontal, Bitmap^ hojaVertical,
		float centroX, float centroY, Direccion direccion)
		: Entidad(
			(direccion == Direccion::Arriba || direccion == Direccion::Abajo)
				? hojaVertical : hojaHorizontal,
			3, 2, 0.0f, 0.0f,
			(direccion == Direccion::Arriba || direccion == Direccion::Abajo)
				? TORP_ANCHO_V : TORP_ANCHO_H,
			(direccion == Direccion::Arriba || direccion == Direccion::Abajo)
				? TORP_ALTO_V : TORP_ALTO_H,
			1, TORP_VELOCIDAD)
	{
		this->direccion = direccion;

		// fila 0 horizontal: apunta a la izquierda / fila 1: a la derecha
		// fila 0 vertical:   apunta hacia arriba  / fila 1: hacia abajo
		switch (direccion)
		{
		case Direccion::Izquierda: fila = 0; break;
		case Direccion::Derecha:   fila = 1; break;
		case Direccion::Arriba:    fila = 0; break;
		case Direccion::Abajo:     fila = 1; break;
		}

		// sale centrado en el punto indicado
		x = centroX - ancho / 2.0f;
		y = centroY - alto / 2.0f;
	}

	void Torpedo::Mover(int anchoVentana, int altoVentana)
	{
		if (!activo) return;

		switch (direccion)
		{
		case Direccion::Izquierda: x -= velocidad; break;
		case Direccion::Derecha:   x += velocidad; break;
		case Direccion::Arriba:    y -= velocidad; break;
		case Direccion::Abajo:     y += velocidad; break;
		}

		// se desactiva al salir de la pantalla
		if (x + ancho < 0 || x > anchoVentana ||
			y + alto < 0 || y > altoVentana)
		{
			activo = false;
		}

		Animar();
	}
}
