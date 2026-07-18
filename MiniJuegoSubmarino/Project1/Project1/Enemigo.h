#pragma once
// ============================================================
//  Enemigo.h
//  Clase HIJA de Entidad y PADRE de Pez y Tiburon.
//  Define el movimiento automatico horizontal de los enemigos.
// ============================================================
#include "Entidad.h"

namespace Project1 {

	public ref class Enemigo : public Entidad
	{
	public:
		Enemigo(Bitmap^ hoja, int columnas, int filas,
			float x, float y, int ancho, int alto,
			int vida, float velocidad);

		// Movimiento automatico: avanza segun el signo de la velocidad
		// (negativa = derecha a izquierda, positiva = izquierda a derecha)
		// y se desactiva al salir de la pantalla.
		virtual void Mover(int anchoVentana, int altoVentana) override;
	};
}
