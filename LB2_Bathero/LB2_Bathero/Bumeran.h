#pragma once
//==============================================================================
//  Bumeran.h  -  Declaracion del arma arrojadiza de Bathero (hereda de Sprite)
//------------------------------------------------------------------------------
//  Estados:  0 = Quieto (en la mano de Bathero, no se ve)
//            1 = Lanzado (vuela hacia la derecha girando)
//            2 = Regresando (vuelve a su posicion de lanzamiento)
//  Si atrapa un enemigo o llega al limite, regresa a su posicion inicial.
//==============================================================================
#include "Sprite.h"

namespace LB2_Bathero {

	ref class Bumeran : public Sprite {
	private:
		int estado;        // 0 quieto, 1 lanzado, 2 regresando
		float ox, oy;      // Origen al que debe regresar
		float vel;         // Rapidez de vuelo
		float alcanceMax;  // X maxima que alcanza antes de regresar
		float angulo;      // Angulo de giro (efecto visual)

	public:
		Bumeran();

		// Esta activo (visible y colisionable) solo cuando esta en vuelo.
		property bool Activo { bool get() { return estado != 0; } }

		void Lanzar(float bx, float by, int areaAncho); // lanzar desde Bathero
		void Regresar();                                // ordenar el regreso
		void SeguirA(float bx, float by);               // acompaniar a Bathero

		virtual void Actualizar() override;
		virtual void Dibujar(Graphics^ g) override;
	};
}
