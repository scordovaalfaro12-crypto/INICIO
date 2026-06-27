#pragma once
//==============================================================================
//  Enemigo.h  -  Declaracion de la clase BASE de los villanos (hereda de Sprite)
//------------------------------------------------------------------------------
//  Los dos tipos comparten la hoja "Enemigos.png" (6 columnas x 4 filas = dos
//  personajes de 3 columnas) y una velocidad aleatoria. Enemigo1 usa las
//  columnas 0..2 y Enemigo2 las columnas 3..5.
//==============================================================================
#include "Sprite.h"

namespace LB2_Bathero {

	ref class Enemigo : public Sprite {
	protected:
		int areaW, areaH, mitad; // Dimensiones del area y mitad de la pantalla
		Random^ azar;            // Generador aleatorio COMPARTIDO

	public:
		Enemigo(int w, int h, int mitad, Random^ azar);
	};
}
