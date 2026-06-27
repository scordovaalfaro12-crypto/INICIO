#pragma once
//==============================================================================
//  Controlador.h  -  Objeto CONTROLADOR de la animacion
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
		Controlador(int ancho, int alto) {
			this->ancho = ancho;
			this->alto = alto;
			this->mitad = ancho / 2;

			// --- Creacion del buffer (doble buffer manual) ---
			buffer = gcnew Bitmap(ancho, alto);
			g = Graphics::FromImage(buffer);
			g->InterpolationMode =
				System::Drawing::Drawing2D::InterpolationMode::NearestNeighbor;

			fondo = Util::CargarImagen("fondo.png");
			azar = gcnew Random();
			bathero = gcnew Bathero(alto);
			enemigos = gcnew List<Enemigo^>();
			bumeran = gcnew Bumeran();

			tArriba = tAbajo = tIzq = tDer = false;
			eDown = fDown = spDown = false;
			atrapados1 = atrapados2 = 0;
			terminado = false;
			maxEnemigos = 40;

			fuenteHUD = gcnew Font("Consolas", 10, FontStyle::Bold);
			brochaHUD = gcnew SolidBrush(Color::FromArgb(150, 0, 0, 0));
			lapizDivisor = gcnew Pen(Color::FromArgb(120, 90, 110, 170), 2);
		}

		// El formulario toma este buffer ya dibujado y lo vuelca a pantalla.
		property Image^ Buffer { Image^ get() { return buffer; } }
		property bool Terminado { bool get() { return terminado; } }
		property int Atrapados { int get() { return atrapados1 + atrapados2; } }

		//------------------------- USO DE LAS TECLAS --------------------------
		void TeclaAbajo(Keys k) {
			if (terminado) return;
			switch (k) {
			case Keys::Left:  tIzq = true; break;
			case Keys::Right: tDer = true; break;
			case Keys::Up:    tArriba = true; break;
			case Keys::Down:  tAbajo = true; break;
			case Keys::E: if (!eDown) { eDown = true; GenerarEnemigos1(); } break;
			case Keys::F: if (!fDown) { fDown = true; GenerarEnemigos2(); } break;
			case Keys::Space: if (!spDown) { spDown = true; LanzarBumeran(); } break;
			case Keys::X: terminado = true; break;
			}
		}

		void TeclaArriba(Keys k) {
			switch (k) {
			case Keys::Left:  tIzq = false; break;
			case Keys::Right: tDer = false; break;
			case Keys::Up:    tArriba = false; break;
			case Keys::Down:  tAbajo = false; break;
			case Keys::E: eDown = false; break;
			case Keys::F: fDown = false; break;
			case Keys::Space: spDown = false; break;
			}
		}

		//----------------------- GENERACION DE VILLANOS -----------------------
		void GenerarEnemigos1() {        // tecla E: aparecen 3..5
			int n = azar->Next(3, 6);
			for (int i = 0; i < n && enemigos->Count < maxEnemigos; i++)
				enemigos->Add(gcnew Enemigo1(ancho, alto, mitad, azar));
		}

		void GenerarEnemigos2() {        // tecla F: aparecen 2..4
			int n = azar->Next(2, 5);
			for (int i = 0; i < n && enemigos->Count < maxEnemigos; i++)
				enemigos->Add(gcnew Enemigo2(ancho, alto, mitad, azar));
		}

		void LanzarBumeran() {           // barra espaciadora
			bumeran->Lanzar(bathero->X + bathero->Ancho - 6,
				bathero->Y + bathero->Alto / 3.0f, ancho);
		}

		//---------------------------- ANIMACION -------------------------------
		void Actualizar() {
			if (terminado) return;

			// Mover a Bathero (confinado a la mitad izquierda).
			int limX = mitad - bathero->Ancho - 8;
			int limY = alto - bathero->Alto;
			bathero->Mover(tArriba, tAbajo, tIzq, tDer, limX, limY);

			// El bumeran acompania a Bathero mientras no esta en vuelo.
			bumeran->SeguirA(bathero->X + bathero->Ancho - 6,
				bathero->Y + bathero->Alto / 3.0f);

			// Mover enemigos y bumeran.
			for each (Enemigo ^ en in enemigos) en->Actualizar();
			bumeran->Actualizar();

			// Colisiones bumeran <-> enemigos (se recorre al reves para borrar).
			if (bumeran->Activo) {
				for (int i = enemigos->Count - 1; i >= 0; i--) {
					if (bumeran->Colisiona(enemigos[i])) {
						if (dynamic_cast<Enemigo1^>(enemigos[i]) != nullptr) atrapados1++;
						else atrapados2++;
						enemigos->RemoveAt(i); // el enemigo desaparece
						bumeran->Regresar();   // y el bumeran vuelve
					}
				}
			}
		}

		//----------------------- DIBUJO SOBRE EL BUFFER -----------------------
		void Dibujar() {
			// Fondo
			if (fondo != nullptr) g->DrawImage(fondo, 0, 0, ancho, alto);
			else g->Clear(Color::FromArgb(10, 12, 28));

			// Linea divisoria de las dos zonas
			g->DrawLine(lapizDivisor, mitad, 0, mitad, alto);

			// Sprites
			bathero->Dibujar(g);
			for each (Enemigo ^ en in enemigos) en->Dibujar(g);
			bumeran->Dibujar(g);

			// Panel de informacion (HUD)
			DibujarHUD();
		}

		void DibujarHUD() {
			g->FillRectangle(brochaHUD, 0, 0, ancho, 30);
			String^ info = String::Format(
				"Atrapados: {0}  (Enemigo1: {1} / Enemigo2: {2})    |    "
				"FLECHAS: mover    E: Enemigo1    F: Enemigo2    ESPACIO: bumeran    X: salir",
				Atrapados, atrapados1, atrapados2);
			g->DrawString(info, fuenteHUD, Brushes::White, 8, 7);
		}

		//---------------------------- REPORTE FINAL ---------------------------
		String^ Reporte() {
			return String::Format(
				"===== FIN DE LA SIMULACION =====\n\n"
				"Total de enemigos atrapados: {0}\n\n"
				"   - Enemigo1 (horizontales): {1}\n"
				"   - Enemigo2 (verticales):   {2}\n\n"
				"Enemigos que quedaron en pantalla: {3}",
				Atrapados, atrapados1, atrapados2, enemigos->Count);
		}

		// Libera explicitamente los recursos administrados.
		void Liberar() {
			if (fuenteHUD != nullptr) { delete fuenteHUD; fuenteHUD = nullptr; }
			if (brochaHUD != nullptr) { delete brochaHUD; brochaHUD = nullptr; }
			if (lapizDivisor != nullptr) { delete lapizDivisor; lapizDivisor = nullptr; }
			if (g != nullptr) { delete g; g = nullptr; }
			if (buffer != nullptr) { delete buffer; buffer = nullptr; }
			if (enemigos != nullptr) enemigos->Clear();
		}
	};
}
