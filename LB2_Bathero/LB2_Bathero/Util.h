#pragma once
//==============================================================================
//  Util.h  -  Utilidades generales (carga de imagenes / recursos)
//==============================================================================
using namespace System;
using namespace System::IO;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace LB2_Bathero {

	// Clase estatica de apoyo. Carga una imagen buscandola en varias rutas
	// posibles para que el programa funcione tanto si los recursos estan junto
	// al .exe como dentro de la carpeta del proyecto.  Se carga a traves de un
	// MemoryStream para NO dejar el archivo bloqueado en disco.
	ref class Util abstract sealed {
	public:
		static Image^ CargarImagen(String^ nombre) {
			array<String^>^ rutas = gcnew array<String^>{
				Path::Combine(Application::StartupPath, "Recursos\\" + nombre),
				Path::Combine(Application::StartupPath, nombre),
				Path::Combine(Application::StartupPath, "..\\..\\Recursos\\" + nombre),
				Path::Combine(Application::StartupPath, "..\\..\\..\\Recursos\\" + nombre),
				Path::Combine(Application::StartupPath, "..\\..\\..\\LB2_Bathero\\Recursos\\" + nombre),
				Path::Combine("Recursos", nombre),
				nombre
			};

			for each (String ^ ruta in rutas) {
				try {
					if (File::Exists(ruta)) {
						array<Byte>^ datos = File::ReadAllBytes(ruta);
						MemoryStream^ ms = gcnew MemoryStream(datos);
						Image^ original = Image::FromStream(ms);
						// Copiamos a un Bitmap para soltar el stream y el archivo.
						Bitmap^ copia = gcnew Bitmap(original);
						delete original;
						return copia;
					}
				}
				catch (Exception^) {
					// Si una ruta falla, se intenta con la siguiente.
				}
			}
			return nullptr; // No encontrada: las clases dibujan un marcador.
		}
	};
}
