#include "MinijuegoJupiter.h"
#include "Superficie.h"
#include "Config.h"
#include "Azar.h"
#include "Sonido.h"
#include "Estilos.h"

namespace JuegoEspacial {

	MinijuegoJupiter::MinijuegoJupiter(Astronauta^ pjugador)
		: Minijuego(pjugador, 4, L"Tormenta eléctrica",
			L"Estás sobre la cubierta de nubes de Júpiter.\n"
			L"Cuando veas la señal de aviso (!) apártate: ¡ahí caerá un rayo!\n"
			L"El viento te empuja. Sobrevive 25 segundos.") {
		rayos = gcnew List<RayoTormenta^>();
		naveEstacionada = gcnew Nave(80.0f, Config::SUELO);
		restante = 25 * Config::TICKS_SEGUNDO;
		enfriamientoRayo = 70;
		viento = 0.0f;
		cicloViento = 240;
	}

	void MinijuegoJupiter::iniciar() {
		jugador->setEscala(0.95f);
		jugador->setJetpack(false);
		jugador->setVidas(3);
		jugador->setPosicion(500.0f, Config::SUELO);
	}

	String^ MinijuegoJupiter::progreso() {
		String^ flecha = L"";
		if (viento > 0.1f) flecha = L"   Viento →";
		if (viento < -0.1f) flecha = L"   Viento ←";
		return String::Format(L"Sobrevive: {0} s{1}", Math::Max(0, (restante + 59) / 60), flecha);
	}

	void MinijuegoJupiter::actualizar(Entrada^ entrada) {
		tictac();
		naveEstacionada->mover();
		if (!jugando()) return;

		restante--;
		if (restante <= 0) {
			sumar(60);
			ganar();
			return;
		}

		// rachas de viento periodicas
		cicloViento--;
		if (cicloViento <= 0) {
			cicloViento = 240;
			viento = Azar::entreF(1.6f, 2.6f) * (Azar::moneda() ? 1.0f : -1.0f);
		}
		if (cicloViento < 150) viento = 0.0f;     // la racha dura 90 ticks

		// nuevos rayos (mas seguidos con el tiempo)
		enfriamientoRayo--;
		if (enfriamientoRayo <= 0) {
			rayos->Add(gcnew RayoTormenta(Azar::entreF(60.0f, 940.0f)));
			enfriamientoRayo = Math::Max(36, 68 - (1500 - restante) / 60);
		}

		// movimiento (el viento empuja siempre)
		float dx = viento;
		if (entrada->izquierda) dx -= 5.6f;
		if (entrada->derecha) dx += 5.6f;
		jugador->caminar(dx, 25.0f, 975.0f);
		jugador->mover();

		// rayos activos
		for (int i = rayos->Count - 1; i >= 0; i--) {
			RayoTormenta^ rayo = rayos[i];
			rayo->t++;
			if (rayo->t == RayoTormenta::AVISO) Sonido::tono(1500, 30);
			if (rayo->t > RayoTormenta::AVISO && rayo->t <= RayoTormenta::FIN) {
				// descarga: hace danio si el jugador esta debajo
				if (!jugador->esInvulnerable() && Math::Abs(jugador->getX() - rayo->x) < 38.0f) {
					jugador->perderVida();
					Particulas::explosion(particulas, jugador->getX(), jugador->getY() - 40.0f, Color::FromArgb(255, 250, 150), 20);
					if (jugador->getVidas() <= 0) perder();
				}
				// chispas en el suelo
				if (ticks % 3 == 0) {
					particulas->Add(gcnew Particula(rayo->x + Azar::entreF(-20.0f, 20.0f), Config::SUELO,
						Azar::entreF(-2.0f, 2.0f), Azar::entreF(-3.0f, -0.5f),
						2.5f, 18, Color::FromArgb(255, 250, 160), true));
				}
			}
			if (rayo->t > RayoTormenta::FIN) rayos->RemoveAt(i);
		}
	}

	void MinijuegoJupiter::dibujar(Graphics^ g) {
		Superficie::dibujarFondo(g, 4, ticks);
		naveEstacionada->dibujar(g);

		for each (RayoTormenta^ rayo in rayos) {
			if (rayo->t <= RayoTormenta::AVISO) {
				// advertencia parpadeante
				int alfa = 120 + (int)(100.0 * Math::Sin(rayo->t * 0.5));
				if (alfa < 0) alfa = 0;
				if (alfa > 255) alfa = 255;
				Pen guia(Color::FromArgb(alfa / 2, 255, 240, 120), 2.0f);
				g->DrawLine(%guia, rayo->x, 60.0f, rayo->x, Config::SUELO);
				array<PointF>^ triangulo = gcnew array<PointF>(3);
				triangulo[0] = PointF(rayo->x - 14.0f, Config::SUELO - 36.0f);
				triangulo[1] = PointF(rayo->x + 14.0f, Config::SUELO - 36.0f);
				triangulo[2] = PointF(rayo->x, Config::SUELO - 10.0f);
				Pen marco(Color::FromArgb(alfa, 255, 220, 60), 2.5f);
				g->DrawPolygon(%marco, triangulo);
				Estilos::textoCentrado(g, L"!", Estilos::normalNegrita, Color::FromArgb(alfa, 255, 240, 120),
					rayo->x, Config::SUELO - 28.0f);
			}
			else {
				// descarga en zigzag (parpadea distinto en cada cuadro)
				int alfa = 255 - (rayo->t - RayoTormenta::AVISO) * 9;
				if (alfa < 60) alfa = 60;
				SolidBrush columna(Color::FromArgb(50, 255, 250, 170));
				g->FillRectangle(%columna, rayo->x - 26.0f, 50.0f, 52.0f, Config::SUELO - 50.0f);
				array<PointF>^ zigzag = gcnew array<PointF>(8);
				for (int p = 0; p < 8; p++) {
					float py = 50.0f + p * (Config::SUELO - 50.0f) / 7.0f;
					float px = rayo->x + ((p == 0 || p == 7) ? 0.0f : Azar::entreF(-16.0f, 16.0f));
					zigzag[p] = PointF(px, py);
				}
				Pen halo(Color::FromArgb(alfa / 2, 255, 240, 120), 10.0f);
				g->DrawLines(%halo, zigzag);
				Pen nucleo(Color::FromArgb(alfa, 255, 255, 230), 3.5f);
				g->DrawLines(%nucleo, zigzag);
			}
		}

		// lineas de viento
		if (viento != 0.0f) {
			Pen rafaga(Color::FromArgb(90, 240, 240, 255), 2.0f);
			int direccion = (viento > 0.0f) ? 1 : -1;
			for (int i = 0; i < 7; i++) {
				int base = (ticks * 6 * direccion + i * 150) % 1100;
				if (base < 0) base += 1100;
				float vx = (float)base - 50.0f;
				float vy = 150.0f + i * 60.0f;
				g->DrawLine(%rafaga, vx, vy, vx + 46.0f * (viento > 0 ? 1.0f : -1.0f), vy);
			}
		}

		jugador->dibujar(g);
		Particulas::dibujar(g, particulas);
	}
}
