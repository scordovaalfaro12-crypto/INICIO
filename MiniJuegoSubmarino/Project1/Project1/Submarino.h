#pragma once
// ============================================================
//  Submarino.h
//  Clase HIJA de Entidad. Es el jugador.
//  Hereda tamanio, vida y movimiento de la clase padre.
//  Se desplaza con las flechas o con W, A, S, D.
// ============================================================
#include "Entidad.h"

namespace Project1 {

	public ref class Submarino : public Entidad
	{
	private:
		Bitmap^ hojaHorizontal;   // sprites mirando izquierda / derecha
		Bitmap^ hojaVertical;     // sprites mirando arriba / abajo
		Direccion direccion;      // hacia donde apunta el submarino

		// banderas de teclas presionadas (permite movimiento continuo)
		bool moverIzquierda;
		bool moverDerecha;
		bool moverArriba;
		bool moverAbajo;

		void ActualizarSprite();  // cambia hoja/fila segun la direccion

	public:
		Submarino(Bitmap^ hojaHorizontal, Bitmap^ hojaVertical,
			float x, float y);

		virtual void Mover(int anchoVentana, int altoVentana) override;

		void EmpezarMovimiento(Direccion d);
		void PararMovimiento(Direccion d);

		property Direccion Sentido { Direccion get(); }
	};
}
