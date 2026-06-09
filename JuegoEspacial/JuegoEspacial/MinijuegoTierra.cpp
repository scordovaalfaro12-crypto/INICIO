#include "MinijuegoTierra.h"
#include "Superficie.h"
#include "Config.h"
#include "Azar.h"
#include "Sonido.h"

namespace JuegoEspacial {

	MinijuegoTierra::MinijuegoTierra(Astronauta^ pjugador)
		: Minijuego(pjugador, 2, L"Limpieza orbital",
			L"¡La órbita de la Tierra está llena de basura espacial!\n"
			L"Muévete con A / D o las flechas y dispara con ESPACIO.\n"
			L"Destrúyela toda antes de que llegue a la plataforma.") {
		piezas = gcnew List<BasuraEspacial^>();
		laseres = gcnew List<Proyectil^>();
		totalPiezas = 15;
		direccion = 1.0f;
		enfriamientoDisparo = 0;
	}

	void MinijuegoTierra::iniciar() {
		jugador->setEscala(0.95f);
		jugador->setJetpack(false);
		jugador->setVidas(3);
		jugador->setPosicion(500.0f, Config::SUELO);

		// formacion de 3 filas x 5 columnas
		piezas->Clear();
		for (int fila = 0; fila < 3; fila++) {
			for (int columna = 0; columna < 5; columna++) {
				piezas->Add(gcnew BasuraEspacial(
					190.0f + columna * 120.0f,
					110.0f + fila * 80.0f,
					(fila + columna) % 3));
			}
		}
	}

	String^ MinijuegoTierra::progreso() {
		return String::Format(L"Basura restante: {0}", piezas->Count);
	}

	void MinijuegoTierra::actualizar(Entrada^ entrada) {
		tictac();
		if (!jugando()) return;

		// la formacion va y viene, y baja al tocar un borde (cada vez mas rapida)
		float velocidad = 1.1f + (totalPiezas - piezas->Count) * 0.16f;
		bool bajar = false;
		for each (BasuraEspacial^ pieza in piezas) {
			float nx = pieza->getX() + direccion * velocidad;
			if (nx > 955.0f || nx < 45.0f) bajar = true;
		}
		if (bajar) {
			direccion = -direccion;
			for each (BasuraEspacial^ pieza in piezas) {
				pieza->setPosicion(pieza->getX(), pieza->getY() + 18.0f);
			}
		}
		for each (BasuraEspacial^ pieza in piezas) {
			pieza->setPosicion(pieza->getX() + direccion * velocidad, pieza->getY());
			pieza->mover();
		}

		// movimiento y disparo del jugador
		if (entrada->izquierda) jugador->caminar(-6.2f, 25.0f, 975.0f);
		if (entrada->derecha) jugador->caminar(6.2f, 25.0f, 975.0f);
		if (enfriamientoDisparo > 0) enfriamientoDisparo--;
		if (entrada->disparo && enfriamientoDisparo <= 0) {
			laseres->Add(jugador->disparar());
			enfriamientoDisparo = 16;
		}
		jugador->mover();

		// laseres
		for (int i = laseres->Count - 1; i >= 0; i--) {
			Proyectil^ laser = laseres[i];
			laser->mover();
			if (laser->fuera()) {
				laseres->RemoveAt(i);
				continue;
			}
			for (int j = piezas->Count - 1; j >= 0; j--) {
				BasuraEspacial^ pieza = piezas[j];
				if (laser->zona().IntersectsWith(pieza->zona())) {
					pieza->atacar();
					pieza->recibirGolpe(1);
					if (pieza->destruido()) {
						Particulas::explosion(particulas, pieza->getX(), pieza->getY(), Color::FromArgb(120, 220, 255), 18);
						Sonido::explosion();
						sumar(15);
						piezas->RemoveAt(j);
					}
					laseres->RemoveAt(i);
					break;
				}
			}
		}

		// derrota si la basura llega a la plataforma o toca al jugador
		for (int j = piezas->Count - 1; j >= 0; j--) {
			BasuraEspacial^ pieza = piezas[j];
			if (!jugador->esInvulnerable() && pieza->zona().IntersectsWith(jugador->zona())) {
				pieza->atacar();
				jugador->perderVida();
				Particulas::explosion(particulas, pieza->getX(), pieza->getY(), Color::FromArgb(255, 120, 80), 16);
				piezas->RemoveAt(j);
				if (jugador->getVidas() <= 0) perder();
				continue;
			}
			if (pieza->getY() > 505.0f) {
				perder();
			}
		}

		if (piezas->Count == 0 && jugando()) {
			sumar(50);
			ganar();
		}
	}

	void MinijuegoTierra::dibujar(Graphics^ g) {
		Superficie::dibujarFondo(g, 2, ticks);

		for each (Proyectil^ laser in laseres) {
			laser->dibujar(g);
		}
		for each (BasuraEspacial^ pieza in piezas) {
			pieza->dibujar(g);
		}

		jugador->dibujar(g);
		Particulas::dibujar(g, particulas);

		// linea de peligro
		Pen peligro(Color::FromArgb(70 + (int)(40.0 * Math::Sin(ticks * 0.1)), 255, 80, 80), 2.0f);
		peligro.DashStyle = System::Drawing::Drawing2D::DashStyle::Dash;
		g->DrawLine(%peligro, 0.0f, 508.0f, 1000.0f, 508.0f);
	}
}
