#include "MinijuegoMarte.h"
#include "Superficie.h"
#include "Config.h"
#include "Azar.h"

namespace JuegoEspacial {

	MinijuegoMarte::MinijuegoMarte(Astronauta^ pjugador)
		: Minijuego(pjugador, 3, L"Caza de artefactos",
			L"Bajo la arena de Marte hay artefactos de una civilización antigua.\n"
			L"Vuela con el jetpack usando W A S D o las flechas.\n"
			L"Recoge 6 artefactos y esquiva las rocas rodantes.") {
		rocas = gcnew List<Asteroide^>();
		naveEstacionada = gcnew Nave(80.0f, Config::SUELO);
		actual = nullptr;
		recogidos = 0;
	}

	void MinijuegoMarte::iniciar() {
		jugador->setEscala(0.9f);
		jugador->setJetpack(true);
		jugador->setVidas(3);
		jugador->setPosicion(500.0f, 380.0f);

		rocas->Clear();
		for (int i = 0; i < 4; i++) {
			rocas->Add(gcnew Asteroide(
				Azar::entreF(200.0f, 800.0f), Azar::entreF(180.0f, 480.0f),
				Azar::entreF(1.6f, 2.9f) * (Azar::moneda() ? 1.0f : -1.0f),
				Azar::entreF(1.4f, 2.6f) * (Azar::moneda() ? 1.0f : -1.0f),
				Azar::entreF(16.0f, 26.0f), Color::FromArgb(150, 80, 50)));
		}
		nuevoArtefacto();
	}

	void MinijuegoMarte::nuevoArtefacto() {
		// busca un lugar alejado del jugador
		float ax = 500.0f, ay = 300.0f;
		for (int intento = 0; intento < 12; intento++) {
			ax = Azar::entreF(80.0f, 920.0f);
			ay = Azar::entreF(170.0f, 520.0f);
			float dx = ax - jugador->getX();
			float dy = ay - jugador->getY();
			if (Math::Sqrt(dx * dx + dy * dy) > 220.0) break;
		}
		actual = gcnew Artefacto(ax, ay, 50);
	}

	String^ MinijuegoMarte::progreso() {
		return String::Format(L"Artefactos: {0}/6", recogidos);
	}

	void MinijuegoMarte::actualizar(Entrada^ entrada) {
		tictac();
		naveEstacionada->mover();
		if (!jugando()) return;

		// tormenta de arena decorativa
		if (ticks % 4 == 0) {
			particulas->Add(gcnew Particula(-10.0f, Azar::entreF(120.0f, 540.0f),
				Azar::entreF(5.0f, 9.0f), Azar::entreF(-0.4f, 0.4f),
				2.5f, 50, Color::FromArgb(210, 150, 90), false));
		}

		// vuelo con jetpack
		float dx = 0.0f, dy = 0.0f;
		if (entrada->izquierda) dx -= 5.0f;
		if (entrada->derecha) dx += 5.0f;
		if (entrada->arriba) dy -= 5.0f;
		if (entrada->abajo) dy += 5.0f;
		jugador->volar(dx, dy, 40.0f, 150.0f, 960.0f, Config::SUELO);
		jugador->mover();

		// rocas rodantes que rebotan
		for each (Asteroide^ roca in rocas) {
			roca->mover();
			roca->rebotar(30.0f, 110.0f, 970.0f, Config::SUELO);
			if (!jugador->esInvulnerable() && roca->zona().IntersectsWith(jugador->zona())) {
				roca->atacar();
				jugador->perderVida();
				Particulas::explosion(particulas, jugador->getX(), jugador->getY() - 40.0f, Color::FromArgb(255, 120, 80), 16);
				if (jugador->getVidas() <= 0) perder();
			}
		}

		// artefacto actual
		if (actual != nullptr) {
			actual->mover();
			if (actual->zona().IntersectsWith(jugador->zona())) {
				jugador->recolectarItem(actual);
				puntajeLocal += actual->getValor();
				recogidos++;
				Particulas::explosion(particulas, actual->getX(), actual->getY(), Color::FromArgb(90, 230, 255), 20);
				if (recogidos >= 6) {
					actual = nullptr;
					ganar();
				}
				else {
					nuevoArtefacto();
				}
			}
		}
	}

	void MinijuegoMarte::dibujar(Graphics^ g) {
		Superficie::dibujarFondo(g, 3, ticks);
		naveEstacionada->dibujar(g);

		if (actual != nullptr) {
			actual->dibujar(g);
			// flecha que apunta al artefacto si esta lejos
			float dx = actual->getX() - jugador->getX();
			float dy = actual->getY() - (jugador->getY() - 45.0f);
			float distancia = (float)Math::Sqrt(dx * dx + dy * dy);
			if (distancia > 320.0f) {
				float nx = dx / distancia, ny = dy / distancia;
				float fx = jugador->getX() + nx * 70.0f;
				float fy = jugador->getY() - 45.0f + ny * 70.0f;
				array<PointF>^ flecha = gcnew array<PointF>(3);
				flecha[0] = PointF(fx + nx * 14.0f, fy + ny * 14.0f);
				flecha[1] = PointF(fx - ny * 7.0f, fy + nx * 7.0f);
				flecha[2] = PointF(fx + ny * 7.0f, fy - nx * 7.0f);
				SolidBrush punta(Color::FromArgb(170, 90, 230, 255));
				g->FillPolygon(%punta, flecha);
			}
		}

		for each (Asteroide^ roca in rocas) {
			roca->dibujar(g);
		}

		jugador->dibujar(g);
		Particulas::dibujar(g, particulas);
	}
}
