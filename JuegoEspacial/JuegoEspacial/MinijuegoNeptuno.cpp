#include "MinijuegoNeptuno.h"
#include "Superficie.h"
#include "Config.h"
#include "Azar.h"
#include "Sonido.h"

namespace JuegoEspacial {

	MinijuegoNeptuno::MinijuegoNeptuno(Astronauta^ pjugador)
		: Minijuego(pjugador, 7, L"El Jefe Final",
			L"¡El JEFE FINAL custodia Neptuno!\n"
			L"Muévete con A / D o las flechas y dispara con ESPACIO.\n"
			L"Esquiva sus rayos y asteroides. ¡Libera la galaxia!") {
		jefe = gcnew JefeFinal(500.0f, 150.0f, 24);
		rayos = gcnew List<Proyectil^>();
		asteroides = gcnew List<Asteroide^>();
		laseres = gcnew List<Proyectil^>();
		naveEstacionada = gcnew Nave(80.0f, Config::SUELO);
		enfriamientoDisparo = 0;
		enfriamientoAtaque = 110;
		avisoAtaque = 0;
		tipoAtaque = 0;
	}

	void MinijuegoNeptuno::iniciar() {
		jugador->setEscala(0.95f);
		jugador->setJetpack(false);
		jugador->setVidas(3);
		jugador->setPosicion(500.0f, Config::SUELO);
	}

	String^ MinijuegoNeptuno::progreso() {
		return String::Format(L"Vida del jefe: {0}/{1}", Math::Max(0, jefe->getVida()), jefe->getVidaMax());
	}

	void MinijuegoNeptuno::dibujarHUD(Graphics^ g) {
		Minijuego::dibujarHUD(g);
		jefe->dibujarBarraVida(g);
	}

	void MinijuegoNeptuno::actualizar(Entrada^ entrada) {
		tictac();
		naveEstacionada->mover();
		if (!jugando()) return;

		// el jefe sigue al jugador con la mirada
		jefe->setObjetivo(jugador->getX(), jugador->getY() - 45.0f);
		jefe->mover();

		// eleccion del proximo ataque
		if (avisoAtaque <= 0) {
			enfriamientoAtaque--;
			if (enfriamientoAtaque <= 0) {
				jefe->atacar();                                   // carga de energia (aviso)
				Sonido::tono(260, 70);
				tipoAtaque = (Azar::entre(0, 3) == 2) ? 1 : 0;    // mas rayos que asteroides
				avisoAtaque = 24;
			}
		}
		else {
			avisoAtaque--;
			if (avisoAtaque == 0) {
				if (tipoAtaque == 0) {
					rayos->Add(jefe->dispararRayo(jugador->getX(), Config::SUELO - 30.0f));
					Sonido::tono(480, 60);
				}
				else {
					asteroides->Add(jefe->lanzarAsteroide());
					if (jefe->enfurecido()) asteroides->Add(jefe->lanzarAsteroide());
				}
				enfriamientoAtaque = jefe->enfurecido() ? 70 : 115;
			}
		}

		// movimiento y disparo del jugador
		if (entrada->izquierda) jugador->caminar(-6.0f, 25.0f, 975.0f);
		if (entrada->derecha) jugador->caminar(6.0f, 25.0f, 975.0f);
		if (enfriamientoDisparo > 0) enfriamientoDisparo--;
		if (entrada->disparo && enfriamientoDisparo <= 0) {
			laseres->Add(jugador->disparar());
			enfriamientoDisparo = 20;
		}
		jugador->mover();

		// laseres del jugador
		for (int i = laseres->Count - 1; i >= 0; i--) {
			Proyectil^ laser = laseres[i];
			laser->mover();
			if (laser->fuera()) {
				laseres->RemoveAt(i);
				continue;
			}

			// contra el jefe
			if (laser->zona().IntersectsWith(jefe->zona())) {
				jefe->recibirGolpe(1);
				sumar(10);
				Sonido::tono(350, 30);
				Particulas::explosion(particulas, laser->getX(), laser->getY(), Color::FromArgb(255, 160, 250), 10);
				laseres->RemoveAt(i);
				if (jefe->destruido()) {
					Particulas::explosion(particulas, jefe->getX(), jefe->getY(), Color::FromArgb(255, 200, 90), 40);
					Particulas::explosion(particulas, jefe->getX() - 40.0f, jefe->getY() + 10.0f, Color::FromArgb(255, 110, 220), 30);
					Particulas::explosion(particulas, jefe->getX() + 40.0f, jefe->getY() + 10.0f, Color::FromArgb(120, 230, 255), 30);
					sumar(150);
					ganar();
					return;
				}
				continue;
			}

			// contra los asteroides
			for (int j = asteroides->Count - 1; j >= 0; j--) {
				if (laser->zona().IntersectsWith(asteroides[j]->zona())) {
					asteroides[j]->recibirGolpe(1);
					if (asteroides[j]->destruido()) {
						Particulas::explosion(particulas, asteroides[j]->getX(), asteroides[j]->getY(), Color::FromArgb(170, 120, 90), 14);
						Sonido::explosion();
						sumar(5);
						asteroides->RemoveAt(j);
					}
					laseres->RemoveAt(i);
					break;
				}
			}
		}

		// rayos del jefe
		for (int i = rayos->Count - 1; i >= 0; i--) {
			Proyectil^ rayo = rayos[i];
			rayo->mover();
			if (!jugador->esInvulnerable() && rayo->zona().IntersectsWith(jugador->zona())) {
				jugador->perderVida();
				Particulas::explosion(particulas, jugador->getX(), jugador->getY() - 40.0f, Color::FromArgb(255, 120, 220), 18);
				rayos->RemoveAt(i);
				if (jugador->getVidas() <= 0) perder();
				continue;
			}
			if (rayo->getY() > Config::SUELO - 6.0f) {
				Particulas::explosion(particulas, rayo->getX(), Config::SUELO - 8.0f, Color::FromArgb(255, 120, 220), 8);
				rayos->RemoveAt(i);
				continue;
			}
			if (rayo->fuera()) rayos->RemoveAt(i);
		}

		// asteroides del jefe
		for (int i = asteroides->Count - 1; i >= 0; i--) {
			Asteroide^ asteroide = asteroides[i];
			asteroide->mover();
			if (!jugador->esInvulnerable() && asteroide->zona().IntersectsWith(jugador->zona())) {
				asteroide->atacar();
				jugador->perderVida();
				Particulas::explosion(particulas, jugador->getX(), jugador->getY() - 40.0f, Color::FromArgb(255, 140, 90), 18);
				asteroides->RemoveAt(i);
				if (jugador->getVidas() <= 0) perder();
				continue;
			}
			if (asteroide->getY() > Config::SUELO - asteroide->getRadio() * 0.4f) {
				Particulas::explosion(particulas, asteroide->getX(), Config::SUELO - 6.0f, Color::FromArgb(170, 120, 90), 10);
				asteroides->RemoveAt(i);
			}
		}
	}

	void MinijuegoNeptuno::dibujar(Graphics^ g) {
		Superficie::dibujarFondo(g, 7, ticks);
		naveEstacionada->dibujar(g);

		jefe->dibujar(g);

		for each (Proyectil^ laser in laseres) {
			laser->dibujar(g);
		}
		for each (Proyectil^ rayo in rayos) {
			rayo->dibujar(g);
		}
		for each (Asteroide^ asteroide in asteroides) {
			asteroide->dibujar(g);
		}

		jugador->dibujar(g);
		Particulas::dibujar(g, particulas);
	}
}
