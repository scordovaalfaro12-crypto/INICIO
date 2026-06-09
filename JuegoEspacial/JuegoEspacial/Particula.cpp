#include "Particula.h"
#include "Azar.h"

namespace JuegoEspacial {

	Particula::Particula(float px, float py, float pvx, float pvy, float pradio, int pvida, Color pcolor, bool pgravedad) {
		x = px; y = py;
		vx = pvx; vy = pvy;
		radio = pradio;
		vida = pvida; vidaMax = pvida;
		color = pcolor;
		conGravedad = pgravedad;
	}

	void Particula::mover() {
		x += vx;
		y += vy;
		if (conGravedad) vy += 0.15f;
		vida--;
	}

	bool Particula::muerta() {
		return vida <= 0;
	}

	void Particula::dibujar(Graphics^ g) {
		if (vida <= 0) return;
		int alfa = (int)(255.0f * vida / vidaMax);
		if (alfa < 0) alfa = 0;
		if (alfa > 255) alfa = 255;
		float r = radio * (0.4f + 0.6f * vida / vidaMax);
		SolidBrush pincel(Color::FromArgb(alfa, color));
		g->FillEllipse(%pincel, x - r, y - r, r * 2.0f, r * 2.0f);
	}

	void Particulas::actualizar(List<Particula^>^ lista) {
		for (int i = lista->Count - 1; i >= 0; i--) {
			lista[i]->mover();
			if (lista[i]->muerta()) lista->RemoveAt(i);
		}
	}

	void Particulas::dibujar(Graphics^ g, List<Particula^>^ lista) {
		for each (Particula^ p in lista) {
			p->dibujar(g);
		}
	}

	void Particulas::explosion(List<Particula^>^ lista, float x, float y, Color color, int cantidad) {
		for (int i = 0; i < cantidad; i++) {
			float angulo = Azar::entreF(0.0f, 6.2832f);
			float velocidad = Azar::entreF(1.0f, 4.5f);
			lista->Add(gcnew Particula(x, y,
				(float)Math::Cos(angulo) * velocidad,
				(float)Math::Sin(angulo) * velocidad,
				Azar::entreF(2.0f, 5.0f), Azar::entre(20, 45), color, false));
		}
	}

	void Particulas::polvo(List<Particula^>^ lista, float x, float y, Color color, int cantidad) {
		for (int i = 0; i < cantidad; i++) {
			lista->Add(gcnew Particula(
				x + Azar::entreF(-22.0f, 22.0f), y + Azar::entreF(-4.0f, 2.0f),
				Azar::entreF(-1.8f, 1.8f), Azar::entreF(-1.4f, -0.2f),
				Azar::entreF(3.0f, 7.0f), Azar::entre(30, 60), color, false));
		}
	}

	void Particulas::llama(List<Particula^>^ lista, float x, float y, float fuerza) {
		lista->Add(gcnew Particula(
			x + Azar::entreF(-4.0f, 4.0f), y,
			Azar::entreF(-0.6f, 0.6f), Azar::entreF(1.5f, 3.0f) * fuerza,
			Azar::entreF(3.0f, 6.0f), Azar::entre(12, 26),
			(Azar::moneda() ? Color::FromArgb(255, 180, 60) : Color::FromArgb(255, 230, 120)),
			false));
	}
}
