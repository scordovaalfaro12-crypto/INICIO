#include "Aterrizaje.h"
#include "Superficie.h"
#include "SistemaSolar.h"
#include "Config.h"
#include "Sonido.h"
#include "Azar.h"
#include "Estilos.h"

namespace JuegoEspacial {

	Aterrizaje::Aterrizaje(int pplaneta, bool pdespegue, Astronauta^ pjugador) {
		planeta = pplaneta;
		despegue = pdespegue;
		jugador = pjugador;
		tick = 0;
		terminado = false;
		sonoAterrizaje = false;
		sonoDespegue = false;
		naveX = 240.0f;
		naveY = despegue ? Config::SUELO : -30.0f;
		nave = gcnew Nave(naveX, naveY);
		nave->setPatas(true);
		particulas = gcnew List<Particula^>();
		jugador->setEscala(1.0f);
		jugador->setJetpack(false);
		if (despegue) {
			jugador->setPosicion(naveX + 240.0f, Config::SUELO);
		}
	}

	float Aterrizaje::suavizar(float t) {
		return t * t * (3.0f - 2.0f * t);
	}

	bool Aterrizaje::haTerminado() {
		return terminado;
	}

	void Aterrizaje::saltar() {
		terminado = true;
	}

	void Aterrizaje::actualizar() {
		if (terminado) return;
		tick++;
		nave->mover();
		jugador->mover();
		Particulas::actualizar(particulas);

		float suelo = Config::SUELO;

		if (!despegue) {
			// ---------- ATERRIZAJE ----------
			// 0-150: descenso frenando con la llama encendida
			if (tick <= 150) {
				float p = suavizar(tick / 150.0f);
				naveY = -30.0f + (suelo + 30.0f) * p;
				naveX = 240.0f + (float)Math::Sin(tick * 0.06) * 4.0f;
				nave->setLlama(true);
				Particulas::llama(particulas, naveX, naveY - 4.0f, 1.0f);
				if (tick % 4 == 0) Particulas::llama(particulas, naveX, naveY - 4.0f, 1.4f);
			}
			// 150: toca el suelo, polvo y sonido
			if (tick == 150) {
				naveY = suelo;
				nave->setLlama(false);
				Sonido::aterrizajeNave();
				Particulas::polvo(particulas, naveX - 20.0f, suelo, Superficie::colorSueloOscuro(planeta), 16);
				Particulas::polvo(particulas, naveX + 20.0f, suelo, Superficie::colorSueloOscuro(planeta), 16);
			}
			// 235-265: el astronauta baja por la escalera
			if (tick == 235) {
				jugador->setPosicion(naveX + 24.0f, suelo - 46.0f);
			}
			if (tick > 235 && tick <= 265) {
				float p = (tick - 235) / 30.0f;
				jugador->setPosicion(naveX + 24.0f, suelo - 46.0f + 46.0f * p);
			}
			// 265-345: camina hacia su puesto
			if (tick > 265 && tick <= 345) {
				jugador->caminar(2.9f, 0.0f, 1000.0f);
			}
			if (tick >= 400) terminado = true;
			nave->setPosicion(naveX, naveY);
		}
		else {
			// ---------- DESPEGUE ----------
			// 0-70: camina de regreso a la nave
			if (tick <= 70) {
				if (jugador->getX() > naveX + 26.0f) {
					jugador->caminar(-3.2f, 0.0f, 1000.0f);
				}
			}
			// 70-100: sube por la escalera
			if (tick > 70 && tick <= 100) {
				float p = (tick - 70) / 30.0f;
				jugador->setPosicion(naveX + 24.0f, Config::SUELO - 46.0f * p);
			}
			// 100: ignicion
			if (tick == 100 && !sonoDespegue) {
				sonoDespegue = true;
				Sonido::despegueNave();
			}
			if (tick > 100 && tick <= 160) {
				nave->setLlama(true);
				naveX = 240.0f + Azar::entreF(-2.0f, 2.0f);     // vibracion del motor
				Particulas::llama(particulas, naveX, naveY - 2.0f, 1.2f);
				Particulas::polvo(particulas, naveX, Config::SUELO, Superficie::colorSueloOscuro(planeta), 1);
			}
			// 160+: asciende acelerando
			if (tick > 160) {
				float velocidad = (tick - 160) * 0.14f;
				naveY -= velocidad;
				Particulas::llama(particulas, naveX, naveY - 2.0f, 1.5f);
				if (naveY < -160.0f) terminado = true;
			}
			nave->setPosicion(naveX, naveY);
		}
	}

	void Aterrizaje::dibujarEscalera(Graphics^ g, float progreso) {
		// escalera que sale del costado de la nave hasta el suelo
		float topeY = naveY - 52.0f;
		float largo = (Config::SUELO - topeY) * progreso;
		Pen riel(Color::FromArgb(180, 185, 200), 3.0f);
		g->DrawLine(%riel, naveX + 17.0f, topeY, naveX + 17.0f, topeY + largo);
		g->DrawLine(%riel, naveX + 30.0f, topeY, naveX + 30.0f, topeY + largo);
		Pen peldano(Color::FromArgb(150, 155, 170), 2.0f);
		int cantidad = (int)(largo / 12.0f);
		for (int i = 1; i <= cantidad; i++) {
			g->DrawLine(%peldano, naveX + 17.0f, topeY + i * 12.0f, naveX + 30.0f, topeY + i * 12.0f);
		}
	}

	void Aterrizaje::dibujar(Graphics^ g, int tickGlobal) {
		Superficie::dibujarFondo(g, planeta, tickGlobal);

		// escotilla y escalera
		if (!despegue) {
			if (tick > 190) {
				float p = Math::Min(1.0f, (tick - 190) / 45.0f);
				// puerta circular que se abre (se oscurece)
				SolidBrush puerta(Color::FromArgb((int)(220 * p), 25, 28, 40));
				g->FillEllipse(%puerta, naveX + 4.0f, naveY - 64.0f, 14.0f, 18.0f);
				dibujarEscalera(g, p);
			}
		}
		else {
			if (tick <= 100) {
				float p = 1.0f - Math::Max(0.0f, (tick - 85.0f) / 15.0f);     // se cierra al final
				SolidBrush puerta(Color::FromArgb((int)(220 * p), 25, 28, 40));
				g->FillEllipse(%puerta, naveX + 4.0f, naveY - 64.0f, 14.0f, 18.0f);
				dibujarEscalera(g, p);
			}
		}

		nave->dibujar(g);
		Particulas::dibujar(g, particulas);

		// el astronauta es visible cuando esta fuera de la nave
		bool jugadorVisible = despegue ? (tick <= 100) : (tick >= 235);
		if (jugadorVisible) jugador->dibujar(g);

		// rotulos de la cinematica
		String^ nombre = SistemaSolar::nombre(planeta);
		if (!despegue) {
			if (tick < 150) {
				Estilos::textoSombra(g, String::Format(L"Aterrizando en {0}...", nombre), Estilos::grande,
					Color::FromArgb(170, 220, 255), 500.0f, 90.0f);
			}
			else if (tick > 330) {
				Estilos::textoSombra(g, String::Format(L"¡A explorar {0}!", nombre), Estilos::grande,
					Color::FromArgb(255, 215, 90), 500.0f, 90.0f);
			}
		}
		else {
			if (tick < 100) {
				Estilos::textoSombra(g, L"Volviendo a la nave...", Estilos::grande,
					Color::FromArgb(170, 220, 255), 500.0f, 90.0f);
			}
			else {
				Estilos::textoSombra(g, L"¡Despegue!", Estilos::grande,
					Color::FromArgb(255, 215, 90), 500.0f, 90.0f);
			}
		}

		Estilos::textoIzquierda(g, L"CLIC o ENTER para saltar", Estilos::pequena,
			Color::FromArgb(120, 140, 180), 830.0f, 678.0f);
	}
}
