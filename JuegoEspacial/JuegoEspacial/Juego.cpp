#include "Juego.h"
#include "Config.h"
#include "Sonido.h"
#include "Estilos.h"
#include "Fondo.h"
#include "Azar.h"
#include "Estrella.h"
#include "MinijuegoMercurio.h"
#include "MinijuegoVenus.h"
#include "MinijuegoTierra.h"
#include "MinijuegoMarte.h"
#include "MinijuegoJupiter.h"
#include "MinijuegoSaturno.h"
#include "MinijuegoUrano.h"
#include "MinijuegoNeptuno.h"

using namespace System::Windows::Forms;

namespace JuegoEspacial {

	Juego::Juego() {
		nivel = 0;
		estado = L"MENU";
		tick = 0;
		tickEstado = 0;
		menu = gcnew MenuJuego();
		sistema = gcnew SistemaSolar();
		transicion = nullptr;
		minijuego = nullptr;
		entrada = gcnew Entrada();
		completados = gcnew array<bool>(8);
		fiesta = gcnew List<Particula^>();
		personajeSel = 0;
		planetaActual = 0;
		subMini = 0;
		ganadoUltimo = false;
		bonusDado = false;
		jugador = gcnew Astronauta(MenuJuego::nombres[0], MenuJuego::colorPersonaje(0), 0, 500.0f, Config::SUELO);
	}

	String^ Juego::getEstado() { return estado; }
	int Juego::getNivel() { return nivel; }

	void Juego::cambiarEstado(String^ nuevo) {
		estado = nuevo;
		tickEstado = 0;
		menu->reiniciarAnimacion();
	}

	void Juego::iniciar() {
		cambiarEstado(L"SISTEMA");
		Sonido::seleccionar();
	}

	void Juego::finalizar() {
		fiesta->Clear();
		cambiarEstado(L"FINAL");
		Sonido::victoria();
	}

	void Juego::volverAlMenu() {
		cambiarEstado(L"MENU");
		Sonido::mover();
	}

	Minijuego^ Juego::crearMinijuego(int indice) {
		switch (indice) {
		case 0: return gcnew MinijuegoMercurio(jugador);
		case 1: return gcnew MinijuegoVenus(jugador);
		case 2: return gcnew MinijuegoTierra(jugador);
		case 3: return gcnew MinijuegoMarte(jugador);
		case 4: return gcnew MinijuegoJupiter(jugador);
		case 5: return gcnew MinijuegoSaturno(jugador);
		case 6: return gcnew MinijuegoUrano(jugador);
		default: return gcnew MinijuegoNeptuno(jugador);
		}
	}

	// ================= LOGICA PRINCIPAL =================

	void Juego::actualizar() {
		tick++;
		tickEstado++;

		if (estado == L"ATERRIZAJE") {
			transicion->actualizar();
			if (transicion->haTerminado()) {
				minijuego = crearMinijuego(planetaActual);
				minijuego->iniciar();
				subMini = 0;
				cambiarEstado(L"MINIJUEGO");
			}
		}
		else if (estado == L"DESPEGUE") {
			transicion->actualizar();
			if (transicion->haTerminado()) {
				if (nivel >= 8) finalizar();
				else cambiarEstado(L"SISTEMA");
			}
		}
		else if (estado == L"MINIJUEGO") {
			if (subMini == 1) {
				minijuego->actualizar(entrada);
				if (minijuego->ganado() || minijuego->perdido()) {
					subMini = 2;
					terminarMinijuego();
				}
			}
			else if (subMini == 2) {
				// las particulas siguen animandose detras del cartel
				minijuego->actualizar(entrada);
			}
		}
		else if (estado == L"FINAL") {
			if (tickEstado % 18 == 0) {
				Color color;
				switch (Azar::entre(0, 6)) {
				case 0: color = Color::FromArgb(255, 90, 90); break;
				case 1: color = Color::FromArgb(255, 210, 70); break;
				case 2: color = Color::FromArgb(110, 230, 120); break;
				case 3: color = Color::FromArgb(90, 200, 255); break;
				case 4: color = Color::FromArgb(220, 110, 250); break;
				default: color = Color::FromArgb(255, 160, 80); break;
				}
				Particulas::explosion(fiesta, Azar::entreF(120.0f, 880.0f), Azar::entreF(80.0f, 400.0f), color, 26);
				if (tickEstado % 54 == 0) Sonido::tono(523 + Azar::entre(0, 4) * 110, 60);
			}
			Particulas::actualizar(fiesta);
		}
	}

	void Juego::terminarMinijuego() {
		ganadoUltimo = minijuego->ganado();
		bonusDado = false;
		if (ganadoUltimo && !completados[planetaActual]) {
			completados[planetaActual] = true;
			nivel++;
			jugador->sumarPuntaje(100);
			bonusDado = true;
		}
	}

	void Juego::entrarPlaneta(int indice) {
		if (!sistema->desbloqueado(indice, completados)) {
			Sonido::error();
			return;
		}
		planetaActual = indice;
		transicion = gcnew Aterrizaje(indice, false, jugador);
		cambiarEstado(L"ATERRIZAJE");
		Sonido::seleccionar();
	}

	void Juego::irADespegue() {
		transicion = gcnew Aterrizaje(planetaActual, true, jugador);
		cambiarEstado(L"DESPEGUE");
	}

	void Juego::elegirPersonaje(int indice) {
		personajeSel = indice;
		jugador->cambiarPerfil(MenuJuego::nombres[indice], MenuJuego::colorPersonaje(indice), indice);
		Sonido::seleccionar();
	}

	void Juego::ejecutarAccion(AccionMenu accion) {
		switch (accion) {
		case AccionMenu::Iniciar:
			iniciar();
			break;
		case AccionMenu::Personajes:
			cambiarEstado(L"PERSONAJES");
			Sonido::seleccionar();
			break;
		case AccionMenu::Controles:
			cambiarEstado(L"CONTROLES");
			Sonido::seleccionar();
			break;
		case AccionMenu::Creditos:
			cambiarEstado(L"CREDITOS");
			Sonido::seleccionar();
			break;
		case AccionMenu::Salir:
			Application::Exit();
			break;
		case AccionMenu::Volver:
			volverAlMenu();
			break;
		default:
			break;
		}
	}

	// ================= ENTRADA =================

	void Juego::teclaAbajo(Keys tecla) {
		// teclas mantenidas para los minijuegos
		if (tecla == Keys::Left || tecla == Keys::A) entrada->izquierda = true;
		if (tecla == Keys::Right || tecla == Keys::D) entrada->derecha = true;
		if (tecla == Keys::Up || tecla == Keys::W) entrada->arriba = true;
		if (tecla == Keys::Down || tecla == Keys::S) entrada->abajo = true;
		if (tecla == Keys::Space) entrada->disparo = true;

		if (estado == L"MENU") {
			if (tecla == Keys::Up || tecla == Keys::W) { menu->moverSeleccion(-1); Sonido::mover(); }
			else if (tecla == Keys::Down || tecla == Keys::S) { menu->moverSeleccion(1); Sonido::mover(); }
			else if (tecla == Keys::Return || tecla == Keys::Space) ejecutarAccion(menu->accionSeleccionada());
			else if (tecla == Keys::Escape) Application::Exit();
		}
		else if (estado == L"PERSONAJES") {
			if (tecla == Keys::Left || tecla == Keys::A) { menu->moverPersonaje(-1); Sonido::mover(); }
			else if (tecla == Keys::Right || tecla == Keys::D) { menu->moverPersonaje(1); Sonido::mover(); }
			else if (tecla == Keys::Return || tecla == Keys::Space) elegirPersonaje(menu->getTarjetaMarcada());
			else if (tecla == Keys::Escape) volverAlMenu();
		}
		else if (estado == L"CONTROLES" || estado == L"CREDITOS") {
			if (tecla == Keys::Escape || tecla == Keys::Return) volverAlMenu();
		}
		else if (estado == L"SISTEMA") {
			if (tecla == Keys::Left || tecla == Keys::A) { sistema->marcarConTeclado(-1); Sonido::mover(); }
			else if (tecla == Keys::Right || tecla == Keys::D) { sistema->marcarConTeclado(1); Sonido::mover(); }
			else if (tecla == Keys::Return || tecla == Keys::Space) {
				if (sistema->getMarcado() != -1) entrarPlaneta(sistema->getMarcado());
			}
			else if (tecla == Keys::Escape) volverAlMenu();
		}
		else if (estado == L"ATERRIZAJE" || estado == L"DESPEGUE") {
			if (tecla == Keys::Return || tecla == Keys::Space || tecla == Keys::Escape) transicion->saltar();
		}
		else if (estado == L"MINIJUEGO") {
			if (subMini == 0) {
				if (tecla == Keys::Return || tecla == Keys::Space) { subMini = 1; Sonido::seleccionar(); }
				else if (tecla == Keys::Escape) irADespegue();
			}
			else if (subMini == 1) {
				if (tecla == Keys::Escape) irADespegue();
			}
			else {
				if (tecla == Keys::Return || tecla == Keys::Space) irADespegue();
				else if (tecla == Keys::R) {
					minijuego = crearMinijuego(planetaActual);
					minijuego->iniciar();
					subMini = 0;
					Sonido::seleccionar();
				}
				else if (tecla == Keys::Escape) irADespegue();
			}
		}
		else if (estado == L"FINAL") {
			if (tecla == Keys::Return || tecla == Keys::Space || tecla == Keys::Escape) {
				cambiarEstado(L"MENU");
				Sonido::seleccionar();
			}
		}
	}

	void Juego::teclaArriba(Keys tecla) {
		if (tecla == Keys::Left || tecla == Keys::A) entrada->izquierda = false;
		if (tecla == Keys::Right || tecla == Keys::D) entrada->derecha = false;
		if (tecla == Keys::Up || tecla == Keys::W) entrada->arriba = false;
		if (tecla == Keys::Down || tecla == Keys::S) entrada->abajo = false;
		if (tecla == Keys::Space) entrada->disparo = false;
	}

	void Juego::ratonMovido(int mx, int my) {
		entrada->ratonX = mx;
		entrada->ratonY = my;

		if (estado == L"MENU") {
			if (menu->ratonPrincipal(mx, my)) Sonido::mover();
		}
		else if (estado == L"PERSONAJES") {
			if (menu->ratonPersonajes(mx, my)) Sonido::mover();
		}
		else if (estado == L"CONTROLES" || estado == L"CREDITOS") {
			if (menu->ratonVolver(mx, my)) Sonido::mover();
		}
		else if (estado == L"SISTEMA") {
			if (sistema->moverRaton(mx, my)) Sonido::mover();
		}
	}

	void Juego::clic(int mx, int my) {
		if (estado == L"MENU") {
			AccionMenu accion = menu->clicPrincipal(mx, my);
			if (accion != AccionMenu::Ninguna) ejecutarAccion(accion);
		}
		else if (estado == L"PERSONAJES") {
			AccionMenu accion = menu->clicPersonajes(mx, my);
			if (accion == AccionMenu::ElegirPersonaje) elegirPersonaje(menu->getPersonajeClicado());
			else if (accion == AccionMenu::Volver) volverAlMenu();
		}
		else if (estado == L"CONTROLES" || estado == L"CREDITOS") {
			if (menu->clicVolver(mx, my) == AccionMenu::Volver) volverAlMenu();
		}
		else if (estado == L"SISTEMA") {
			int indice = sistema->planetaEn(mx, my);
			if (indice != -1) entrarPlaneta(indice);
		}
		else if (estado == L"ATERRIZAJE" || estado == L"DESPEGUE") {
			transicion->saltar();
		}
		else if (estado == L"MINIJUEGO") {
			if (subMini == 0) { subMini = 1; Sonido::seleccionar(); }
			else if (subMini == 1) minijuego->clic(mx, my);
			else irADespegue();
		}
		else if (estado == L"FINAL") {
			cambiarEstado(L"MENU");
			Sonido::seleccionar();
		}
	}

	// ================= DIBUJO =================

	void Juego::dibujar(Graphics^ g) {
		if (estado == L"MENU") {
			menu->dibujarPrincipal(g, tick);
		}
		else if (estado == L"PERSONAJES") {
			menu->dibujarPersonajes(g, tick, personajeSel);
		}
		else if (estado == L"CONTROLES") {
			menu->dibujarControles(g, tick);
		}
		else if (estado == L"CREDITOS") {
			menu->dibujarCreditos(g, tick);
		}
		else if (estado == L"SISTEMA") {
			sistema->dibujar(g, tick, completados, jugador->getPuntaje(), jugador->getNombre(), jugador->getColorTraje());
		}
		else if (estado == L"ATERRIZAJE" || estado == L"DESPEGUE") {
			transicion->dibujar(g, tick);
		}
		else if (estado == L"MINIJUEGO") {
			minijuego->dibujar(g);
			minijuego->dibujarHUD(g);
			if (subMini == 0) dibujarIntro(g);
			else if (subMini == 2) dibujarResultado(g);
		}
		else if (estado == L"FINAL") {
			dibujarFinal(g);
		}
	}

	void Juego::dibujarIntro(Graphics^ g) {
		// velo oscuro sobre el escenario
		SolidBrush velo(Color::FromArgb(130, 0, 0, 10));
		g->FillRectangle(%velo, 0.0f, 0.0f, (float)Config::ANCHO, (float)Config::ALTO);

		Estilos::rellenarRedondeado(g, Color::FromArgb(235, 10, 16, 38), 180.0f, 150.0f, 640.0f, 400.0f, 20.0f);
		Estilos::bordeRedondeado(g, Color::FromArgb(200, 90, 200, 255), 2.5f, 180.0f, 150.0f, 640.0f, 400.0f, 20.0f);

		Estilos::textoCentrado(g, String::Format(L"MISIÓN EN {0}", SistemaSolar::nombre(minijuego->getPlaneta())),
			Estilos::normalNegrita, Color::FromArgb(120, 210, 255), 500.0f, 192.0f);
		Estilos::textoSombra(g, minijuego->getTitulo(), Estilos::grande, Color::FromArgb(255, 215, 90), 500.0f, 240.0f);

		SolidBrush texto(Color::FromArgb(215, 225, 245));
		RectangleF zonaTexto(230.0f, 290.0f, 540.0f, 150.0f);
		g->DrawString(minijuego->getInstrucciones(), Estilos::normal, %texto, zonaTexto, Estilos::centroArriba);

		int alfa = 150 + (int)(100.0 * Math::Sin(tickEstado * 0.1));
		if (alfa < 0) alfa = 0;
		if (alfa > 255) alfa = 255;
		Estilos::textoCentrado(g, L"ENTER o CLIC para comenzar", Estilos::subtitulo,
			Color::FromArgb(alfa, 140, 250, 150), 500.0f, 470.0f);
		Estilos::textoCentrado(g, L"ESC para abandonar la misión", Estilos::pequena,
			Color::FromArgb(130, 150, 200), 500.0f, 520.0f);
	}

	void Juego::dibujarResultado(Graphics^ g) {
		SolidBrush velo(Color::FromArgb(140, 0, 0, 10));
		g->FillRectangle(%velo, 0.0f, 0.0f, (float)Config::ANCHO, (float)Config::ALTO);

		Color borde = ganadoUltimo ? Color::FromArgb(110, 230, 130) : Color::FromArgb(240, 90, 90);
		Estilos::rellenarRedondeado(g, Color::FromArgb(235, 10, 16, 38), 230.0f, 160.0f, 540.0f, 360.0f, 20.0f);
		Estilos::bordeRedondeado(g, borde, 3.0f, 230.0f, 160.0f, 540.0f, 360.0f, 20.0f);

		if (ganadoUltimo) {
			Estilos::textoSombra(g, L"¡MISIÓN CUMPLIDA!", Estilos::grande, Color::FromArgb(120, 240, 140), 500.0f, 212.0f);
			Estrella::dibujarForma(g, 500.0f, 282.0f, 40.0f, tickEstado * 1.5f, Color::FromArgb(255, 210, 70));
			Estrella::dibujarForma(g, 500.0f, 282.0f, 22.0f, -tickEstado * 1.5f, Color::FromArgb(255, 240, 160));
			if (nivel >= 8) {
				Estilos::textoCentrado(g, L"¡Has liberado toda la galaxia!", Estilos::normalNegrita,
					Color::FromArgb(255, 215, 90), 500.0f, 338.0f);
			}
		}
		else {
			Estilos::textoSombra(g, L"MISIÓN FALLIDA", Estilos::grande, Color::FromArgb(250, 110, 110), 500.0f, 212.0f);
			// una X grande de derrota
			Pen cruz(Color::FromArgb(220, 240, 90, 90), 10.0f);
			g->DrawLine(%cruz, 470.0f, 252.0f, 530.0f, 312.0f);
			g->DrawLine(%cruz, 530.0f, 252.0f, 470.0f, 312.0f);
		}

		Estilos::textoCentrado(g, String::Format(L"Puntos de la misión: {0}", minijuego->getPuntajeLocal()),
			Estilos::normalNegrita, Color::White, 500.0f, 372.0f);
		if (bonusDado) {
			Estilos::textoCentrado(g, L"+100 por completarla por primera vez", Estilos::normal,
				Color::FromArgb(255, 220, 120), 500.0f, 400.0f);
		}
		Estilos::textoCentrado(g, String::Format(L"Puntaje total: {0}", jugador->getPuntaje()),
			Estilos::normal, Color::FromArgb(170, 200, 240), 500.0f, 428.0f);

		int alfa = 150 + (int)(100.0 * Math::Sin(tickEstado * 0.1));
		if (alfa < 0) alfa = 0;
		if (alfa > 255) alfa = 255;
		String^ indicacion = ganadoUltimo
			? L"ENTER → despegar y volver al mapa"
			: L"R → reintentar      ENTER → volver al mapa";
		Estilos::textoCentrado(g, indicacion, Estilos::normalNegrita, Color::FromArgb(alfa, 140, 250, 150), 500.0f, 478.0f);
	}

	void Juego::dibujarFinal(Graphics^ g) {
		Fondo::dibujar(g, tick);
		Particulas::dibujar(g, fiesta);

		Estilos::textoSombra(g, L"¡GALAXIA COMPLETADA!", Estilos::titulo, Color::FromArgb(255, 215, 90), 500.0f, 150.0f);
		Estilos::textoCentrado(g, L"Las 8 misiones han sido superadas", Estilos::subtitulo,
			Color::FromArgb(150, 210, 255), 500.0f, 220.0f);
		Estilos::textoCentrado(g, String::Format(L"PUNTAJE FINAL: {0}", jugador->getPuntaje()),
			Estilos::grande, Color::White, 500.0f, 290.0f);
		Estilos::textoCentrado(g, String::Format(L"Comandante {0}, la galaxia te lo agradece.", jugador->getNombre()),
			Estilos::normal, Color::FromArgb(170, 200, 240), 500.0f, 345.0f);

		// la tripulacion celebra marchando
		for (int i = 0; i < 4; i++) {
			float px = 330.0f + i * 115.0f;
			Astronauta::dibujarModelo(g, px, 560.0f, 1.15f, MenuJuego::colorPersonaje(i), i,
				tick * 0.15f + i * 1.2f, false, false);
			Estilos::textoCentrado(g, MenuJuego::nombres[i], Estilos::pequena,
				Color::FromArgb(180, 205, 240), px, 585.0f);
		}

		int alfa = 150 + (int)(100.0 * Math::Sin(tickEstado * 0.08));
		if (alfa < 0) alfa = 0;
		if (alfa > 255) alfa = 255;
		Estilos::textoCentrado(g, L"ENTER → volver al menú", Estilos::normalNegrita,
			Color::FromArgb(alfa, 140, 250, 150), 500.0f, 635.0f);
		Estilos::textoCentrado(g, L"Hecho por Bryan, Lenin, Gustavo y Mustafa — 100% figuras geométricas",
			Estilos::pequena, Color::FromArgb(130, 150, 200), 500.0f, 668.0f);
	}
}
