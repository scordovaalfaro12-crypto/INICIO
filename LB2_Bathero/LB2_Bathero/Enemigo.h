#pragma once
//==============================================================================
//  Enemigo.h  -  Clase BASE de los villanos (hereda de Sprite)
//------------------------------------------------------------------------------
//  Los dos tipos de enemigos comparten la hoja "Enemigos.png" (6 columnas x
//  4 filas = dos personajes de 3 columnas cada uno) y una velocidad aleatoria.
//  Enemigo1 usa las columnas 0..2 y Enemigo2 las columnas 3..5.
//==============================================================================
#include "Sprite.h"

namespace LB2_Bathero {

	ref class Enemigo : public Sprite {
	protected:
		int areaW, areaH, mitad; // Dimensiones del area y mitad de la pantalla
		Random^ azar;            // Generador aleatorio COMPARTIDO (evita repetir)

	public:
		Enemigo(int w, int h, int mitad, Random^ azar) {
			this->areaW = w;
			this->areaH = h;
			this->mitad = mitad;
			this->azar = azar;
			hoja = Util::CargarImagen("Enemigos.png");
			colsHoja = 6; filasHoja = 4; numFrames = 3;
			ancho = 50; alto = 56;
		}
	};
}
