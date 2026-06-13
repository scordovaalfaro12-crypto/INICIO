#pragma once
using namespace System;
using namespace System::Drawing;

namespace PoliNavis {

	// Carga las imagenes de fondo DECORATIVAS (lo unico permitido por la
	// rubrica como imagen). Si no encuentra el archivo, devuelve nullptr
	// y el juego dibuja un fondo de estrellas con figuras como respaldo,
	// asi el programa nunca se cae aunque falte la carpeta img.
	public ref class Recursos abstract sealed {
	public:
		static Image^ fondoMenu;       // menu.png
		static Image^ fondoEspacio;    // espacio.png (Nivel 1)
		static Image^ fondoNebulosa;   // nebulosa.png (Nivel 2)
		static bool cargado = false;

		static void cargar();

	private:
		static Image^ intentarCargar(String^ nombre);
	};
}
