#include "MenuJuego.h"
#include "Fondo.h"
#include "Estilos.h"
#include "Config.h"
#include "Astronauta.h"
#include "Estrella.h"
#include "SistemaSolar.h"

namespace JuegoEspacial {

	MenuJuego::MenuJuego() {
		opcion = 0;
		tarjeta = 0;
		personajeClicado = -1;
		volverMarcado = false;
		animacion = 0.0f;
	}

	Color MenuJuego::colorPersonaje(int indice) {
		switch (indice) {
		case 0: return Color::FromArgb(70, 130, 220);    // azul comandante
		case 1: return Color::FromArgb(235, 140, 50);    // naranja ingeniero
		case 2: return Color::FromArgb(90, 190, 110);    // verde cientifico
		default: return Color::FromArgb(220, 80, 80);    // rojo explorador
		}
	}

	void MenuJuego::reiniciarAnimacion() {
		animacion = 0.0f;
	}

	void MenuJuego::avanzarAnimacion() {
		animacion += 0.045f;
		if (animacion > 1.0f) animacion = 1.0f;
	}

	// ================= PANTALLA PRINCIPAL =================

	RectangleF MenuJuego::rectBoton(int indice) {
		return RectangleF(370.0f, 268.0f + indice * 74.0f, 260.0f, 58.0f);
	}

	void MenuJuego::dibujarPrincipal(Graphics^ g, int tick) {
		avanzarAnimacion();
		Fondo::dibujar(g, tick);

		// planeta decorativo con anillos
		SistemaSolar::dibujarPlaneta(g, 5, 850.0f, 560.0f, 72.0f, tick);
		// planeta pequeno arriba a la izquierda
		SistemaSolar::dibujarPlaneta(g, 7, 110.0f, 110.0f, 34.0f, tick);

		// cohete que pasea por el menu
		float naveX = 130.0f + (float)Math::Sin(tick * 0.011) * 70.0f;
		float naveY = 470.0f + (float)Math::Cos(tick * 0.014) * 45.0f;
		g->TranslateTransform(naveX, naveY);
		g->RotateTransform(90.0f + (float)Math::Sin(tick * 0.02) * 6.0f);
		// cuerpo simplificado del cohete decorativo
		SolidBrush casco(Color::FromArgb(235, 238, 244));
		g->FillEllipse(%casco, -13.0f, -86.0f, 26.0f, 34.0f);
		g->FillRectangle(%casco, -13.0f, -70.0f, 26.0f, 52.0f);
		SolidBrush rojo(Color::FromArgb(205, 60, 60));
		g->FillPie(%rojo, -13.0f, -86.0f, 26.0f, 34.0f, 180.0f, 180.0f);
		array<PointF>^ aleta = gcnew array<PointF>(3);
		aleta[0] = PointF(-13.0f, -34.0f);
		aleta[1] = PointF(-24.0f, -2.0f);
		aleta[2] = PointF(-13.0f, -10.0f);
		g->FillPolygon(%rojo, aleta);
		aleta[0] = PointF(13.0f, -34.0f);
		aleta[1] = PointF(24.0f, -2.0f);
		aleta[2] = PointF(13.0f, -10.0f);
		g->FillPolygon(%rojo, aleta);
		SolidBrush vidrio(Color::FromArgb(70, 160, 210));
		g->FillEllipse(%vidrio, -6.0f, -62.0f, 12.0f, 12.0f);
		float flicker = (float)Math::Sin(tick * 0.5) * 4.0f;
		array<PointF>^ fuego = gcnew array<PointF>(3);
		fuego[0] = PointF(-7.0f, -16.0f);
		fuego[1] = PointF(7.0f, -16.0f);
		fuego[2] = PointF(0.0f, 12.0f + flicker);
		SolidBrush llama(Color::FromArgb(220, 255, 170, 60));
		g->FillPolygon(%llama, fuego);
		g->ResetTransform();

		// titulo con sombra y subtitulo
		float entrada = (1.0f - animacion) * -120.0f;
		Estilos::textoSombra(g, L"ODISEA ESPACIAL", Estilos::titulo, Color::FromArgb(255, 215, 90), 500.0f, 120.0f + entrada);
		Estilos::textoCentrado(g, L"Explora los 8 planetas y completa sus misiones",
			Estilos::subtitulo, Color::FromArgb(150, 200, 255), 500.0f, 185.0f + entrada);
		Estilos::textoCentrado(g, L"· hecho 100% con figuras geométricas ·",
			Estilos::pequena, Color::FromArgb(120, 140, 190), 500.0f, 215.0f + entrada);

		// botones
		dibujarBoton(g, 0, L"INICIAR", tick);
		dibujarBoton(g, 1, L"PERSONAJES", tick);
		dibujarBoton(g, 2, L"CONTROLES", tick);
		dibujarBoton(g, 3, L"CRÉDITOS", tick);
		dibujarBoton(g, 4, L"SALIR", tick);

		Estilos::textoCentrado(g, L"Usa el RATÓN o las flechas ↑ ↓ y ENTER",
			Estilos::pequena, Color::FromArgb(130, 150, 200), 500.0f, 660.0f);
	}

	void MenuJuego::dibujarBoton(Graphics^ g, int indice, String^ texto, int tick) {
		RectangleF r = rectBoton(indice);
		bool marcado = (opcion == indice);
		float despl = marcado ? 8.0f : 0.0f;
		float bx = r.X + despl;

		// retraso escalonado al entrar
		float retraso = (1.0f - animacion) * (60.0f + indice * 50.0f);
		bx += retraso;

		if (marcado) {
			int alfa = 160 + (int)(60.0 * Math::Sin(tick * 0.12));
			Estilos::rellenarRedondeado(g, Color::FromArgb(230, 35, 80, 150), bx, r.Y, r.Width, r.Height, 14.0f);
			Estilos::bordeRedondeado(g, Color::FromArgb(alfa, 110, 230, 255), 2.5f, bx - 3.0f, r.Y - 3.0f, r.Width + 6.0f, r.Height + 6.0f, 16.0f);
			// flecha indicadora
			array<PointF>^ flecha = gcnew array<PointF>(3);
			flecha[0] = PointF(bx - 26.0f, r.Y + r.Height / 2.0f - 9.0f);
			flecha[1] = PointF(bx - 26.0f, r.Y + r.Height / 2.0f + 9.0f);
			flecha[2] = PointF(bx - 12.0f, r.Y + r.Height / 2.0f);
			SolidBrush punta(Color::FromArgb(110, 230, 255));
			g->FillPolygon(%punta, flecha);
		}
		else {
			Estilos::rellenarRedondeado(g, Color::FromArgb(200, 16, 26, 56), bx, r.Y, r.Width, r.Height, 14.0f);
			Estilos::bordeRedondeado(g, Color::FromArgb(150, 70, 95, 150), 1.5f, bx, r.Y, r.Width, r.Height, 14.0f);
		}

		// icono geometrico de cada boton
		float icx = bx + 34.0f;
		float icy = r.Y + r.Height / 2.0f;
		if (indice == 0) {
			array<PointF>^ play = gcnew array<PointF>(3);
			play[0] = PointF(icx - 7.0f, icy - 10.0f);
			play[1] = PointF(icx - 7.0f, icy + 10.0f);
			play[2] = PointF(icx + 11.0f, icy);
			SolidBrush verde(Color::FromArgb(120, 230, 130));
			g->FillPolygon(%verde, play);
		}
		else if (indice == 1) {
			dibujarCascoMini(g, icx, icy, 11.0f, colorPersonaje(0));
		}
		else if (indice == 2) {
			Estilos::rellenarRedondeado(g, Color::FromArgb(210, 215, 230), icx - 11.0f, icy - 9.0f, 22.0f, 18.0f, 4.0f);
			Estilos::textoCentrado(g, L"W", Estilos::pequena, Color::FromArgb(40, 45, 60), icx, icy);
		}
		else if (indice == 3) {
			Estrella::dibujarForma(g, icx, icy, 11.0f, (float)(tick % 360), Color::FromArgb(255, 210, 70));
		}
		else {
			SolidBrush puerta(Color::FromArgb(200, 120, 90));
			g->FillRectangle(%puerta, icx - 8.0f, icy - 11.0f, 16.0f, 22.0f);
			SolidBrush pomo(Color::FromArgb(255, 230, 140));
			g->FillEllipse(%pomo, icx + 2.0f, icy - 2.0f, 4.0f, 4.0f);
		}

		Estilos::textoCentrado(g, texto, Estilos::boton, Color::White, bx + r.Width / 2.0f + 14.0f, icy);
	}

	bool MenuJuego::ratonPrincipal(int mx, int my) {
		for (int i = 0; i < 5; i++) {
			if (rectBoton(i).Contains((float)mx, (float)my)) {
				if (opcion != i) {
					opcion = i;
					return true;
				}
				return false;
			}
		}
		return false;
	}

	AccionMenu MenuJuego::clicPrincipal(int mx, int my) {
		for (int i = 0; i < 5; i++) {
			if (rectBoton(i).Contains((float)mx, (float)my)) {
				opcion = i;
				return accionSeleccionada();
			}
		}
		return AccionMenu::Ninguna;
	}

	void MenuJuego::moverSeleccion(int direccion) {
		opcion = (opcion + direccion + 5) % 5;
	}

	AccionMenu MenuJuego::accionSeleccionada() {
		switch (opcion) {
		case 0: return AccionMenu::Iniciar;
		case 1: return AccionMenu::Personajes;
		case 2: return AccionMenu::Controles;
		case 3: return AccionMenu::Creditos;
		default: return AccionMenu::Salir;
		}
	}

	// ================= PERSONAJES =================

	RectangleF MenuJuego::rectTarjeta(int indice) {
		return RectangleF(68.0f + indice * 222.0f, 168.0f, 200.0f, 396.0f);
	}

	void MenuJuego::dibujarPersonajes(Graphics^ g, int tick, int seleccionado) {
		avanzarAnimacion();
		Fondo::dibujar(g, tick);
		dibujarTituloPantalla(g, L"ELIGE TU ASTRONAUTA");

		for (int i = 0; i < 4; i++) {
			RectangleF r = rectTarjeta(i);
			float subida = (1.0f - animacion) * (120.0f + i * 60.0f);
			float ty = r.Y + subida;
			bool marcada = (tarjeta == i);
			bool elegida = (seleccionado == i);

			if (marcada) ty -= 8.0f;

			Estilos::rellenarRedondeado(g, Color::FromArgb(215, 16, 26, 56), r.X, ty, r.Width, r.Height, 16.0f);
			if (elegida) {
				Estilos::bordeRedondeado(g, Color::FromArgb(255, 255, 210, 70), 3.0f, r.X - 2.0f, ty - 2.0f, r.Width + 4.0f, r.Height + 4.0f, 18.0f);
			}
			else if (marcada) {
				Estilos::bordeRedondeado(g, Color::FromArgb(220, 110, 230, 255), 2.5f, r.X, ty, r.Width, r.Height, 16.0f);
			}
			else {
				Estilos::bordeRedondeado(g, Color::FromArgb(140, 70, 95, 150), 1.5f, r.X, ty, r.Width, r.Height, 16.0f);
			}

			// el astronauta marcha en su tarjeta cuando esta marcada
			float fase = marcada ? tick * 0.18f : 0.0f;
			Astronauta::dibujarModelo(g, r.X + r.Width / 2.0f, ty + 218.0f, 1.55f,
				colorPersonaje(i), i, fase, false, false);

			Estilos::textoCentrado(g, nombres[i], Estilos::subtitulo, Color::White, r.X + r.Width / 2.0f, ty + 252.0f);
			Estilos::textoCentrado(g, titulos[i], Estilos::normalNegrita, colorPersonaje(i), r.X + r.Width / 2.0f, ty + 280.0f);

			// descripcion con ajuste de linea automatico
			SolidBrush plata(Color::FromArgb(185, 200, 230));
			RectangleF zonaTexto(r.X + 12.0f, ty + 300.0f, r.Width - 24.0f, 84.0f);
			g->DrawString(descripciones[i], Estilos::pequena, %plata, zonaTexto, Estilos::centroArriba);

			if (elegida) {
				Estilos::rellenarRedondeado(g, Color::FromArgb(255, 210, 70), r.X + 30.0f, ty + r.Height - 26.0f, r.Width - 60.0f, 22.0f, 8.0f);
				Estilos::textoCentrado(g, L"SELECCIONADO", Estilos::pequena, Color::FromArgb(60, 45, 10), r.X + r.Width / 2.0f, ty + r.Height - 15.0f);
			}
		}

		Estilos::textoCentrado(g, L"Haz CLIC en una tarjeta para elegir tu personaje (o ← → y ENTER)",
			Estilos::normal, Color::FromArgb(150, 170, 215), 500.0f, 596.0f);
		dibujarVolver(g);
	}

	AccionMenu MenuJuego::clicPersonajes(int mx, int my) {
		for (int i = 0; i < 4; i++) {
			if (rectTarjeta(i).Contains((float)mx, (float)my)) {
				personajeClicado = i;
				tarjeta = i;
				return AccionMenu::ElegirPersonaje;
			}
		}
		if (rectVolver().Contains((float)mx, (float)my)) return AccionMenu::Volver;
		return AccionMenu::Ninguna;
	}

	bool MenuJuego::ratonPersonajes(int mx, int my) {
		bool cambio = false;
		for (int i = 0; i < 4; i++) {
			if (rectTarjeta(i).Contains((float)mx, (float)my)) {
				if (tarjeta != i) {
					tarjeta = i;
					cambio = true;
				}
			}
		}
		if (ratonVolver(mx, my)) cambio = true;
		return cambio;
	}

	void MenuJuego::moverPersonaje(int direccion) {
		tarjeta = (tarjeta + direccion + 4) % 4;
	}

	int MenuJuego::getTarjetaMarcada() { return tarjeta; }
	int MenuJuego::getPersonajeClicado() { return personajeClicado; }

	// ================= CONTROLES =================

	void MenuJuego::dibujarControles(Graphics^ g, int tick) {
		avanzarAnimacion();
		Fondo::dibujar(g, tick);
		dibujarTituloPantalla(g, L"CONTROLES");

		float entrada = (1.0f - animacion) * 200.0f;
		float panelX = 120.0f + entrada;
		Estilos::rellenarRedondeado(g, Color::FromArgb(205, 14, 22, 48), panelX, 150.0f, 760.0f, 440.0f, 18.0f);
		Estilos::bordeRedondeado(g, Color::FromArgb(160, 90, 140, 220), 2.0f, panelX, 150.0f, 760.0f, 440.0f, 18.0f);

		float fila = 190.0f;
		Color celeste = Color::FromArgb(140, 210, 255);
		Color blanco = Color::FromArgb(225, 232, 245);

		// menus
		Estilos::textoIzquierda(g, L"MENÚS", Estilos::normalNegrita, celeste, panelX + 36.0f, fila);
		dibujarTecla(g, L"↑", panelX + 170.0f, fila - 15.0f, 34.0f);
		dibujarTecla(g, L"↓", panelX + 210.0f, fila - 15.0f, 34.0f);
		dibujarTecla(g, L"ENTER", panelX + 250.0f, fila - 15.0f, 70.0f);
		dibujarRaton(g, panelX + 336.0f, fila - 15.0f);
		Estilos::textoIzquierda(g, L"moverse y elegir; el ratón también funciona", Estilos::normal, blanco, panelX + 376.0f, fila);

		// caminar
		fila += 62.0f;
		Estilos::textoIzquierda(g, L"CAMINAR", Estilos::normalNegrita, celeste, panelX + 36.0f, fila);
		dibujarTecla(g, L"A", panelX + 170.0f, fila - 15.0f, 34.0f);
		dibujarTecla(g, L"D", panelX + 210.0f, fila - 15.0f, 34.0f);
		dibujarTecla(g, L"←", panelX + 250.0f, fila - 15.0f, 34.0f);
		dibujarTecla(g, L"→", panelX + 290.0f, fila - 15.0f, 34.0f);
		Estilos::textoIzquierda(g, L"mover al astronauta por el planeta", Estilos::normal, blanco, panelX + 340.0f, fila);

		// volar
		fila += 62.0f;
		Estilos::textoIzquierda(g, L"VOLAR", Estilos::normalNegrita, celeste, panelX + 36.0f, fila);
		dibujarTecla(g, L"W", panelX + 170.0f, fila - 15.0f, 34.0f);
		dibujarTecla(g, L"A", panelX + 210.0f, fila - 15.0f, 34.0f);
		dibujarTecla(g, L"S", panelX + 250.0f, fila - 15.0f, 34.0f);
		dibujarTecla(g, L"D", panelX + 290.0f, fila - 15.0f, 34.0f);
		Estilos::textoIzquierda(g, L"jetpack en Marte · la nave en Saturno usa ↑ ↓", Estilos::normal, blanco, panelX + 340.0f, fila);

		// disparar
		fila += 62.0f;
		Estilos::textoIzquierda(g, L"DISPARAR", Estilos::normalNegrita, celeste, panelX + 36.0f, fila);
		dibujarTecla(g, L"ESPACIO", panelX + 170.0f, fila - 15.0f, 110.0f);
		Estilos::textoIzquierda(g, L"láser contra la basura espacial y el jefe", Estilos::normal, blanco, panelX + 340.0f, fila);

		// urano
		fila += 62.0f;
		Estilos::textoIzquierda(g, L"URANO", Estilos::normalNegrita, celeste, panelX + 36.0f, fila);
		dibujarRaton(g, panelX + 170.0f, fila - 15.0f);
		Estilos::textoIzquierda(g, L"haz CLIC en los cristales para repetir la melodía", Estilos::normal, blanco, panelX + 340.0f, fila);

		// salir
		fila += 62.0f;
		Estilos::textoIzquierda(g, L"VOLVER", Estilos::normalNegrita, celeste, panelX + 36.0f, fila);
		dibujarTecla(g, L"ESC", panelX + 170.0f, fila - 15.0f, 54.0f);
		Estilos::textoIzquierda(g, L"regresar al menú o abandonar una misión", Estilos::normal, blanco, panelX + 340.0f, fila);

		dibujarVolver(g);
	}

	void MenuJuego::dibujarTecla(Graphics^ g, String^ letra, float x, float y, float ancho) {
		// tecla con relieve
		Estilos::rellenarRedondeado(g, Color::FromArgb(60, 65, 85), x, y + 3.0f, ancho, 30.0f, 6.0f);
		Estilos::rellenarRedondeado(g, Color::FromArgb(215, 220, 235), x, y, ancho, 28.0f, 6.0f);
		Estilos::textoCentrado(g, letra, Estilos::pequena, Color::FromArgb(40, 45, 60), x + ancho / 2.0f, y + 14.0f);
	}

	void MenuJuego::dibujarRaton(Graphics^ g, float x, float y) {
		Estilos::rellenarRedondeado(g, Color::FromArgb(215, 220, 235), x, y, 22.0f, 32.0f, 10.0f);
		Pen division(Color::FromArgb(120, 125, 140), 1.5f);
		g->DrawLine(%division, x + 11.0f, y, x + 11.0f, y + 13.0f);
		g->DrawLine(%division, x, y + 13.0f, x + 22.0f, y + 13.0f);
		SolidBrush rueda(Color::FromArgb(120, 125, 140));
		g->FillRectangle(%rueda, x + 9.5f, y + 4.0f, 3.0f, 6.0f);
	}

	// ================= CREDITOS =================

	void MenuJuego::dibujarCreditos(Graphics^ g, int tick) {
		avanzarAnimacion();
		Fondo::dibujar(g, tick);
		dibujarTituloPantalla(g, L"CRÉDITOS");

		array<String^>^ nombresCompletos = gcnew array<String^> {
			L"Bryan Cordova", L"Lenin Palomino", L"Gustavo Arimana", L"Mustafa Altuntas"
		};
		array<String^>^ roles = gcnew array<String^> {
			L"Programación y lógica del juego  ·  Developer",
			L"Diseño visual con figuras geométricas  ·  Diseñador",
			L"Jugabilidad y pruebas  ·  QA Tester",
			L"Sonido, UML y documentación  ·  Productor"
		};

		for (int i = 0; i < 4; i++) {
			float lado = (i % 2 == 0) ? -1.0f : 1.0f;
			float entrada = (1.0f - animacion) * 320.0f * lado;
			float fy = 180.0f + i * 95.0f;

			Estilos::rellenarRedondeado(g, Color::FromArgb(195, 14, 22, 48), 220.0f + entrada, fy, 560.0f, 76.0f, 14.0f);
			Estilos::bordeRedondeado(g, Color::FromArgb(130, colorPersonaje(i)), 2.0f, 220.0f + entrada, fy, 560.0f, 76.0f, 14.0f);

			dibujarCascoMini(g, 268.0f + entrada, fy + 38.0f, 22.0f, colorPersonaje(i));
			Estilos::textoIzquierda(g, nombresCompletos[i], Estilos::subtitulo, Color::White, 310.0f + entrada, fy + 26.0f);
			Estilos::textoIzquierda(g, roles[i], Estilos::normal, Color::FromArgb(165, 195, 235), 310.0f + entrada, fy + 54.0f);

			// estrellita decorativa
			Estrella::dibujarForma(g, 744.0f + entrada, fy + 38.0f, 12.0f, (float)((tick * 2 + i * 45) % 360), Color::FromArgb(255, 210, 70));
		}

		Estilos::textoCentrado(g, L"Proyecto en C++/CLI · Windows Forms · GDI+ · Sin sprites: solo figuras geométricas",
			Estilos::pequena, Color::FromArgb(130, 150, 200), 500.0f, 600.0f);
		dibujarVolver(g);
	}

	void MenuJuego::dibujarCascoMini(Graphics^ g, float cx, float cy, float r, Color color) {
		SolidBrush casco(color);
		g->FillEllipse(%casco, cx - r, cy - r, r * 2.0f, r * 2.0f);
		SolidBrush visor(Color::FromArgb(25, 35, 70));
		g->FillEllipse(%visor, cx - r * 0.55f, cy - r * 0.35f, r * 1.3f, r * 0.95f);
		SolidBrush reflejo(Color::FromArgb(150, 210, 235, 255));
		g->FillEllipse(%reflejo, cx - r * 0.25f, cy - r * 0.15f, r * 0.4f, r * 0.3f);
	}

	// ================= COMUNES =================

	void MenuJuego::dibujarTituloPantalla(Graphics^ g, String^ texto) {
		Estilos::textoSombra(g, texto, Estilos::enorme, Color::FromArgb(255, 215, 90), 500.0f, 80.0f);
		Pen subrayado(Color::FromArgb(150, 110, 230, 255), 2.5f);
		g->DrawLine(%subrayado, 330.0f, 118.0f, 670.0f, 118.0f);
	}

	RectangleF MenuJuego::rectVolver() {
		return RectangleF(36.0f, 622.0f, 190.0f, 46.0f);
	}

	void MenuJuego::dibujarVolver(Graphics^ g) {
		RectangleF r = rectVolver();
		if (volverMarcado) {
			Estilos::rellenarRedondeado(g, Color::FromArgb(230, 35, 80, 150), r.X, r.Y, r.Width, r.Height, 12.0f);
			Estilos::bordeRedondeado(g, Color::FromArgb(220, 110, 230, 255), 2.0f, r.X, r.Y, r.Width, r.Height, 12.0f);
		}
		else {
			Estilos::rellenarRedondeado(g, Color::FromArgb(200, 16, 26, 56), r.X, r.Y, r.Width, r.Height, 12.0f);
			Estilos::bordeRedondeado(g, Color::FromArgb(150, 70, 95, 150), 1.5f, r.X, r.Y, r.Width, r.Height, 12.0f);
		}
		Estilos::textoCentrado(g, L"← VOLVER (ESC)", Estilos::normalNegrita, Color::White, r.X + r.Width / 2.0f, r.Y + r.Height / 2.0f);
	}

	bool MenuJuego::ratonVolver(int mx, int my) {
		bool antes = volverMarcado;
		volverMarcado = rectVolver().Contains((float)mx, (float)my);
		return volverMarcado && !antes;
	}

	AccionMenu MenuJuego::clicVolver(int mx, int my) {
		if (rectVolver().Contains((float)mx, (float)my)) return AccionMenu::Volver;
		return AccionMenu::Ninguna;
	}
}
