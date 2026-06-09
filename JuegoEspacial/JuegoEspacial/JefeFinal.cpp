#include "JefeFinal.h"
#include "Azar.h"
#include "Estilos.h"
#include "Config.h"

namespace JuegoEspacial {

	JefeFinal::JefeFinal(float px, float py, int pvida) : Enemigo(px, py, pvida) {
		vidaMax = pvida;
		velocidadX = 2.2f;
		objetivoX = px;
		objetivoY = py + 300.0f;
		giro = 0.0f;
		ticks = 0;
	}

	bool JefeFinal::enfurecido() {
		return vida <= vidaMax / 3;
	}

	int JefeFinal::getVidaMax() {
		return vidaMax;
	}

	void JefeFinal::setObjetivo(float px, float py) {
		objetivoX = px;
		objetivoY = py;
	}

	void JefeFinal::mover() {
		ticks++;
		giro += enfurecido() ? 2.6f : 1.2f;
		x += velocidadX * (enfurecido() ? 1.7f : 1.0f);
		if (x < 130.0f && velocidadX < 0) velocidadX = -velocidadX;
		if (x > Config::ANCHO - 130.0f && velocidadX > 0) velocidadX = -velocidadX;
		if (destello > 0) destello--;
	}

	void JefeFinal::atacar() {
		// resplandor de carga: avisa al jugador que viene un ataque
		destello = 22;
	}

	Proyectil^ JefeFinal::dispararRayo(float pobjetivoX, float pobjetivoY) {
		float origenY = y + 32.0f;
		float dx = pobjetivoX - x;
		float dy = pobjetivoY - origenY;
		float largo = (float)Math::Sqrt(dx * dx + dy * dy);
		if (largo < 1.0f) largo = 1.0f;
		float velocidad = enfurecido() ? 8.2f : 6.4f;
		return gcnew Proyectil(x, origenY, dx / largo * velocidad, dy / largo * velocidad, Proyectil::RAYO);
	}

	Asteroide^ JefeFinal::lanzarAsteroide() {
		return gcnew Asteroide(x, y + 26.0f,
			Azar::entreF(-2.6f, 2.6f), Azar::entreF(2.2f, 3.6f),
			Azar::entreF(14.0f, 24.0f), Color::FromArgb(150, 70, 60));
	}

	RectangleF JefeFinal::zona() {
		return RectangleF(x - 52.0f, y - 26.0f, 104.0f, 58.0f);
	}

	void JefeFinal::dibujar(Graphics^ g) {
		float yv = y + (float)Math::Sin(ticks * 0.05) * 6.0f;
		Color colorPuas = enfurecido() ? Color::FromArgb(170, 40, 50) : Color::FromArgb(80, 50, 120);

		// aura de furia
		if (enfurecido()) {
			int alfa = 40 + (int)(25.0 * Math::Sin(ticks * 0.2));
			SolidBrush aura(Color::FromArgb(alfa, 255, 60, 60));
			g->FillEllipse(%aura, x - 80.0f, yv - 60.0f, 160.0f, 120.0f);
		}

		// corona de puas giratorias
		SolidBrush pua(colorPuas);
		for (int i = 0; i < 10; i++) {
			double a = (giro + i * 36.0) * Math::PI / 180.0;
			double a1 = a - 0.16;
			double a2 = a + 0.16;
			array<PointF>^ triangulo = gcnew array<PointF>(3);
			triangulo[0] = PointF(x + (float)Math::Cos(a) * 62.0f, yv + (float)(Math::Sin(a) * 62.0 * 0.55));
			triangulo[1] = PointF(x + (float)Math::Cos(a1) * 38.0f, yv + (float)(Math::Sin(a1) * 38.0 * 0.55));
			triangulo[2] = PointF(x + (float)Math::Cos(a2) * 38.0f, yv + (float)(Math::Sin(a2) * 38.0 * 0.55));
			g->FillPolygon(%pua, triangulo);
		}

		// platillo principal
		SolidBrush platillo(Color::FromArgb(58, 35, 85));
		g->FillEllipse(%platillo, x - 52.0f, yv - 20.0f, 104.0f, 42.0f);
		Pen filo(Color::FromArgb(120, 90, 170), 2.0f);
		g->DrawEllipse(%filo, x - 52.0f, yv - 20.0f, 104.0f, 42.0f);

		// cupula superior translucida
		SolidBrush cupula(Color::FromArgb(150, 130, 90, 200));
		g->FillPie(%cupula, x - 30.0f, yv - 40.0f, 60.0f, 52.0f, 180.0f, 180.0f);

		// luces inferiores que parpadean
		for (int i = 0; i < 5; i++) {
			bool encendida = ((ticks / 9) + i) % 2 == 0;
			SolidBrush luz(encendida ? Color::FromArgb(255, 230, 110) : Color::FromArgb(90, 90, 110));
			g->FillEllipse(%luz, x - 38.0f + i * 18.0f, yv + 10.0f, 8.0f, 8.0f);
		}

		// ojo central que sigue al jugador
		SolidBrush esclera(Color::FromArgb(245, 230, 240));
		g->FillEllipse(%esclera, x - 14.0f, yv - 16.0f, 28.0f, 28.0f);
		float dx = objetivoX - x;
		float dy = objetivoY - yv;
		float largo = (float)Math::Sqrt(dx * dx + dy * dy);
		if (largo < 1.0f) largo = 1.0f;
		float pupilaX = x + dx / largo * 5.0f;
		float pupilaY = yv - 2.0f + dy / largo * 5.0f;
		SolidBrush pupila(enfurecido() ? Color::FromArgb(255, 40, 40) : Color::FromArgb(180, 30, 60));
		g->FillEllipse(%pupila, pupilaX - 6.5f, pupilaY - 6.5f, 13.0f, 13.0f);
		SolidBrush destelloOjo(Color::FromArgb(220, 255, 255, 255));
		g->FillEllipse(%destelloOjo, pupilaX - 4.5f, pupilaY - 5.0f, 4.0f, 4.0f);

		// carga de energia antes de atacar
		if (destello > 0) {
			int alfa = Math::Min(220, destello * 11);
			Pen aviso(Color::FromArgb(alfa, 255, 120, 240), 2.0f);
			g->DrawLine(%aviso, x, yv + 26.0f, objetivoX, objetivoY);
			SolidBrush carga(Color::FromArgb(alfa, 255, 160, 250));
			g->FillEllipse(%carga, x - 9.0f, yv + 20.0f, 18.0f, 18.0f);
		}
	}

	void JefeFinal::dibujarBarraVida(Graphics^ g) {
		float bx = 260.0f, by = 56.0f, bancho = 480.0f, balto = 16.0f;
		Estilos::rellenarRedondeado(g, Color::FromArgb(180, 20, 20, 35), bx - 6.0f, by - 4.0f, bancho + 12.0f, balto + 8.0f, 8.0f);
		float proporcion = (float)Math::Max(0, vida) / vidaMax;
		Color relleno = enfurecido() ? Color::FromArgb(255, 70, 70) : Color::FromArgb(200, 60, 200);
		SolidBrush barra(relleno);
		g->FillRectangle(%barra, bx, by, bancho * proporcion, balto);
		Pen marco(Color::FromArgb(220, 220, 240), 1.5f);
		g->DrawRectangle(%marco, bx, by, bancho, balto);
		Estilos::textoCentrado(g, L"JEFE FINAL", Estilos::pequena, Color::White, bx + bancho / 2.0f, by + balto / 2.0f);
	}
}
