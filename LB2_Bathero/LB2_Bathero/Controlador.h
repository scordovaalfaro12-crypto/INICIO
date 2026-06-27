#pragma once
//==============================================================================
//  Controlador.h  -  Declaracion del objeto CONTROLADOR de la animacion
//------------------------------------------------------------------------------
//  Responsabilidades (segun la rubrica):
//    * Administra el BUFFER (Bitmap fuera de pantalla) para evitar el parpadeo.
//    * Controla la ANIMACION: actualiza y dibuja a todos los objetos.
//    * Controla el USO DE LAS TECLAS (mover, generar enemigos, lanzar, salir).
//    * Detecta colisiones y lleva el conteo para el REPORTE final.
//==============================================================================
#include "Bathero.h"
#include "Enemigo1.h"
#include "Enemigo2.h"
#include "Bumeran.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Collections::Generic;
using namespace System::Windows::Forms;

namespace LB2_Bathero {

	ref class Controlador {
	private:
		int ancho, alto, mitad;
		Bitmap^ buffer;           // <-- BUFFER para evitar el parpadeo
		Graphics^ g;              // Lienzo de dibujo sobre el buffer
		Image^ fondo;             // Imagen de fondo
		Bathero^ bathero;
		List<Enemigo^>^ enemigos; // Lista polimorfica de villanos
		Bumeran^ bumeran;
		Random^ azar;

		// Estado de las teclas de movimiento (se mantienen presionadas).
		bool tArriba, tAbajo, tIzq, tDer;
		// Control de pulsacion unica (evita repeticion al mantener la tecla).
		bool eDown, fDown, spDown;

		int atrapados1, atrapados2; // Conteo por tipo para el reporte
		bool terminado;
		int maxEnemigos;

		// Recursos de dibujo creados una sola vez (eficiencia).
		Font^ fuenteHUD;
		SolidBrush^ brochaHUD;
		Pen^ lapizDivisor;

	public:
		Controlador(int ancho, int alto);

		property Image^ Buffer { Image^ get() { return buffer; } }
		property bool Terminado { bool get() { return terminado; } }
		property int Atrapados { int get() { return atrapados1 + atrapados2; } }

		// Uso de las teclas
		void TeclaAbajo(Keys k);
		void TeclaArriba(Keys k);

		// Generacion de villanos / lanzamiento
		void GenerarEnemigos1();
		void GenerarEnemigos2();
		void LanzarBumeran();

		// Animacion
		void Actualizar();
		void Dibujar();
		void DibujarHUD();

		// Reporte y liberacion
		String^ Reporte();
		void Liberar();
	};
}
