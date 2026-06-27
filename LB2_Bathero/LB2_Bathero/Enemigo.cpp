//==============================================================================
//  Enemigo.cpp  -  Implementacion de la clase base de los villanos
//==============================================================================
#include "Enemigo.h"
#include "Util.h"

namespace LB2_Bathero {

	Enemigo::Enemigo(int w, int h, int mitad, Random^ azar) {
		this->areaW = w;
		this->areaH = h;
		this->mitad = mitad;
		this->azar = azar;
		hoja = Util::CargarImagen("Enemigos.png");
		colsHoja = 6; filasHoja = 4; numFrames = 3;
		ancho = 50; alto = 56;
	}
}
