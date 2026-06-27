//==============================================================================
//  Controlador.cpp  -  Implementacion del objeto controlador
//==============================================================================
#include "Controlador.h"
#include "Util.h"

namespace LB2_Bathero {

	Controlador::Controlador(int ancho, int alto) {
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

	//------------------------- USO DE LAS TECLAS --------------------------
	void Controlador::TeclaAbajo(Keys k) {
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

	void Controlador::TeclaArriba(Keys k) {
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
	void Controlador::GenerarEnemigos1() {        // tecla E: aparecen 3..5
		int n = azar->Next(3, 6);
		for (int i = 0; i < n && enemigos->Count < maxEnemigos; i++)
			enemigos->Add(gcnew Enemigo1(ancho, alto, mitad, azar));
	}

	void Controlador::GenerarEnemigos2() {        // tecla F: aparecen 2..4
		int n = azar->Next(2, 5);
		for (int i = 0; i < n && enemigos->Count < maxEnemigos; i++)
			enemigos->Add(gcnew Enemigo2(ancho, alto, mitad, azar));
	}

	void Controlador::LanzarBumeran() {           // barra espaciadora
		bumeran->Lanzar(bathero->X + bathero->Ancho - 6,
			bathero->Y + bathero->Alto / 3.0f, ancho);
	}

	//---------------------------- ANIMACION -------------------------------
	void Controlador::Actualizar() {
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
	void Controlador::Dibujar() {
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

	void Controlador::DibujarHUD() {
		g->FillRectangle(brochaHUD, 0, 0, ancho, 30);
		String^ info = String::Format(
			"Atrapados: {0}  (Enemigo1: {1} / Enemigo2: {2})    |    "
			"FLECHAS: mover    E: Enemigo1    F: Enemigo2    ESPACIO: bumeran    X: salir",
			Atrapados, atrapados1, atrapados2);
		g->DrawString(info, fuenteHUD, Brushes::White, 8, 7);
	}

	//---------------------------- REPORTE FINAL ---------------------------
	String^ Controlador::Reporte() {
		return String::Format(
			"===== FIN DE LA SIMULACION =====\n\n"
			"Total de enemigos atrapados: {0}\n\n"
			"   - Enemigo1 (horizontales): {1}\n"
			"   - Enemigo2 (verticales):   {2}\n\n"
			"Enemigos que quedaron en pantalla: {3}",
			Atrapados, atrapados1, atrapados2, enemigos->Count);
	}

	// Libera explicitamente los recursos administrados.
	void Controlador::Liberar() {
		if (fuenteHUD != nullptr) { delete fuenteHUD; fuenteHUD = nullptr; }
		if (brochaHUD != nullptr) { delete brochaHUD; brochaHUD = nullptr; }
		if (lapizDivisor != nullptr) { delete lapizDivisor; lapizDivisor = nullptr; }
		if (g != nullptr) { delete g; g = nullptr; }
		if (buffer != nullptr) { delete buffer; buffer = nullptr; }
		if (enemigos != nullptr) enemigos->Clear();
	}
}
