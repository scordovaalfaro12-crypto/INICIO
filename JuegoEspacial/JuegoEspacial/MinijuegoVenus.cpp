#include "MinijuegoVenus.h"
#include "Superficie.h"
#include "Config.h"
#include "Azar.h"

namespace JuegoEspacial {

	MinijuegoVenus::MinijuegoVenus(Astronauta^ pjugador)
		: Minijuego(pjugador, 1, L"Atrapa estrellas",
			L"Las nubes de Venus dejan caer polvo de estrellas.\n"
			L"Atrapa 15 estrellas moviéndote con A / D o las flechas.\n"
			L"¡Cuidado con las gotas de lluvia ácida!") {
		estrellas = gcnew List<Estrella^>();
		gotas = gcnew List<Proyectil^>();
		naveEstacionada = gcnew Nave(80.0f, Config::SUELO);
		atrapadas = 0;
		enfriamientoEstrella = 40;
		enfriamientoGota = 90;
	}

	void MinijuegoVenus::iniciar() {
		jugador->setEscala(0.95f);
		jugador->setJetpack(false);
		jugador->setVidas(3);
		jugador->setPosicion(500.0f, Config::SUELO);
	}

	String^ MinijuegoVenus::progreso() {
		return String::Format(L"Estrellas: {0}/15", atrapadas);
	}

	void MinijuegoVenus::actualizar(Entrada^ entrada) {
		tictac();
		naveEstacionada->mover();
		if (!jugando()) return;

		// aparecen estrellas
		enfriamientoEstrella--;
		if (enfriamientoEstrella <= 0) {
			estrellas->Add(gcnew Estrella(
				Azar::entreF(40.0f, 960.0f), -30.0f,
				Azar::entreF(2.2f, 3.6f), 13.0f, 10));
			enfriamientoEstrella = Azar::entre(45, 75);
		}

		// aparecen gotas acidas (cada vez mas seguido)
		enfriamientoGota--;
		if (enfriamientoGota <= 0) {
			gotas->Add(gcnew Proyectil(
				Azar::entreF(40.0f, 960.0f), -20.0f,
				0.0f, Azar::entreF(3.6f, 5.2f), Proyectil::GOTA));
			enfriamientoGota = Math::Max(40, 85 - atrapadas * 3);
		}

		// movimiento del jugador
		if (entrada->izquierda) jugador->caminar(-6.2f, 25.0f, 975.0f);
		if (entrada->derecha) jugador->caminar(6.2f, 25.0f, 975.0f);
		jugador->mover();

		// estrellas
		for (int i = estrellas->Count - 1; i >= 0; i--) {
			Estrella^ e = estrellas[i];
			e->mover();

			if (e->zona().IntersectsWith(jugador->zona())) {
				jugador->recolectarItem(e);              // aplica su efecto (suma puntos)
				puntajeLocal += e->getValor();
				atrapadas++;
				Particulas::explosion(particulas, e->getX(), e->getY(), Color::FromArgb(255, 220, 100), 12);
				estrellas->RemoveAt(i);
				if (atrapadas >= 15) ganar();
				continue;
			}

			if (e->getY() > Config::SUELO + 10.0f) {
				estrellas->RemoveAt(i);
			}
		}

		// gotas acidas
		for (int i = gotas->Count - 1; i >= 0; i--) {
			Proyectil^ gota = gotas[i];
			gota->mover();

			if (!jugador->esInvulnerable() && gota->zona().IntersectsWith(jugador->zona())) {
				jugador->perderVida();
				Particulas::explosion(particulas, gota->getX(), gota->getY(), Color::FromArgb(140, 230, 90), 14);
				gotas->RemoveAt(i);
				if (jugador->getVidas() <= 0) perder();
				continue;
			}

			if (gota->getY() > Config::SUELO - 4.0f) {
				Particulas::explosion(particulas, gota->getX(), Config::SUELO - 4.0f, Color::FromArgb(140, 230, 90), 6);
				gotas->RemoveAt(i);
			}
		}
	}

	void MinijuegoVenus::dibujar(Graphics^ g) {
		Superficie::dibujarFondo(g, 1, ticks);
		naveEstacionada->dibujar(g);

		for each (Estrella^ e in estrellas) {
			e->dibujar(g);
		}
		for each (Proyectil^ gota in gotas) {
			gota->dibujar(g);
		}

		jugador->dibujar(g);
		Particulas::dibujar(g, particulas);
	}
}
