#include "MinijuegoSaturno.h"
#include "SistemaSolar.h"
#include "Fondo.h"
#include "Config.h"
#include "Azar.h"
#include "Sonido.h"

namespace JuegoEspacial {

	MinijuegoSaturno::MinijuegoSaturno(Astronauta^ pjugador)
		: Minijuego(pjugador, 5, L"Carrera de anillos",
			L"¡Hora de pilotar la nave entre los anillos de Saturno!\n"
			L"Sube y baja con ↑ / ↓ (o W / S) y pasa por los huecos.\n"
			L"Chocar daña el casco; recoge estrellas para repararlo. Cruza 12 anillos.") {
		nave = gcnew Nave(150.0f, 350.0f);
		anillos = gcnew List<AnilloCarrera^>();
		estrellas = gcnew List<Estrella^>();
		pasados = 0;
		enfriamientoEstrella = 140;
		usaVidas = false;        // aqui cuenta la integridad de la nave
	}

	void MinijuegoSaturno::iniciar() {
		nave->setPatas(false);
		nave->setLlama(true);
		nave->setVelocidad(180.0f);
		anillos->Clear();
		anillos->Add(gcnew AnilloCarrera(1080.0f, 350.0f, 210.0f));
	}

	String^ MinijuegoSaturno::progreso() {
		return String::Format(L"Anillos: {0}/12", pasados);
	}

	void MinijuegoSaturno::dibujarHUD(Graphics^ g) {
		Minijuego::dibujarHUD(g);
		nave->mostrarEstado(g);
	}

	void MinijuegoSaturno::actualizar(Entrada^ entrada) {
		tictac();
		if (!jugando()) return;

		float velocidadScroll = 4.6f + pasados * 0.12f;
		nave->setVelocidad(180.0f + pasados * 6.0f);

		// control vertical de la nave
		float dy = 0.0f;
		if (entrada->arriba) dy -= 5.4f;
		if (entrada->abajo) dy += 5.4f;
		float nuevaY = nave->getY() + dy;
		if (nuevaY < 95.0f) nuevaY = 95.0f;
		if (nuevaY > 655.0f) nuevaY = 655.0f;
		nave->setPosicion(150.0f, nuevaY);
		nave->setInclinacion(dy * 3.2f);
		nave->mover();

		// estela del motor
		if (ticks % 2 == 0) {
			particulas->Add(gcnew Particula(nave->getX() + 4.0f, nave->getY() + Azar::entreF(-4.0f, 4.0f),
				Azar::entreF(-4.5f, -3.0f), Azar::entreF(-0.5f, 0.5f),
				3.5f, 18, Color::FromArgb(255, 190, 80), false));
		}

		// nuevos anillos
		if (anillos->Count == 0 || anillos[anillos->Count - 1]->x < 1000.0f - 280.0f) {
			float alto = Math::Max(150.0f, 215.0f - pasados * 5.0f);
			anillos->Add(gcnew AnilloCarrera(1080.0f, Azar::entreF(150.0f, 560.0f), alto));
		}

		// nuevas estrellas reparadoras
		enfriamientoEstrella--;
		if (enfriamientoEstrella <= 0) {
			estrellas->Add(gcnew Estrella(1060.0f, Azar::entreF(130.0f, 600.0f), 0.0f, 12.0f, 10));
			enfriamientoEstrella = Azar::entre(130, 210);
		}

		// mover anillos y detectar cruces / choques
		for (int i = anillos->Count - 1; i >= 0; i--) {
			AnilloCarrera^ anillo = anillos[i];
			anillo->x -= velocidadScroll;

			RectangleF zonaNave = nave->zona();
			float topeSuperior = anillo->brechaY - anillo->brechaAlto / 2.0f;
			float topeInferior = anillo->brechaY + anillo->brechaAlto / 2.0f;
			RectangleF barraSuperior(anillo->x - 16.0f, 0.0f, 32.0f, topeSuperior);
			RectangleF barraInferior(anillo->x - 16.0f, topeInferior, 32.0f, Config::ALTO - topeInferior);

			if (!anillo->golpeado && (zonaNave.IntersectsWith(barraSuperior) || zonaNave.IntersectsWith(barraInferior))) {
				anillo->golpeado = true;
				nave->recibirDanio(20);
				Particulas::explosion(particulas, nave->getX() + 60.0f, nave->getY(), Color::FromArgb(255, 160, 70), 20);
				if (nave->destruida()) perder();
			}

			if (!anillo->pasado && !anillo->golpeado && anillo->x < nave->getX()) {
				anillo->pasado = true;
				pasados++;
				sumar(20);
				Sonido::anillo();
				if (pasados >= 12) ganar();
			}

			if (anillo->x < -60.0f) anillos->RemoveAt(i);
		}

		// estrellas reparadoras
		for (int i = estrellas->Count - 1; i >= 0; i--) {
			Estrella^ estrella = estrellas[i];
			estrella->setPosicion(estrella->getX() - velocidadScroll, estrella->getY());
			estrella->mover();
			if (estrella->zona().IntersectsWith(nave->zona())) {
				jugador->recolectarItem(estrella);
				puntajeLocal += estrella->getValor();
				nave->reparar(15);
				Particulas::explosion(particulas, estrella->getX(), estrella->getY(), Color::FromArgb(255, 220, 100), 14);
				estrellas->RemoveAt(i);
				continue;
			}
			if (estrella->getX() < -40.0f) estrellas->RemoveAt(i);
		}
	}

	void MinijuegoSaturno::dibujar(Graphics^ g) {
		Fondo::dibujar(g, ticks);

		// Saturno gigante al fondo
		SistemaSolar::dibujarPlaneta(g, 5, 820.0f, 190.0f, 105.0f, ticks);

		// polvo de los anillos pasando a toda velocidad
		Pen polvo(Color::FromArgb(70, 220, 205, 160), 1.5f);
		for (int i = 0; i < 12; i++) {
			float px = 1000.0f - (float)((ticks * (8 + i % 4) + i * 173) % 1100);
			float py = 80.0f + i * 50.0f;
			g->DrawLine(%polvo, px, py, px + 30.0f + (i % 4) * 8.0f, py);
		}

		// anillos de carrera
		for each (AnilloCarrera^ anillo in anillos) {
			float topeSuperior = anillo->brechaY - anillo->brechaAlto / 2.0f;
			float topeInferior = anillo->brechaY + anillo->brechaAlto / 2.0f;

			Color base = anillo->golpeado ? Color::FromArgb(150, 110, 80) : Color::FromArgb(208, 178, 112);
			SolidBrush barra(base);
			g->FillRectangle(%barra, anillo->x - 16.0f, 0.0f, 32.0f, topeSuperior);
			g->FillRectangle(%barra, anillo->x - 16.0f, topeInferior, 32.0f, Config::ALTO - topeInferior);

			// motas de roca dentro de las barras
			SolidBrush mota(Color::FromArgb(120, 120, 95, 55));
			for (int m = 0; m < 5; m++) {
				float my1 = topeSuperior * (m + 1) / 6.0f;
				g->FillEllipse(%mota, anillo->x - 9.0f + (m % 3) * 6.0f, my1, 7.0f, 7.0f);
				float my2 = topeInferior + (Config::ALTO - topeInferior) * (m + 1) / 6.0f;
				g->FillEllipse(%mota, anillo->x - 10.0f + ((m + 1) % 3) * 7.0f, my2, 7.0f, 7.0f);
			}

			// bordes del hueco resaltados
			Color guia = anillo->pasado ? Color::FromArgb(120, 230, 130) : Color::FromArgb(110, 230, 255);
			Pen filo(Color::FromArgb(220, guia), 3.0f);
			g->DrawLine(%filo, anillo->x - 16.0f, topeSuperior, anillo->x + 16.0f, topeSuperior);
			g->DrawLine(%filo, anillo->x - 16.0f, topeInferior, anillo->x + 16.0f, topeInferior);
		}

		for each (Estrella^ estrella in estrellas) {
			estrella->dibujar(g);
		}

		Particulas::dibujar(g, particulas);
		nave->dibujarHorizontal(g);

		// el casco del piloto se asoma por la ventana
		SolidBrush piloto(jugador->getColorTraje());
		g->FillEllipse(%piloto, nave->getX() + 61.0f, nave->getY() - 6.0f, 13.0f, 13.0f);
	}
}
