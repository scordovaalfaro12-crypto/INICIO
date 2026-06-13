#include "Juego.h"
#include "Planetas.h"
#include "Cometa.h"
#include "Asteroide.h"
#include "FiguraKhaos.h"
#include "Config.h"
#include "Estilos.h"
#include "Fondo.h"
#include "Recursos.h"
#include "Azar.h"
#include "Sonido.h"

namespace PoliNavis {

	// velocidad de la nave (los obstaculos siempre van mas rapido)
	static const float VEL = 4.3f;
	static const float SOLX = 500.0f, SOLY = 360.0f;

	Juego::Juego() {
		Recursos::cargar();
		estado = L"MENU";
		tick = tickEstado = 0;
		menu = gcnew Menu();
		entrada = gcnew Entrada();
		planetas = gcnew List<Planeta^>();
		obstaculos = gcnew List<Obstaculo^>();
		nivel = 1;
		colisiones = 0;
		ticksNivel = 0;
		tiempoNivel1 = 0;
		spawnTimer = 0;
		datoLeyenda = L"Haz clic en un planeta para acelerar su rotacion y traslacion.";
		nave = gcnew Nave(Config::AX, Config::AY);
		sol = gcnew Sol(SOLX, SOLY, 46.0f);
		meta = gcnew Estacion(Config::BX, Config::BY, 0);
	}

	String^ Juego::getEstado() { return estado; }

	void Juego::cambiar(String^ nuevo) {
		estado = nuevo;
		tickEstado = 0;
	}

	float Juego::inicioX() { return (nivel == 1) ? (float)Config::AX : 70.0f; }
	float Juego::inicioY() { return (nivel == 1) ? (float)Config::AY : 350.0f; }

	// ---------------- preparar un nivel ----------------
	void Juego::iniciarNivel(int n) {
		nivel = n;
		colisiones = 0;
		ticksNivel = 0;
		spawnTimer = 0;
		obstaculos->Clear();
		planetas->Clear();

		if (n == 1) {
			nave = gcnew Nave(Config::AX, Config::AY);
			nave->setColorTrazo(Color::FromArgb(120, 230, 255));
			meta = gcnew Estacion(Config::BX, Config::BY, 0);
			// 8 planetas: (radioOrbita, anguloInicial). Tierra y Saturno incluidos.
			planetas->Add(gcnew Mercurio(SOLX, SOLY, 92.0f, 0.0f));
			planetas->Add(gcnew Venus(SOLX, SOLY, 132.0f, 60.0f));
			planetas->Add(gcnew Tierra(SOLX, SOLY, 178.0f, 130.0f));
			planetas->Add(gcnew Marte(SOLX, SOLY, 218.0f, 220.0f));
			planetas->Add(gcnew Jupiter(SOLX, SOLY, 290.0f, 300.0f));
			planetas->Add(gcnew Saturno(SOLX, SOLY, 360.0f, 30.0f));
			planetas->Add(gcnew Urano(SOLX, SOLY, 410.0f, 160.0f));
			planetas->Add(gcnew Neptuno(SOLX, SOLY, 452.0f, 250.0f));
			datoLeyenda = L"Haz clic en un planeta para acelerar su rotacion y traslacion.";
			cambiar(L"NIVEL1");
		}
		else {
			nave = gcnew Nave(70.0f, 350.0f);
			nave->setColorTrazo(Color::FromArgb(255, 90, 220));
			meta = gcnew Estacion(930.0f, 350.0f, 1);
			// campo de figuras geometricas caoticas
			for (int i = 0; i < 11; i++) {
				float px = Azar::entreF(180.0f, 880.0f);
				float py = Azar::entreF(60.0f, 640.0f);
				float ang = Azar::entreF(0.0f, 6.2832f);
				float vel = Azar::entreF(2.5f, 6.0f);
				int lados = Azar::entre(3, 9);   // 3..8 lados (incluye rectangulos)
				float radio = Azar::entreF(16.0f, 40.0f);
				obstaculos->Add(gcnew FiguraKhaos(px, py,
					(float)Math::Cos(ang) * vel, (float)Math::Sin(ang) * vel, radio, lados));
			}
			datoLeyenda = L"Galaxia del Khaos: figuras caoticas y cometas neon veloces. Llega a la salida.";
			cambiar(L"NIVEL2");
		}
		Sonido::seleccionar();
	}

	// ---------------- bucle de logica ----------------
	void Juego::actualizar() {
		tick++; tickEstado++;
		if (estado == L"NIVEL1" || estado == L"NIVEL2") actualizarNivel();
		else if (estado == L"COMPLETO1") {
			// tras unos segundos pasa solo al Nivel 2
			if (tickEstado > 240) iniciarNivel(2);
		}
	}

	void Juego::actualizarNivel() {
		ticksNivel++;
		meta->mover();
		aplicarInput();
		nave->mover();
		if (nave->getEnergia() < 100 && ticksNivel % 25 == 0) nave->recargarEnergia(1);

		if (nivel == 1) {
			sol->mover();
			for each (Planeta^ p in planetas) p->mover();
		}

		// aparicion de cometas/asteroides a intervalos en AMBOS niveles
		spawnTimer--;
		if (spawnTimer <= 0) {
			if (nivel == 1) {
				spawnObstaculoNivel1();
				spawnTimer = Math::Max(35, 90 - ticksNivel / 120);  // cada vez mas seguido
			}
			else {
				spawnObstaculoNivel2();
				spawnTimer = Math::Max(26, 60 - ticksNivel / 120);  // mas seguido y veloz que Nivel 1
			}
		}

		for each (Obstaculo^ o in obstaculos) o->mover();
		// quitar los que salieron de la pantalla (las FiguraKhaos rebotan, no salen)
		for (int i = obstaculos->Count - 1; i >= 0; i--)
			if (obstaculos[i]->fueraDePantalla()) obstaculos->RemoveAt(i);

		revisarColisiones();
	}

	void Juego::aplicarInput() {
		float vx = 0.0f, vy = 0.0f;
		if (entrada->izquierda) vx -= VEL;
		if (entrada->derecha) vx += VEL;
		if (entrada->arriba) vy -= VEL;
		if (entrada->abajo) vy += VEL;
		nave->acelerar(vx, vy);
	}

	void Juego::spawnObstaculoNivel1() {
		// nace en un borde al azar y cruza hacia el lado opuesto
		int borde = Azar::entre(0, 4);
		float px, py, vx, vy;
		float vel = Azar::entreF(4.8f, 7.5f);    // mas rapido que la nave (VEL=4.3)
		if (borde == 0) { px = -20.0f; py = Azar::entreF(40.0f, 660.0f); vx = vel; vy = Azar::entreF(-2.0f, 2.0f); }
		else if (borde == 1) { px = Config::ANCHO + 20.0f; py = Azar::entreF(40.0f, 660.0f); vx = -vel; vy = Azar::entreF(-2.0f, 2.0f); }
		else if (borde == 2) { px = Azar::entreF(40.0f, 960.0f); py = -20.0f; vy = vel; vx = Azar::entreF(-2.0f, 2.0f); }
		else { px = Azar::entreF(40.0f, 960.0f); py = Config::ALTO + 20.0f; vy = -vel; vx = Azar::entreF(-2.0f, 2.0f); }

		if (Azar::moneda())
			obstaculos->Add(gcnew Cometa(px, py, vx, vy, Azar::entreF(7.0f, 11.0f), false));
		else
			obstaculos->Add(gcnew Asteroide(px, py, vx, vy, Azar::entreF(13.0f, 22.0f)));
	}

	// Cometas/asteroides del Nivel 2: mas rapidos e impredecibles que los
	// del Nivel 1, con cometas de colores neon (rubrica: forma propia del nivel).
	void Juego::spawnObstaculoNivel2() {
		int borde = Azar::entre(0, 4);
		float px, py, vx, vy;
		float vel = Azar::entreF(6.5f, 9.5f);    // mas veloz que el Nivel 1
		if (borde == 0) { px = -20.0f; py = Azar::entreF(40.0f, 660.0f); vx = vel; vy = Azar::entreF(-3.5f, 3.5f); }
		else if (borde == 1) { px = Config::ANCHO + 20.0f; py = Azar::entreF(40.0f, 660.0f); vx = -vel; vy = Azar::entreF(-3.5f, 3.5f); }
		else if (borde == 2) { px = Azar::entreF(40.0f, 960.0f); py = -20.0f; vy = vel; vx = Azar::entreF(-3.5f, 3.5f); }
		else { px = Azar::entreF(40.0f, 960.0f); py = Config::ALTO + 20.0f; vy = -vel; vx = Azar::entreF(-3.5f, 3.5f); }

		if (Azar::entre(0, 3) != 0)
			obstaculos->Add(gcnew Cometa(px, py, vx, vy, Azar::entreF(8.0f, 12.0f), true));   // neon
		else
			obstaculos->Add(gcnew Asteroide(px, py, vx, vy, Azar::entreF(12.0f, 20.0f)));
	}

	void Juego::revisarColisiones() {
		// llegada a la meta (Punto B / portal)
		if (nave->area().IntersectsWith(meta->area())) {
			if (nivel == 1) {
				tiempoNivel1 = ticksNivel / 60;
				Sonido::victoria();
				cambiar(L"COMPLETO1");
			}
			else {
				Sonido::victoria();
				cambiar(L"COMPLETO2");
			}
			return;
		}

		if (nave->esInvulnerable()) return;

		bool choco = false;
		// contra obstaculos
		for each (Obstaculo^ o in obstaculos) {
			if (nave->area().IntersectsWith(o->area())) { choco = true; break; }
		}
		// contra planetas (Nivel 1): tambien hay que evitarlos
		if (!choco && nivel == 1) {
			for each (Planeta^ p in planetas) {
				if (nave->area().IntersectsWith(p->area())) { choco = true; break; }
			}
		}

		if (choco) {
			colisiones++;
			nave->perderVida();
			nave->gastarEnergia(12);
			nave->reiniciarEn(inicioX(), inicioY());
			if (nave->sinVidas()) {
				Sonido::derrota();
				cambiar(L"DERROTA");
			}
		}
	}

	// ---------------- entrada ----------------
	void Juego::teclaAbajo(Keys t) {
		if (t == Keys::Left || t == Keys::A) entrada->izquierda = true;
		if (t == Keys::Right || t == Keys::D) entrada->derecha = true;
		if (t == Keys::Up || t == Keys::W) entrada->arriba = true;
		if (t == Keys::Down || t == Keys::S) entrada->abajo = true;

		if (estado == L"MENU") {
			if (t == Keys::Up || t == Keys::W) { menu->moverSeleccion(-1); Sonido::mover(); }
			else if (t == Keys::Down || t == Keys::S) { menu->moverSeleccion(1); Sonido::mover(); }
			else if (t == Keys::Return || t == Keys::Space) {
				AccionMenu a = menu->seleccionActual();
				if (a == AccionMenu::Nivel1) iniciarNivel(1);
				else if (a == AccionMenu::Nivel2) iniciarNivel(2);
				else if (a == AccionMenu::Instrucciones) cambiar(L"INSTRUCCIONES");
				else if (a == AccionMenu::Creadores) cambiar(L"CREADORES");
				else if (a == AccionMenu::Salir) Application::Exit();
			}
			else if (t == Keys::Escape) Application::Exit();
		}
		else if (estado == L"INSTRUCCIONES" || estado == L"CREADORES") {
			if (t == Keys::Escape || t == Keys::Return) cambiar(L"MENU");
		}
		else if (estado == L"NIVEL1" || estado == L"NIVEL2") {
			if (t == Keys::Escape) cambiar(L"MENU");
		}
		else if (estado == L"COMPLETO1") {
			if (t == Keys::Return || t == Keys::Space) iniciarNivel(2);
			else if (t == Keys::Escape) cambiar(L"MENU");
		}
		else if (estado == L"COMPLETO2" || estado == L"DERROTA") {
			if (t == Keys::Return || t == Keys::Space) cambiar(L"MENU");
			else if (t == Keys::R) iniciarNivel(nivel);
		}
	}

	void Juego::teclaArriba(Keys t) {
		if (t == Keys::Left || t == Keys::A) entrada->izquierda = false;
		if (t == Keys::Right || t == Keys::D) entrada->derecha = false;
		if (t == Keys::Up || t == Keys::W) entrada->arriba = false;
		if (t == Keys::Down || t == Keys::S) entrada->abajo = false;
	}

	void Juego::ratonMovido(int mx, int my) {
		if (estado == L"MENU") { if (menu->hoverPrincipal(mx, my)) Sonido::mover(); }
	}

	void Juego::clic(int mx, int my) {
		if (estado == L"MENU") {
			AccionMenu a = menu->clicPrincipal(mx, my);
			if (a == AccionMenu::Nivel1) iniciarNivel(1);
			else if (a == AccionMenu::Nivel2) iniciarNivel(2);
			else if (a == AccionMenu::Instrucciones) cambiar(L"INSTRUCCIONES");
			else if (a == AccionMenu::Creadores) cambiar(L"CREADORES");
			else if (a == AccionMenu::Salir) Application::Exit();
		}
		else if (estado == L"INSTRUCCIONES" || estado == L"CREADORES") {
			if (menu->clicVolver(mx, my) == AccionMenu::Volver) cambiar(L"MENU");
		}
		else if (estado == L"NIVEL1") {
			// clic sobre un planeta => acelera su rotacion/traslacion (evento)
			for each (Planeta^ p in planetas) {
				float dx = mx - p->getX(), dy = my - p->getY();
				if (Math::Sqrt(dx * dx + dy * dy) <= p->getRadio() + 8.0f) {
					p->acelerar();
					datoLeyenda = p->getNombre() + ": " + p->getDato();
					Sonido::item();
					return;
				}
			}
		}
	}

	// ---------------- dibujo ----------------
	void Juego::dibujar(Graphics^ g) {
		if (estado == L"MENU") { menu->dibujarPrincipal(g, tick); return; }
		if (estado == L"INSTRUCCIONES") { menu->dibujarInstrucciones(g, tick); return; }
		if (estado == L"CREADORES") { menu->dibujarCreadores(g, tick); return; }

		// fondo + escena del nivel
		dibujarFondoNivel(g);
		if (nivel == 2) dibujarAgujeroNegro(g);

		if (nivel == 1) {
			for each (Planeta^ p in planetas) p->dibujarOrbita(g);
			sol->dibujar(g);
			for each (Planeta^ p in planetas) p->dibujar(g);
		}

		meta->dibujar(g);
		dibujarPuntos(g);
		for each (Obstaculo^ o in obstaculos) o->dibujar(g);
		nave->dibujar(g);

		dibujarPanel(g);
		dibujarLeyenda(g);

		// mensajes de exito / derrota
		if (estado == L"COMPLETO1")
			dibujarMensaje(g, L"NIVEL 1 COMPLETO", L"Recargando combustible...",
				String::Format(L"Tiempo: {0:00}:{1:00}   -   ENTER para el Nivel 2", tiempoNivel1 / 60, tiempoNivel1 % 60),
				Color::FromArgb(120, 240, 140));
		else if (estado == L"COMPLETO2")
			dibujarMensaje(g, L"MISION CUMPLIDA", L"Has escapado de la Galaxia Nebulosa del Khaos!",
				String::Format(L"Tiempo total Nivel 2: {0:00}:{1:00}   -   ENTER: menu", ticksNivel / 3600, (ticksNivel / 60) % 60),
				Color::FromArgb(120, 240, 140));
		else if (estado == L"DERROTA")
			dibujarMensaje(g, L"MISION FALLIDA", L"La nave se quedo sin vidas",
				L"ENTER: menu     R: reintentar nivel", Color::FromArgb(250, 110, 110));
	}

	void Juego::dibujarFondoNivel(Graphics^ g) {
		Image^ img = (nivel == 1) ? Recursos::fondoEspacio : Recursos::fondoNebulosa;
		if (img != nullptr) {
			g->DrawImage(img, 0, 0, Config::ANCHO, Config::ALTO);
			SolidBrush velo(Color::FromArgb(nivel == 1 ? 90 : 120, nivel == 1 ? 6 : 30, 6, nivel == 1 ? 16 : 40));
			g->FillRectangle(%velo, 0, 0, Config::ANCHO, Config::ALTO);
		}
		else {
			Fondo::dibujar(g, tick);
		}
	}

	void Juego::dibujarAgujeroNegro(Graphics^ g) {
		// decoracion del Nivel 2: un agujero negro girando al centro-fondo
		float cx = 500.0f, cy = 360.0f;
		for (int k = 6; k >= 1; k--) {
			int a = 12 + k * 6;
			SolidBrush halo(Color::FromArgb(a, 150, 80, 220));
			float r = 40.0f + k * 18.0f;
			g->FillEllipse(%halo, cx - r, cy - r, r * 2.0f, r * 2.0f);
		}
		Pen disco(Color::FromArgb(120, 200, 130, 255), 3.0f);
		for (int i = 0; i < 3; i++) {
			float r = 70.0f + i * 16.0f;
			g->DrawArc(%disco, cx - r, cy - r * 0.4f, r * 2.0f, r * 0.8f,
				(float)(tick * (2 + i)) , 250.0f);
		}
		SolidBrush nucleo(Color::FromArgb(255, 10, 8, 20));
		g->FillEllipse(%nucleo, cx - 40.0f, cy - 40.0f, 80.0f, 80.0f);
	}

	void Juego::dibujarPuntos(Graphics^ g) {
		// marca del Punto A (origen) y etiqueta del Punto B
		float ax = inicioX(), ay = inicioY();
		Pen aro(Color::FromArgb(150, 110, 230, 255), 2.0f);
		g->DrawEllipse(%aro, ax - 20.0f, ay - 20.0f, 40.0f, 40.0f);
		Estilos::textoCentrado(g, (nivel == 1 ? L"A" : L"1"), Estilos::normalNegrita, Color::FromArgb(180, 220, 255), ax, ay - 32.0f);
		float etiqY = Math::Max(62.0f, meta->getY() - 52.0f);
		Estilos::textoCentrado(g, (nivel == 1 ? L"ESTACION (B)" : L"SALIDA (2)"), Estilos::pequena,
			Color::FromArgb(200, 220, 255), meta->getX(), etiqY);
	}

	void Juego::dibujarPanel(Graphics^ g) {
		// barra superior con el panel de control
		SolidBrush barra(Color::FromArgb(175, 6, 10, 24));
		g->FillRectangle(%barra, 0.0f, 0.0f, (float)Config::ANCHO, 44.0f);
		Pen linea(Color::FromArgb(120, 90, 200, 255), 1.5f);
		g->DrawLine(%linea, 0.0f, 44.0f, (float)Config::ANCHO, 44.0f);

		int seg = ticksNivel / 60;
		Estilos::textoIzquierda(g, String::Format(L"TIEMPO  {0:00}:{1:00}", seg / 60, seg % 60),
			Estilos::hud, Color::FromArgb(150, 220, 255), 16.0f, 22.0f);
		Estilos::textoIzquierda(g, String::Format(L"COLISIONES  {0}", colisiones),
			Estilos::hud, Color::FromArgb(255, 180, 120), 200.0f, 22.0f);

		// energia
		Estilos::textoIzquierda(g, L"ENERGIA", Estilos::hud, Color::FromArgb(150, 220, 255), 380.0f, 22.0f);
		float bx = 470.0f, bw = 120.0f;
		SolidBrush fondoB(Color::FromArgb(50, 60, 80));
		g->FillRectangle(%fondoB, bx, 15.0f, bw, 14.0f);
		Color cE = nave->getEnergia() > 50 ? Color::FromArgb(90, 220, 110)
			: (nave->getEnergia() > 25 ? Color::FromArgb(240, 210, 80) : Color::FromArgb(235, 80, 70));
		SolidBrush llenoB(cE);
		g->FillRectangle(%llenoB, bx, 15.0f, bw * nave->getEnergia() / 100.0f, 14.0f);
		Pen marco(Color::FromArgb(200, 205, 220), 1.0f);
		g->DrawRectangle(%marco, bx, 15.0f, bw, 14.0f);
		Estilos::textoIzquierda(g, String::Format(L"{0}%", nave->getEnergia()), Estilos::hud, Color::White, bx + bw + 8.0f, 22.0f);

		// vidas (corazones)
		Estilos::textoIzquierda(g, L"VIDAS", Estilos::hud, Color::FromArgb(150, 220, 255), 700.0f, 22.0f);
		for (int i = 0; i < Config::VIDAS_INICIALES; i++) {
			float cx = 770.0f + i * 30.0f, cy = 22.0f;
			Color c = (i < nave->getVidas()) ? Color::FromArgb(235, 70, 80) : Color::FromArgb(70, 75, 95);
			SolidBrush ch(c);
			g->FillEllipse(%ch, cx - 9.0f, cy - 7.0f, 10.0f, 10.0f);
			g->FillEllipse(%ch, cx - 1.0f, cy - 7.0f, 10.0f, 10.0f);
			array<PointF>^ punta = gcnew array<PointF>(3);
			punta[0] = PointF(cx - 8.5f, cy - 1.0f); punta[1] = PointF(cx + 8.5f, cy - 1.0f); punta[2] = PointF(cx, cy + 9.0f);
			g->FillPolygon(%ch, punta);
		}
		// objetivo
		Estilos::textoIzquierda(g, String::Format(L"META: llega a {0}", nivel == 1 ? L"la ESTACION" : L"la SALIDA"),
			Estilos::hud, Color::FromArgb(180, 255, 180), 900.0f, 22.0f);
	}

	void Juego::dibujarLeyenda(Graphics^ g) {
		if (nivel == 1) {
			// panel lateral con los nombres de los planetas (componente educativo)
			float px = 12.0f, py = 70.0f, pw = 150.0f, ph = 8.0f + planetas->Count * 22.0f + 6.0f;
			Estilos::rellenarRedondeado(g, Color::FromArgb(150, 6, 10, 26), px, py, pw, ph, 10.0f);
			Estilos::bordeRedondeado(g, Color::FromArgb(120, 90, 160, 230), 1.5f, px, py, pw, ph, 10.0f);
			Estilos::textoIzquierda(g, L"PLANETAS", Estilos::pequena, Color::FromArgb(150, 210, 255), px + 12.0f, py + 14.0f);
			for (int i = 0; i < planetas->Count; i++) {
				float fy = py + 30.0f + i * 22.0f;
				SolidBrush punto(planetas[i]->colorLeyenda);
				g->FillEllipse(%punto, px + 14.0f, fy - 6.0f, 12.0f, 12.0f);
				Estilos::textoIzquierda(g, planetas[i]->getNombre(), Estilos::pequena, Color::White, px + 34.0f, fy);
			}
		}
		// franja inferior con el dato educativo
		SolidBrush franja(Color::FromArgb(165, 6, 10, 24));
		g->FillRectangle(%franja, 0.0f, (float)Config::ALTO - 34.0f, (float)Config::ANCHO, 34.0f);
		Estilos::textoCentrado(g, datoLeyenda, Estilos::normal, Color::FromArgb(200, 220, 245),
			500.0f, (float)Config::ALTO - 17.0f);
	}

	void Juego::dibujarMensaje(Graphics^ g, String^ titulo, String^ sub, String^ pie, Color color) {
		SolidBrush velo(Color::FromArgb(150, 0, 0, 12));
		g->FillRectangle(%velo, 0.0f, 0.0f, (float)Config::ANCHO, (float)Config::ALTO);
		Estilos::rellenarRedondeado(g, Color::FromArgb(235, 10, 16, 38), 230.0f, 250.0f, 540.0f, 200.0f, 18.0f);
		Estilos::bordeRedondeado(g, color, 3.0f, 230.0f, 250.0f, 540.0f, 200.0f, 18.0f);
		Estilos::textoSombra(g, titulo, Estilos::grande, color, 500.0f, 300.0f);
		Estilos::textoCentrado(g, sub, Estilos::subtitulo, Color::White, 500.0f, 350.0f);
		int a = 150 + (int)(100.0 * Math::Sin(tickEstado * 0.1));
		if (a < 0) a = 0; if (a > 255) a = 255;
		Estilos::textoCentrado(g, pie, Estilos::normalNegrita, Color::FromArgb(a, 160, 230, 255), 500.0f, 410.0f);
	}
}
