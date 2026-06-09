#include "Superficie.h"
#include "Config.h"
#include "Azar.h"
#include "Fondo.h"
#include "SistemaSolar.h"

using namespace System::Drawing::Drawing2D;

namespace JuegoEspacial {

	Color Superficie::colorCieloArriba(int planeta) {
		switch (planeta) {
		case 0: return Color::FromArgb(12, 8, 8);        // Mercurio: negro calido
		case 1: return Color::FromArgb(95, 70, 18);      // Venus: bruma mostaza
		case 2: return Color::FromArgb(5, 8, 22);        // Tierra: espacio en orbita
		case 3: return Color::FromArgb(150, 75, 50);     // Marte: cielo salmon
		case 4: return Color::FromArgb(120, 80, 50);     // Jupiter: nubes gigantes
		case 5: return Color::FromArgb(8, 8, 20);        // Saturno: luna rocosa
		case 6: return Color::FromArgb(8, 25, 45);       // Urano: caverna helada
		default: return Color::FromArgb(8, 12, 42);      // Neptuno: tormenta azul
		}
	}

	Color Superficie::colorCieloAbajo(int planeta) {
		switch (planeta) {
		case 0: return Color::FromArgb(70, 35, 16);
		case 1: return Color::FromArgb(165, 125, 45);
		case 2: return Color::FromArgb(12, 22, 55);
		case 3: return Color::FromArgb(215, 130, 85);
		case 4: return Color::FromArgb(185, 135, 90);
		case 5: return Color::FromArgb(26, 22, 48);
		case 6: return Color::FromArgb(25, 75, 105);
		default: return Color::FromArgb(25, 35, 95);
		}
	}

	Color Superficie::colorSuelo(int planeta) {
		switch (planeta) {
		case 0: return Color::FromArgb(110, 100, 92);
		case 1: return Color::FromArgb(155, 115, 58);
		case 2: return Color::FromArgb(95, 105, 120);    // plataforma metalica
		case 3: return Color::FromArgb(170, 85, 48);
		case 4: return Color::FromArgb(205, 195, 175);   // cubierta de nubes
		case 5: return Color::FromArgb(125, 120, 108);
		case 6: return Color::FromArgb(140, 200, 220);
		default: return Color::FromArgb(58, 88, 160);
		}
	}

	Color Superficie::colorSueloOscuro(int planeta) {
		switch (planeta) {
		case 0: return Color::FromArgb(75, 67, 60);
		case 1: return Color::FromArgb(115, 84, 42);
		case 2: return Color::FromArgb(65, 72, 85);
		case 3: return Color::FromArgb(125, 60, 33);
		case 4: return Color::FromArgb(160, 150, 132);
		case 5: return Color::FromArgb(88, 84, 75);
		case 6: return Color::FromArgb(100, 160, 185);
		default: return Color::FromArgb(40, 62, 120);
		}
	}

	void Superficie::asegurarDetalles() {
		if (rocasX != nullptr) return;
		rocasX = gcnew array<float>(14);
		rocasTam = gcnew array<float>(14);
		for (int i = 0; i < 14; i++) {
			rocasX[i] = Azar::entreF(15.0f, 985.0f);
			rocasTam[i] = Azar::entreF(6.0f, 18.0f);
		}
	}

	void Superficie::dibujarFondo(Graphics^ g, int planeta, int tick) {
		asegurarDetalles();

		// cielo
		LinearGradientBrush cielo(
			RectangleF(0.0f, 0.0f, (float)Config::ANCHO, (float)Config::ALTO),
			colorCieloArriba(planeta), colorCieloAbajo(planeta), 90.0f);
		g->FillRectangle(%cielo, 0.0f, 0.0f, (float)Config::ANCHO, (float)Config::ALTO);

		// estrellas tenues (solo donde el cielo es oscuro)
		if (planeta == 0 || planeta == 2 || planeta == 5 || planeta == 7 || planeta == 6) {
			Fondo::dibujarEstrellas(g, tick, 130);
		}

		dibujarDecoracion(g, planeta, tick);
		dibujarSuelo(g, planeta, tick);
	}

	void Superficie::dibujarDecoracion(Graphics^ g, int planeta, int tick) {
		float suelo = Config::SUELO;

		if (planeta == 0) {
			// sol gigante asomando
			for (int capa = 3; capa >= 1; capa--) {
				SolidBrush halo(Color::FromArgb(20 * capa, 255, 150, 40));
				float r = 95.0f + capa * 30.0f;
				g->FillEllipse(%halo, 130.0f - r, 120.0f - r, r * 2.0f, r * 2.0f);
			}
			SolidBrush sol(Color::FromArgb(255, 185, 60));
			g->FillEllipse(%sol, 35.0f, 25.0f, 190.0f, 190.0f);
			SolidBrush nucleo(Color::FromArgb(255, 235, 140));
			g->FillEllipse(%nucleo, 65.0f, 55.0f, 130.0f, 130.0f);
		}
		else if (planeta == 1) {
			// nubes acidas que se desplazan
			for (int i = 0; i < 5; i++) {
				float nx = ((tick * 4 + i * 2300) % 12400) / 10.0f - 120.0f;
				float ny = 90.0f + i * 70.0f;
				SolidBrush nube(Color::FromArgb(70, 220, 200, 90));
				g->FillEllipse(%nube, nx, ny, 190.0f, 38.0f);
				g->FillEllipse(%nube, nx + 60.0f, ny - 14.0f, 130.0f, 34.0f);
			}
		}
		else if (planeta == 2) {
			// la Tierra asoma tras el horizonte de la plataforma
			SistemaSolar::dibujarPlaneta(g, 2, 760.0f, suelo + 690.0f, 780.0f, tick);
			Pen atmosfera(Color::FromArgb(90, 120, 220, 255), 5.0f);
			g->DrawArc(%atmosfera, 760.0f - 800.0f, suelo + 690.0f - 800.0f, 1600.0f, 1600.0f, 235.0f, 70.0f);
		}
		else if (planeta == 3) {
			// montanas lejanas y dos lunas
			SolidBrush luna(Color::FromArgb(205, 180, 160));
			g->FillEllipse(%luna, 180.0f, 90.0f, 26.0f, 26.0f);
			g->FillEllipse(%luna, 760.0f, 60.0f, 16.0f, 16.0f);
			SolidBrush montana(Color::FromArgb(135, 65, 38));
			array<PointF>^ cordillera = gcnew array<PointF>(7);
			cordillera[0] = PointF(0.0f, suelo);
			cordillera[1] = PointF(140.0f, suelo - 110.0f);
			cordillera[2] = PointF(310.0f, suelo - 30.0f);
			cordillera[3] = PointF(520.0f, suelo - 140.0f);
			cordillera[4] = PointF(700.0f, suelo - 50.0f);
			cordillera[5] = PointF(880.0f, suelo - 120.0f);
			cordillera[6] = PointF(1000.0f, suelo);
			g->FillPolygon(%montana, cordillera);
		}
		else if (planeta == 4) {
			// bandas de nubes gigantes y la gran mancha roja
			SolidBrush banda1(Color::FromArgb(60, 90, 55, 35));
			SolidBrush banda2(Color::FromArgb(50, 235, 210, 170));
			g->FillRectangle(%banda1, 0.0f, 80.0f, 1000.0f, 70.0f);
			g->FillRectangle(%banda2, 0.0f, 190.0f, 1000.0f, 55.0f);
			g->FillRectangle(%banda1, 0.0f, 300.0f, 1000.0f, 80.0f);
			SolidBrush mancha(Color::FromArgb(120, 200, 80, 55));
			g->FillEllipse(%mancha, 640.0f, 150.0f, 230.0f, 90.0f);
		}
		else if (planeta == 5) {
			// Saturno enorme en el cielo de su luna
			SistemaSolar::dibujarPlaneta(g, 5, 790.0f, 170.0f, 95.0f, tick);
		}
		else if (planeta == 6) {
			// estalactitas de hielo colgando
			SolidBrush hielo(Color::FromArgb(170, 120, 200, 230));
			for (int i = 0; i < 9; i++) {
				float ex = 40.0f + i * 115.0f;
				float alto = 60.0f + (float)Math::Abs(Math::Sin(i * 2.7)) * 70.0f;
				array<PointF>^ punta = gcnew array<PointF>(3);
				punta[0] = PointF(ex - 22.0f, 0.0f);
				punta[1] = PointF(ex + 22.0f, 0.0f);
				punta[2] = PointF(ex, alto);
				g->FillPolygon(%hielo, punta);
			}
		}
		else if (planeta == 7) {
			// rafagas de viento helado
			for (int i = 0; i < 6; i++) {
				float vx = (float)((tick * 3 + i * 180) % 1200) - 100.0f;
				float vy = 90.0f + i * 75.0f;
				Pen viento(Color::FromArgb(90, 200, 220, 255), 2.0f);
				g->DrawLine(%viento, vx, vy, vx + 80.0f, vy);
				g->DrawLine(%viento, vx + 95.0f, vy, vx + 115.0f, vy);
			}
		}
	}

	void Superficie::dibujarSuelo(Graphics^ g, int planeta, int tick) {
		float suelo = Config::SUELO;
		Color cSuelo = colorSuelo(planeta);
		Color cOscuro = colorSueloOscuro(planeta);

		if (planeta == 4) {
			// cubierta de nubes esponjosa
			SolidBrush nube(cSuelo);
			for (int i = 0; i < 12; i++) {
				g->FillEllipse(%nube, -40.0f + i * 90.0f, suelo - 22.0f, 130.0f, 60.0f);
			}
			g->FillRectangle(%nube, 0.0f, suelo + 8.0f, 1000.0f, (float)Config::ALTO - suelo);
			SolidBrush sombraNube(Color::FromArgb(90, cOscuro));
			for (int i = 0; i < 8; i++) {
				g->FillEllipse(%sombraNube, 20.0f + i * 130.0f, suelo + 25.0f, 100.0f, 35.0f);
			}
			return;
		}

		// base del suelo
		SolidBrush base(cSuelo);
		g->FillRectangle(%base, 0.0f, suelo, 1000.0f, (float)Config::ALTO - suelo);
		Pen horizonte(Color::FromArgb(120, 255, 255, 255), 2.0f);
		g->DrawLine(%horizonte, 0.0f, suelo, 1000.0f, suelo);

		if (planeta == 2) {
			// plataforma metalica: lineas de paneles y luces
			Pen lineaPanel(cOscuro, 2.0f);
			for (int i = 0; i <= 10; i++) {
				g->DrawLine(%lineaPanel, i * 100.0f, suelo, i * 100.0f - 40.0f, (float)Config::ALTO);
			}
			g->DrawLine(%lineaPanel, 0.0f, suelo + 45.0f, 1000.0f, suelo + 45.0f);
			for (int i = 0; i < 10; i++) {
				bool encendida = ((tick / 30) + i) % 2 == 0;
				SolidBrush baliza(encendida ? Color::FromArgb(255, 190, 60) : Color::FromArgb(110, 80, 40));
				g->FillEllipse(%baliza, 45.0f + i * 100.0f, suelo + 8.0f, 9.0f, 9.0f);
			}
			return;
		}

		// rocas y crateres para los demas planetas
		SolidBrush roca(cOscuro);
		for (int i = 0; i < 14; i++) {
			float rx = rocasX[i];
			float rt = rocasTam[i];
			if (planeta == 6) {
				// cristales de hielo puntiagudos
				array<PointF>^ cristal = gcnew array<PointF>(3);
				cristal[0] = PointF(rx - rt, suelo + 2.0f);
				cristal[1] = PointF(rx + rt, suelo + 2.0f);
				cristal[2] = PointF(rx, suelo - rt * 2.2f);
				SolidBrush hielo(Color::FromArgb(190, 200, 240, 255));
				g->FillPolygon(%hielo, cristal);
			}
			else {
				array<PointF>^ piedra = gcnew array<PointF>(4);
				piedra[0] = PointF(rx - rt, suelo + 14.0f + rt * 0.4f);
				piedra[1] = PointF(rx - rt * 0.4f, suelo + 8.0f);
				piedra[2] = PointF(rx + rt * 0.5f, suelo + 9.0f);
				piedra[3] = PointF(rx + rt, suelo + 15.0f + rt * 0.4f);
				g->FillPolygon(%roca, piedra);
			}
		}

		// crateres
		Pen bordeCrater(Color::FromArgb(140, cOscuro), 2.0f);
		g->DrawEllipse(%bordeCrater, 130.0f, suelo + 52.0f, 90.0f, 26.0f);
		g->DrawEllipse(%bordeCrater, 600.0f, suelo + 80.0f, 130.0f, 32.0f);
		g->DrawEllipse(%bordeCrater, 840.0f, suelo + 40.0f, 70.0f, 20.0f);
	}
}
