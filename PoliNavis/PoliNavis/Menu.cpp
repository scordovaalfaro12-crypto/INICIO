#include "Menu.h"
#include "Estilos.h"
#include "Config.h"
#include "Fondo.h"
#include "Recursos.h"

namespace PoliNavis {

	// Las etiquetas de los botones se crean dentro de la funcion (en C++/CLI
	// no se permite un arreglo administrado como variable global).
	static array<String^>^ opciones() {
		return gcnew array<String^> {
			L"NIVEL 1 - Sistema Solar",
			L"NIVEL 2 - Galaxia del Khaos",
			L"INSTRUCCIONES",
			L"CREADORES",
			L"SALIR"
		};
	}

	Menu::Menu() { opcion = 0; }

	RectangleF Menu::rectBoton(int i) {
		// columna a la izquierda para no tapar el arte de menu.png
		return RectangleF(60.0f, 250.0f + i * 80.0f, 380.0f, 62.0f);
	}
	RectangleF Menu::rectVolver() {
		return RectangleF(36.0f, 626.0f, 200.0f, 46.0f);
	}

	void Menu::dibujarFondo(Graphics^ g, int tick, bool menu) {
		Image^ img = menu ? Recursos::fondoMenu : Recursos::fondoEspacio;
		if (img != nullptr) {
			g->DrawImage(img, 0, 0, Config::ANCHO, Config::ALTO);
			// en el menu el velo es tenue para lucir el arte; en las
			// sub-pantallas es mas oscuro para que el texto se lea bien.
			int a = menu ? 40 : 140;
			SolidBrush velo(Color::FromArgb(a, 4, 6, 16));
			g->FillRectangle(%velo, 0, 0, Config::ANCHO, Config::ALTO);
		}
		else {
			Fondo::dibujar(g, tick);   // respaldo: estrellas con figuras
		}
	}

	// ===================== PRINCIPAL =====================
	void Menu::dibujarPrincipal(Graphics^ g, int tick) {
		dibujarFondo(g, tick, true);

		// Si NO hay imagen de fondo, dibujamos un titulo propio (respaldo).
		// Si hay menu.png, su arte ya trae el titulo, asi que no lo tapamos.
		if (Recursos::fondoMenu == nullptr) {
			Estilos::textoSombra(g, L"PoliNavis", Estilos::titulo, Color::FromArgb(255, 215, 90), 500.0f, 110.0f);
			Estilos::textoCentrado(g, L"Viaje Interestelar", Estilos::subtitulo, Color::FromArgb(150, 210, 255), 500.0f, 165.0f);
		}

		// panel translucido detras de la columna de botones (legibilidad)
		Estilos::rellenarRedondeado(g, Color::FromArgb(120, 6, 10, 26), 48.0f, 236.0f, 404.0f, 408.0f, 16.0f);
		Estilos::bordeRedondeado(g, Color::FromArgb(90, 110, 180, 240), 1.5f, 48.0f, 236.0f, 404.0f, 408.0f, 16.0f);

		array<String^>^ op = opciones();
		for (int i = 0; i < 5; i++) boton(g, i, op[i], tick);

		Estilos::textoCentrado(g, L"Raton o flechas + ENTER", Estilos::pequena,
			Color::FromArgb(190, 210, 240), 250.0f, 664.0f);
	}

	void Menu::boton(Graphics^ g, int i, String^ texto, int tick) {
		RectangleF r = rectBoton(i);
		bool m = (opcion == i);
		float bx = r.X + (m ? 8.0f : 0.0f);
		if (m) {
			int a = 160 + (int)(60.0 * Math::Sin(tick * 0.12));
			Estilos::rellenarRedondeado(g, Color::FromArgb(230, 35, 80, 150), bx, r.Y, r.Width, r.Height, 12.0f);
			Estilos::bordeRedondeado(g, Color::FromArgb(a, 110, 230, 255), 2.5f, bx - 3.0f, r.Y - 3.0f, r.Width + 6.0f, r.Height + 6.0f, 14.0f);
		}
		else {
			Estilos::rellenarRedondeado(g, Color::FromArgb(200, 16, 26, 56), bx, r.Y, r.Width, r.Height, 12.0f);
			Estilos::bordeRedondeado(g, Color::FromArgb(150, 70, 95, 150), 1.5f, bx, r.Y, r.Width, r.Height, 12.0f);
		}
		Estilos::textoCentrado(g, texto, Estilos::boton, Color::White, bx + r.Width / 2.0f, r.Y + r.Height / 2.0f);
	}

	bool Menu::hoverPrincipal(int mx, int my) {
		for (int i = 0; i < 5; i++) {
			if (rectBoton(i).Contains((float)mx, (float)my)) {
				if (opcion != i) { opcion = i; return true; }
				return false;
			}
		}
		return false;
	}

	AccionMenu Menu::seleccionActual() {
		switch (opcion) {
		case 0: return AccionMenu::Nivel1;
		case 1: return AccionMenu::Nivel2;
		case 2: return AccionMenu::Instrucciones;
		case 3: return AccionMenu::Creadores;
		default: return AccionMenu::Salir;
		}
	}

	AccionMenu Menu::clicPrincipal(int mx, int my) {
		for (int i = 0; i < 5; i++) {
			if (rectBoton(i).Contains((float)mx, (float)my)) {
				opcion = i;
				return seleccionActual();
			}
		}
		return AccionMenu::Ninguna;
	}

	void Menu::moverSeleccion(int d) {
		opcion = (opcion + d + 5) % 5;
	}

	// ===================== INSTRUCCIONES =====================
	void Menu::dibujarInstrucciones(Graphics^ g, int tick) {
		dibujarFondo(g, tick, false);
		Estilos::textoSombra(g, L"INSTRUCCIONES", Estilos::enorme, Color::FromArgb(255, 215, 90), 500.0f, 70.0f);

		Estilos::rellenarRedondeado(g, Color::FromArgb(205, 12, 18, 40), 90.0f, 120.0f, 820.0f, 470.0f, 16.0f);
		Estilos::bordeRedondeado(g, Color::FromArgb(160, 90, 140, 220), 2.0f, 90.0f, 120.0f, 820.0f, 470.0f, 16.0f);

		Color cel = Color::FromArgb(140, 210, 255), bl = Color::FromArgb(220, 228, 245);
		float xi = 120.0f;
		Estilos::textoIzquierda(g, L"OBJETIVO EDUCATIVO", Estilos::normalNegrita, cel, xi, 150.0f);
		Estilos::textoIzquierda(g, L"Aprende los nombres de los planetas y sus movimientos de ROTACION (giro sobre su eje)", Estilos::normal, bl, xi, 178.0f);
		Estilos::textoIzquierda(g, L"y TRASLACION (orbita alrededor del Sol) mientras navegas por el espacio.", Estilos::normal, bl, xi, 200.0f);

		Estilos::textoIzquierda(g, L"NIVEL 1 - Sistema Solar", Estilos::normalNegrita, cel, xi, 240.0f);
		Estilos::textoIzquierda(g, L"Lleva la nave del Punto A (abajo-izquierda) a la Estacion Espacial (arriba-derecha).", Estilos::normal, bl, xi, 266.0f);
		Estilos::textoIzquierda(g, L"Esquiva cometas y asteroides: si chocas, vuelves al inicio y pierdes una vida.", Estilos::normal, bl, xi, 288.0f);
		Estilos::textoIzquierda(g, L"Hay 8 planetas con orbita y rotacion. Haz CLIC en un planeta para acelerarlo.", Estilos::normal, bl, xi, 310.0f);

		Estilos::textoIzquierda(g, L"NIVEL 2 - Galaxia Nebulosa del Khaos", Estilos::normalNegrita, cel, xi, 350.0f);
		Estilos::textoIzquierda(g, L"Un agujero negro te absorbe a una galaxia de figuras geometricas caoticas.", Estilos::normal, bl, xi, 376.0f);
		Estilos::textoIzquierda(g, L"Llega del Lugar 1 al portal de salida (Lugar 2) esquivando figuras y cometas neon mas veloces.", Estilos::normal, bl, xi, 398.0f);

		Estilos::textoIzquierda(g, L"CONTROLES", Estilos::normalNegrita, cel, xi, 438.0f);
		Estilos::textoIzquierda(g, L"Mover la nave:  Flechas  o  W A S D", Estilos::normal, bl, xi, 466.0f);
		Estilos::textoIzquierda(g, L"Acelerar planeta:  CLIC sobre el planeta (Nivel 1)", Estilos::normal, bl, xi, 488.0f);
		Estilos::textoIzquierda(g, L"Volver al menu / pausar:  ESC", Estilos::normal, bl, xi, 510.0f);

		Estilos::textoIzquierda(g, L"PANEL: arriba veras el tiempo, las colisiones, la energia y las vidas restantes.",
			Estilos::normal, Color::FromArgb(255, 220, 120), xi, 552.0f);

		botonVolver(g);
	}

	// ===================== CREADORES =====================
	void Menu::dibujarCreadores(Graphics^ g, int tick) {
		dibujarFondo(g, tick, false);
		Estilos::textoSombra(g, L"CREADORES", Estilos::enorme, Color::FromArgb(255, 215, 90), 500.0f, 70.0f);

		array<String^>^ nombres = gcnew array<String^> {
			L"Bryan Cordova", L"Lenin Palomino", L"Gustavo Arimana", L"Mustafa Altuntas", L"Biary Tineo"
		};
		array<String^>^ roles = gcnew array<String^> {
			L"Lider de proyecto y clase controladora (Juego)",
			L"Diseno de la nave, planetas y figuras (POO)",
			L"Niveles, colisiones y panel de control",
			L"Obstaculos, fisica del Nivel 2 y sonido",
			L"Menu, instrucciones, UML y documentacion"
		};
		array<Color>^ col = gcnew array<Color> {
			Color::FromArgb(70, 130, 220), Color::FromArgb(235, 140, 50),
			Color::FromArgb(90, 190, 110), Color::FromArgb(220, 80, 80),
			Color::FromArgb(180, 120, 240)
		};

		for (int i = 0; i < 5; i++) {
			float fy = 140.0f + i * 86.0f;
			Estilos::rellenarRedondeado(g, Color::FromArgb(200, 14, 22, 48), 220.0f, fy, 560.0f, 70.0f, 14.0f);
			Estilos::bordeRedondeado(g, Color::FromArgb(150, col[i]), 2.0f, 220.0f, fy, 560.0f, 70.0f, 14.0f);
			// casco mini del color del integrante
			SolidBrush casco(col[i]);
			g->FillEllipse(%casco, 250.0f, fy + 18.0f, 34.0f, 34.0f);
			SolidBrush visor(Color::FromArgb(30, 40, 75));
			g->FillEllipse(%visor, 259.0f, fy + 27.0f, 20.0f, 16.0f);
			Estilos::textoIzquierda(g, nombres[i], Estilos::subtitulo, Color::White, 310.0f, fy + 26.0f);
			Estilos::textoIzquierda(g, roles[i], Estilos::normal, Color::FromArgb(170, 200, 240), 310.0f, fy + 50.0f);
		}

		Estilos::textoCentrado(g, L"PoliNavis - Practica Calificada 1 - Algoritmos de Programacion",
			Estilos::pequena, Color::FromArgb(130, 150, 200), 500.0f, 600.0f);
		botonVolver(g);
	}

	void Menu::botonVolver(Graphics^ g) {
		RectangleF r = rectVolver();
		Estilos::rellenarRedondeado(g, Color::FromArgb(210, 16, 26, 56), r.X, r.Y, r.Width, r.Height, 12.0f);
		Estilos::bordeRedondeado(g, Color::FromArgb(170, 110, 200, 255), 2.0f, r.X, r.Y, r.Width, r.Height, 12.0f);
		Estilos::textoCentrado(g, L"< VOLVER  (ESC)", Estilos::normalNegrita, Color::White, r.X + r.Width / 2.0f, r.Y + r.Height / 2.0f);
	}

	AccionMenu Menu::clicVolver(int mx, int my) {
		if (rectVolver().Contains((float)mx, (float)my)) return AccionMenu::Volver;
		return AccionMenu::Ninguna;
	}
}
