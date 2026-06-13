#include "Recursos.h"

using namespace System::IO;

namespace PoliNavis {

	// Busca la imagen en varias rutas posibles: junto al .exe, en la carpeta
	// del proyecto o subiendo desde x64/Debug. Devuelve nullptr si no existe.
	Image^ Recursos::intentarCargar(String^ nombre) {
		array<String^>^ rutas = gcnew array<String^> {
			"img/" + nombre,
			"../../img/" + nombre,
			"../../PoliNavis/img/" + nombre,
			nombre
		};
		for each (String^ ruta in rutas) {
			try {
				if (File::Exists(ruta)) return Image::FromFile(ruta);
			}
			catch (Exception^) {
				// ruta invalida: probar la siguiente
			}
		}
		return nullptr;
	}

	void Recursos::cargar() {
		if (cargado) return;
		cargado = true;
		fondoMenu = intentarCargar("menu.png");
		fondoEspacio = intentarCargar("espacio.png");
		fondoNebulosa = intentarCargar("nebulosa.png");
	}
}
