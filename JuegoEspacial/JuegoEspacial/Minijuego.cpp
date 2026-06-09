#include "Minijuego.h"
#include "Sonido.h"
#include "Estilos.h"
#include "Config.h"
#include "SistemaSolar.h"

namespace JuegoEspacial {

	Minijuego::Minijuego(Astronauta^ pjugador, int pplaneta, String^ ptitulo, String^ pinstrucciones) {
		jugador = pjugador;
		planeta = pplaneta;
		titulo = ptitulo;
		instrucciones = pinstrucciones;
		estadoInterno = 0;
		ticks = 0;
		puntajeLocal = 0;
		usaVidas = true;
		particulas = gcnew List<Particula^>();
	}

	void Minijuego::clic(int mx, int my) {
		// la mayoria de minijuegos no usa el raton; Urano lo redefine
	}

	bool Minijuego::jugando() { return estadoInterno == 0; }
	bool Minijuego::ganado() { return estadoInterno == 1; }
	bool Minijuego::perdido() { return estadoInterno == 2; }
	String^ Minijuego::getTitulo() { return titulo; }
	String^ Minijuego::getInstrucciones() { return instrucciones; }
	int Minijuego::getPuntajeLocal() { return puntajeLocal; }
	int Minijuego::getPlaneta() { return planeta; }

	void Minijuego::ganar() {
		if (estadoInterno != 0) return;
		estadoInterno = 1;
		Sonido::victoria();
	}

	void Minijuego::perder() {
		if (estadoInterno != 0) return;
		estadoInterno = 2;
		Sonido::derrota();
	}

	void Minijuego::sumar(int cantidad) {
		puntajeLocal += cantidad;
		jugador->sumarPuntaje(cantidad);
	}

	void Minijuego::tictac() {
		ticks++;
		Particulas::actualizar(particulas);
	}

	void Minijuego::dibujarHUD(Graphics^ g) {
		// barra superior translucida
		SolidBrush barra(Color::FromArgb(165, 5, 8, 18));
		g->FillRectangle(%barra, 0.0f, 0.0f, (float)Config::ANCHO, 46.0f);
		Pen linea(Color::FromArgb(120, 90, 200, 255), 1.5f);
		g->DrawLine(%linea, 0.0f, 46.0f, (float)Config::ANCHO, 46.0f);

		Estilos::textoIzquierda(g,
			String::Format(L"{0} · {1}", SistemaSolar::nombre(planeta), titulo),
			Estilos::hud, Color::FromArgb(150, 220, 255), 14.0f, 23.0f);

		Estilos::textoCentrado(g, progreso(), Estilos::normalNegrita, Color::White, 560.0f, 23.0f);

		Estilos::textoIzquierda(g,
			String::Format(L"Puntos: {0}", puntajeLocal),
			Estilos::hud, Color::FromArgb(255, 220, 120), 700.0f, 23.0f);

		if (usaVidas) dibujarCorazones(g);
	}

	void Minijuego::dibujarCorazones(Graphics^ g) {
		int vidas = jugador->getVidas();
		for (int i = 0; i < 3; i++) {
			float cx = 905.0f + i * 30.0f;
			float cy = 21.0f;
			Color color = (i < vidas) ? Color::FromArgb(235, 70, 80) : Color::FromArgb(70, 75, 95);
			SolidBrush pincel(color);
			g->FillEllipse(%pincel, cx - 9.5f, cy - 7.0f, 10.0f, 10.0f);
			g->FillEllipse(%pincel, cx - 0.5f, cy - 7.0f, 10.0f, 10.0f);
			array<PointF>^ punta = gcnew array<PointF>(3);
			punta[0] = PointF(cx - 9.0f, cy - 0.5f);
			punta[1] = PointF(cx + 9.0f, cy - 0.5f);
			punta[2] = PointF(cx, cy + 10.0f);
			g->FillPolygon(%pincel, punta);
		}
	}
}
