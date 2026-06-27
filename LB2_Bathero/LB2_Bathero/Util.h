#pragma once
//==============================================================================
//  Util.h  -  Declaracion de la clase utilitaria (carga de imagenes/recursos)
//==============================================================================
using namespace System;
using namespace System::Drawing;

namespace LB2_Bathero {

	// Clase estatica de apoyo. Carga una imagen buscandola en varias rutas
	// posibles para que funcione tanto si los recursos estan junto al .exe como
	// dentro de la carpeta del proyecto.
	ref class Util abstract sealed {
	public:
		static Image^ CargarImagen(String^ nombre);
	};
}
