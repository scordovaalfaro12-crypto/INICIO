#include "MinijuegoUrano.h"
#include "Superficie.h"
#include "Config.h"
#include "Azar.h"
#include "Sonido.h"
#include "Estilos.h"

namespace JuegoEspacial {

	MinijuegoUrano::MinijuegoUrano(Astronauta^ pjugador)
		: Minijuego(pjugador, 6, L"Cristales de memoria",
			L"Los cristales de Urano guardan una melodía antigua.\n"
			L"OBSERVA la secuencia de luces y sonidos, y repítela\n"
			L"haciendo CLIC en los cristales. Supera 6 rondas. 3 errores y pierdes.") {
		secuencia = gcnew List<int>();
		naveEstacionada = gcnew Nave(80.0f, Config::SUELO);
		faseSimon = 2;
		temporizador = 70;
		mostrando = 0;
		paso = 0;
		errores = 0;
		iluminado = -1;
		iluminadoTicks = 0;
		cx = gcnew array<float> { 545.0f, 700.0f, 545.0f, 390.0f };   // arriba, derecha, abajo, izquierda
		cy = gcnew array<float> { 215.0f, 370.0f, 525.0f, 370.0f };
	}

	void MinijuegoUrano::iniciar() {
		jugador->setEscala(0.95f);
		jugador->setJetpack(false);
		jugador->setVidas(3);
		jugador->setPosicion(195.0f, Config::SUELO);
		secuencia->Clear();
		secuencia->Add(Azar::entre(0, 4));
	}

	Color MinijuegoUrano::colorCristal(int indice) {
		switch (indice) {
		case 0: return Color::FromArgb(80, 220, 230);     // cian
		case 1: return Color::FromArgb(220, 90, 200);     // magenta
		case 2: return Color::FromArgb(235, 210, 90);     // ambar
		default: return Color::FromArgb(110, 220, 120);   // verde
		}
	}

	String^ MinijuegoUrano::progreso() {
		return String::Format(L"Ronda: {0}/{1}   Errores: {2}/3", secuencia->Count, RONDA_FINAL, errores);
	}

	void MinijuegoUrano::encender(int indice) {
		iluminado = indice;
		iluminadoTicks = 26;
		Sonido::notaSimon(indice);
	}

	int MinijuegoUrano::cristalEn(int mx, int my) {
		for (int i = 0; i < 4; i++) {
			float dx = mx - cx[i];
			float dy = my - cy[i];
			if (Math::Abs(dx) / 52.0f + Math::Abs(dy) / 68.0f <= 1.0f) return i;   // rombo
		}
		return -1;
	}

	void MinijuegoUrano::actualizar(Entrada^ entrada) {
		tictac();
		naveEstacionada->mover();
		jugador->mover();
		if (!jugando()) return;

		if (iluminadoTicks > 0) {
			iluminadoTicks--;
			if (iluminadoTicks == 0) iluminado = -1;
		}

		if (faseSimon == 2) {
			// pausa antes de mostrar la secuencia
			temporizador--;
			if (temporizador <= 0) {
				faseSimon = 0;
				mostrando = 0;
				temporizador = 16;
			}
		}
		else if (faseSimon == 0) {
			// el juego enciende cristal por cristal
			temporizador--;
			if (temporizador <= 0) {
				if (mostrando < secuencia->Count) {
					encender(secuencia[mostrando]);
					mostrando++;
					temporizador = 38;
				}
				else {
					faseSimon = 1;
					paso = 0;
				}
			}
		}
		// faseSimon == 1: espera los clics del jugador
	}

	void MinijuegoUrano::clic(int mx, int my) {
		if (!jugando()) return;
		if (faseSimon != 1) return;

		int indice = cristalEn(mx, my);
		if (indice == -1) return;

		encender(indice);

		if (indice == secuencia[paso]) {
			paso++;
			if (paso == secuencia->Count) {
				sumar(15);
				if (secuencia->Count >= RONDA_FINAL) {
					sumar(40);
					ganar();
					return;
				}
				secuencia->Add(Azar::entre(0, 4));
				faseSimon = 2;
				temporizador = 55;
			}
		}
		else {
			Sonido::error();
			errores++;
			jugador->perderVida();
			if (errores >= 3) {
				perder();
				return;
			}
			faseSimon = 2;        // vuelve a mostrar la misma secuencia
			temporizador = 70;
		}
	}

	void MinijuegoUrano::dibujarCristal(Graphics^ g, int indice, bool encendido) {
		Color base = colorCristal(indice);
		Color cuerpo = encendido ? base : Estilos::oscurecer(base, 0.4f);

		// halo cuando esta encendido
		if (encendido) {
			SolidBrush halo(Color::FromArgb(90, base));
			g->FillEllipse(%halo, cx[indice] - 75.0f, cy[indice] - 90.0f, 150.0f, 180.0f);
		}

		// rombo principal
		array<PointF>^ rombo = gcnew array<PointF>(4);
		rombo[0] = PointF(cx[indice], cy[indice] - 64.0f);
		rombo[1] = PointF(cx[indice] + 48.0f, cy[indice]);
		rombo[2] = PointF(cx[indice], cy[indice] + 64.0f);
		rombo[3] = PointF(cx[indice] - 48.0f, cy[indice]);
		SolidBrush relleno(cuerpo);
		g->FillPolygon(%relleno, rombo);
		Pen filo(encendido ? Color::White : Estilos::aclarar(cuerpo, 0.25f), 2.5f);
		g->DrawPolygon(%filo, rombo);

		// facetas internas
		Pen faceta(Color::FromArgb(encendido ? 220 : 90, 255, 255, 255), 1.5f);
		g->DrawLine(%faceta, cx[indice], cy[indice] - 64.0f, cx[indice], cy[indice] + 64.0f);
		g->DrawLine(%faceta, cx[indice] - 48.0f, cy[indice], cx[indice] + 48.0f, cy[indice]);
		g->DrawLine(%faceta, cx[indice], cy[indice] - 64.0f, cx[indice] + 24.0f, cy[indice]);
		g->DrawLine(%faceta, cx[indice], cy[indice] + 64.0f, cx[indice] - 24.0f, cy[indice]);
	}

	void MinijuegoUrano::dibujar(Graphics^ g) {
		Superficie::dibujarFondo(g, 6, ticks);
		naveEstacionada->dibujar(g);

		// pedestal central
		SolidBrush pedestal(Color::FromArgb(60, 110, 150));
		g->FillEllipse(%pedestal, 545.0f - 60.0f, 370.0f - 20.0f, 120.0f, 40.0f);
		Color nucleoColor = (faseSimon == 1) ? Color::FromArgb(110, 230, 120) : Color::FromArgb(235, 210, 90);
		float pulso = 10.0f + 2.5f * (float)Math::Sin(ticks * 0.15);
		SolidBrush nucleo(nucleoColor);
		g->FillEllipse(%nucleo, 545.0f - pulso, 370.0f - pulso, pulso * 2.0f, pulso * 2.0f);

		// los 4 cristales
		for (int i = 0; i < 4; i++) {
			dibujarCristal(g, i, iluminado == i);
		}

		// el astronauta observa desde un lado
		jugador->dibujar(g);

		// cartel de estado
		String^ mensaje = (faseSimon == 1) ? L"¡REPITE LA SECUENCIA!" : L"OBSERVA...";
		Color colorMensaje = (faseSimon == 1) ? Color::FromArgb(140, 250, 150) : Color::FromArgb(255, 225, 120);
		Estilos::textoSombra(g, mensaje, Estilos::subtitulo, colorMensaje, 545.0f, 90.0f);

		Particulas::dibujar(g, particulas);
	}
}
