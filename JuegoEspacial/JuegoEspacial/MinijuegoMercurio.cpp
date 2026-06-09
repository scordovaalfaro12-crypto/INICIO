#include "MinijuegoMercurio.h"
#include "Superficie.h"
#include "Config.h"
#include "Azar.h"
#include "Sonido.h"

namespace JuegoEspacial {

	MinijuegoMercurio::MinijuegoMercurio(Astronauta^ pjugador)
		: Minijuego(pjugador, 0, L"Lluvia de fuego",
			L"Llueven meteoritos ardientes muy cerca del Sol.\n"
			L"Muévete con A / D o las flechas y esquívalos.\n"
			L"¡Sobrevive 30 segundos!") {
		meteoros = gcnew List<Asteroide^>();
		naveEstacionada = gcnew Nave(80.0f, Config::SUELO);
		restante = 30 * Config::TICKS_SEGUNDO;
		enfriamientoSpawn = 45;
	}

	void MinijuegoMercurio::iniciar() {
		jugador->setEscala(0.95f);
		jugador->setJetpack(false);
		jugador->setVidas(3);
		jugador->setPosicion(500.0f, Config::SUELO);
	}

	String^ MinijuegoMercurio::progreso() {
		return String::Format(L"Sobrevive: {0} s", Math::Max(0, (restante + 59) / 60));
	}

	void MinijuegoMercurio::actualizar(Entrada^ entrada) {
		tictac();
		naveEstacionada->mover();
		if (!jugando()) return;

		restante--;
		if (restante <= 0) {
			sumar(60);
			ganar();
			return;
		}

		// cuanto mas avanza, mas rapido y mas seguido caen
		float velocidadExtra = (1800 - restante) / 600.0f;
		enfriamientoSpawn--;
		if (enfriamientoSpawn <= 0) {
			meteoros->Add(gcnew Asteroide(
				Azar::entreF(30.0f, 970.0f), -40.0f,
				Azar::entreF(-0.9f, 0.9f), Azar::entreF(3.0f, 5.2f) + velocidadExtra,
				Azar::entreF(9.0f, 20.0f), Color::FromArgb(225, 120, 50)));
			enfriamientoSpawn = Math::Max(13, 32 - (1800 - restante) / 80);
		}

		// movimiento del jugador
		if (entrada->izquierda) jugador->caminar(-6.0f, 25.0f, 975.0f);
		if (entrada->derecha) jugador->caminar(6.0f, 25.0f, 975.0f);
		jugador->mover();

		// meteoros
		for (int i = meteoros->Count - 1; i >= 0; i--) {
			Asteroide^ m = meteoros[i];
			m->mover();

			// estela de fuego
			if (ticks % 3 == 0) {
				particulas->Add(gcnew Particula(m->getX(), m->getY() - m->getRadio() * 0.6f,
					Azar::entreF(-0.5f, 0.5f), -1.6f, 3.5f, 14,
					Color::FromArgb(255, 170, 60), false));
			}

			// choca con el jugador
			if (!jugador->esInvulnerable() && m->zona().IntersectsWith(jugador->zona())) {
				m->atacar();
				jugador->perderVida();
				Particulas::explosion(particulas, m->getX(), m->getY(), Color::FromArgb(255, 120, 60), 16);
				meteoros->RemoveAt(i);
				if (jugador->getVidas() <= 0) perder();
				continue;
			}

			// impacta contra el suelo
			if (m->getY() > Config::SUELO - m->getRadio() * 0.4f) {
				Particulas::explosion(particulas, m->getX(), Config::SUELO - 6.0f, Color::FromArgb(255, 140, 60), 10);
				if (Azar::entre(0, 4) == 0) Sonido::explosion();
				meteoros->RemoveAt(i);
				sumar(2);     // meteorito esquivado
			}
		}
	}

	void MinijuegoMercurio::dibujar(Graphics^ g) {
		Superficie::dibujarFondo(g, 0, ticks);
		naveEstacionada->dibujar(g);

		// sombras de los meteoros sobre el suelo (ayudan a esquivar)
		for each (Asteroide^ m in meteoros) {
			float r = m->getRadio();
			int alfa = (int)Math::Max(15.0f, 80.0f - (Config::SUELO - m->getY()) * 0.12f);
			SolidBrush sombra(Color::FromArgb(alfa, 0, 0, 0));
			g->FillEllipse(%sombra, m->getX() - r * 0.8f, Config::SUELO - 5.0f, r * 1.6f, 10.0f);
		}

		for each (Asteroide^ m in meteoros) {
			m->dibujar(g);
		}

		jugador->dibujar(g);
		Particulas::dibujar(g, particulas);
	}
}
