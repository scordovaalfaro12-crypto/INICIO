#include "SistemaSolar.h"
#include "Fondo.h"
#include "Config.h"
#include "Estilos.h"

using namespace System::Drawing::Drawing2D;

namespace JuegoEspacial {

	SistemaSolar::SistemaSolar() {
		px = gcnew array<float> { 150.0f, 255.0f, 360.0f, 465.0f, 585.0f, 715.0f, 835.0f, 930.0f };
		py = gcnew array<float> { 300.0f, 420.0f, 280.0f, 430.0f, 330.0f, 460.0f, 280.0f, 410.0f };
		radios = gcnew array<float> { 16.0f, 24.0f, 26.0f, 20.0f, 46.0f, 36.0f, 27.0f, 27.0f };
		marcado = -1;
	}

	String^ SistemaSolar::nombre(int indice) {
		array<String^>^ nombres = gcnew array<String^> {
			L"MERCURIO", L"VENUS", L"TIERRA", L"MARTE",
			L"JÚPITER", L"SATURNO", L"URANO", L"NEPTUNO"
		};
		return nombres[indice];
	}

	String^ SistemaSolar::nombreMision(int indice) {
		array<String^>^ misiones = gcnew array<String^> {
			L"Lluvia de fuego", L"Atrapa estrellas", L"Limpieza orbital", L"Caza de artefactos",
			L"Tormenta eléctrica", L"Carrera de anillos", L"Cristales de memoria", L"EL JEFE FINAL"
		};
		return misiones[indice];
	}

	int SistemaSolar::planetaEn(int mx, int my) {
		for (int i = 0; i < 8; i++) {
			float dx = mx - px[i];
			float dy = my - py[i];
			if (Math::Sqrt(dx * dx + dy * dy) <= radios[i] + 14.0f) return i;
		}
		return -1;
	}

	bool SistemaSolar::moverRaton(int mx, int my) {
		int indice = planetaEn(mx, my);
		if (indice != -1 && indice != marcado) {
			marcado = indice;
			return true;
		}
		return false;
	}

	int SistemaSolar::getMarcado() {
		return marcado;
	}

	void SistemaSolar::marcarConTeclado(int direccion) {
		if (marcado == -1) marcado = 0;
		else marcado = (marcado + direccion + 8) % 8;
	}

	bool SistemaSolar::desbloqueado(int indice, array<bool>^ completados) {
		if (indice != 7) return true;
		// Neptuno (el jefe) se desbloquea al completar los otros 7 planetas
		for (int i = 0; i < 7; i++) {
			if (!completados[i]) return false;
		}
		return true;
	}

	void SistemaSolar::dibujar(Graphics^ g, int tick, array<bool>^ completados, int puntaje, String^ nombreJugador, Color colorJugador) {
		Fondo::dibujar(g, tick);

		// ---- el Sol con rayos giratorios ----
		float solX = -55.0f, solY = 360.0f;
		for (int capa = 3; capa >= 1; capa--) {
			SolidBrush halo(Color::FromArgb(22 * capa, 255, 170, 40));
			float r = 130.0f + capa * 26.0f;
			g->FillEllipse(%halo, solX - r, solY - r, r * 2.0f, r * 2.0f);
		}
		Pen rayo(Color::FromArgb(70, 255, 200, 80), 2.0f);
		for (int i = 0; i < 12; i++) {
			double a = (tick * 0.3 + i * 30.0) * Math::PI / 180.0;
			g->DrawLine(%rayo,
				solX + (float)Math::Cos(a) * 135.0f, solY + (float)Math::Sin(a) * 135.0f,
				solX + (float)Math::Cos(a) * 165.0f, solY + (float)Math::Sin(a) * 165.0f);
		}
		SolidBrush sol(Color::FromArgb(255, 200, 60));
		g->FillEllipse(%sol, solX - 130.0f, solY - 130.0f, 260.0f, 260.0f);
		SolidBrush nucleoSol(Color::FromArgb(255, 240, 150));
		g->FillEllipse(%nucleoSol, solX - 95.0f, solY - 95.0f, 190.0f, 190.0f);

		// ---- orbitas ----
		for (int i = 0; i < 8; i++) {
			float dx = px[i] - solX;
			float dy = py[i] - solY;
			float distancia = (float)Math::Sqrt(dx * dx + dy * dy);
			float anguloPlaneta = (float)(Math::Atan2(dy, dx) * 180.0 / Math::PI);
			Pen orbita(Color::FromArgb(38, 200, 220, 255), 1.0f);
			g->DrawArc(%orbita, solX - distancia, solY - distancia, distancia * 2.0f, distancia * 2.0f,
				anguloPlaneta - 16.0f, 32.0f);
		}

		// ---- titulo ----
		Estilos::textoSombra(g, L"SISTEMA SOLAR", Estilos::enorme, Color::FromArgb(170, 220, 255), 500.0f, 48.0f);
		Estilos::textoCentrado(g, L"Haz clic en un planeta para comenzar su misión", Estilos::normal,
			Color::FromArgb(150, 170, 210), 500.0f, 95.0f);

		// ---- planetas ----
		int completadosTotal = 0;
		for (int i = 0; i < 8; i++) {
			if (completados[i]) completadosTotal++;
		}

		for (int i = 0; i < 8; i++) {
			float bob = (float)Math::Sin(tick * 0.03 + i * 1.3) * 4.0f;
			float cy = py[i] + bob;
			bool abierto = desbloqueado(i, completados);

			dibujarPlaneta(g, i, px[i], cy, radios[i], tick);

			// planeta bloqueado: se ve apagado y con candado
			if (!abierto) {
				SolidBrush velo(Color::FromArgb(150, 10, 10, 20));
				g->FillEllipse(%velo, px[i] - radios[i] - 2.0f, cy - radios[i] - 2.0f, radios[i] * 2.0f + 4.0f, radios[i] * 2.0f + 4.0f);
				SolidBrush cuerpoCandado(Color::FromArgb(220, 220, 230));
				g->FillRectangle(%cuerpoCandado, px[i] - 8.0f, cy - 3.0f, 16.0f, 13.0f);
				Pen arco(Color::FromArgb(220, 220, 230), 3.0f);
				g->DrawArc(%arco, px[i] - 6.0f, cy - 14.0f, 12.0f, 14.0f, 180.0f, 180.0f);
				SolidBrush ojoCerradura(Color::FromArgb(60, 60, 80));
				g->FillEllipse(%ojoCerradura, px[i] - 2.5f, cy + 0.5f, 5.0f, 5.0f);
			}

			// insignia de mision completada
			if (completados[i]) {
				float bx = px[i] + radios[i] * 0.75f;
				float by = cy - radios[i] * 0.75f - 6.0f;
				SolidBrush insignia(Color::FromArgb(60, 190, 90));
				g->FillEllipse(%insignia, bx - 10.0f, by - 10.0f, 20.0f, 20.0f);
				Pen palomita(Color::White, 3.0f);
				g->DrawLine(%palomita, bx - 5.0f, by, bx - 1.0f, by + 5.0f);
				g->DrawLine(%palomita, bx - 1.0f, by + 5.0f, bx + 6.0f, by - 4.0f);
			}

			// anillo pulsante sobre el planeta marcado
			if (marcado == i) {
				int alfa = 150 + (int)(90.0 * Math::Sin(tick * 0.15));
				Pen seleccion(Color::FromArgb(alfa, 90, 230, 255), 3.0f);
				float r = radios[i] + 11.0f;
				g->DrawEllipse(%seleccion, px[i] - r, cy - r, r * 2.0f, r * 2.0f);
			}

			// nombre y mision
			Estilos::textoCentrado(g, nombre(i), Estilos::normalNegrita, Color::White, px[i], cy + radios[i] + 20.0f);
			Estilos::textoCentrado(g, nombreMision(i), Estilos::pequena,
				(i == 7 ? Color::FromArgb(255, 130, 130) : Color::FromArgb(150, 175, 215)),
				px[i], cy + radios[i] + 38.0f);
		}

		// ---- panel del jugador ----
		Estilos::rellenarRedondeado(g, Color::FromArgb(170, 10, 14, 30), 712.0f, 60.0f, 276.0f, 86.0f, 12.0f);
		Estilos::bordeRedondeado(g, Color::FromArgb(120, 120, 200, 255), 1.5f, 712.0f, 60.0f, 276.0f, 86.0f, 12.0f);
		// mini casco del color del jugador
		SolidBrush miniCasco(colorJugador);
		g->FillEllipse(%miniCasco, 728.0f, 76.0f, 26.0f, 26.0f);
		SolidBrush miniVisor(Color::FromArgb(25, 35, 70));
		g->FillEllipse(%miniVisor, 735.0f, 83.0f, 16.0f, 13.0f);
		Estilos::textoIzquierda(g, String::Format(L"ASTRONAUTA {0}", nombreJugador), Estilos::normalNegrita, Color::White, 766.0f, 88.0f);
		Estilos::textoIzquierda(g, String::Format(L"Puntaje: {0}   Misiones: {1}/8", puntaje, completadosTotal),
			Estilos::normal, Color::FromArgb(160, 200, 240), 728.0f, 124.0f);

		// ---- panel de informacion del planeta marcado ----
		if (marcado != -1) {
			bool abierto = desbloqueado(marcado, completados);
			Estilos::rellenarRedondeado(g, Color::FromArgb(200, 12, 18, 40), 290.0f, 596.0f, 420.0f, 80.0f, 12.0f);
			Estilos::bordeRedondeado(g, Color::FromArgb(200, 90, 230, 255), 2.0f, 290.0f, 596.0f, 420.0f, 80.0f, 12.0f);
			Estilos::textoCentrado(g, String::Format(L"{0} — {1}", nombre(marcado), nombreMision(marcado)),
				Estilos::normalNegrita, Color::White, 500.0f, 618.0f);
			if (abierto) {
				String^ extra = completados[marcado] ? L"COMPLETADO — puedes repetir la misión (CLIC o ENTER)" : L"CLIC o ENTER para aterrizar";
				Estilos::textoCentrado(g, extra, Estilos::pequena, Color::FromArgb(150, 220, 170), 500.0f, 650.0f);
			}
			else {
				Estilos::textoCentrado(g, L"BLOQUEADO: completa los otros 7 planetas", Estilos::pequena,
					Color::FromArgb(255, 140, 140), 500.0f, 650.0f);
			}
		}

		Estilos::textoIzquierda(g, L"ESC → menú", Estilos::pequena, Color::FromArgb(120, 140, 180), 16.0f, 678.0f);
	}

	void SistemaSolar::dibujarPlaneta(Graphics^ g, int indice, float cx, float cy, float r, int tick) {
		// anillo trasero de Saturno
		if (indice == 5) {
			g->TranslateTransform(cx, cy);
			g->RotateTransform(-18.0f);
			Pen anilloTras(Color::FromArgb(200, 215, 190, 140), r * 0.20f);
			g->DrawArc(%anilloTras, -r * 1.9f, -r * 0.62f, r * 3.8f, r * 1.24f, 180.0f, 180.0f);
			g->ResetTransform();
		}

		// halo suave
		SolidBrush halo(Color::FromArgb(35, 200, 220, 255));
		g->FillEllipse(%halo, cx - r * 1.22f, cy - r * 1.22f, r * 2.44f, r * 2.44f);

		// recorte circular para los detalles internos
		GraphicsPath recorte;
		recorte.AddEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);

		if (indice == 0) {
			// MERCURIO: roca gris con crateres
			SolidBrush base(Color::FromArgb(150, 140, 130));
			g->FillEllipse(%base, cx - r, cy - r, r * 2.0f, r * 2.0f);
			g->SetClip(%recorte);
			SolidBrush crater(Color::FromArgb(110, 100, 92));
			g->FillEllipse(%crater, cx - r * 0.5f, cy - r * 0.4f, r * 0.5f, r * 0.45f);
			g->FillEllipse(%crater, cx + r * 0.1f, cy + r * 0.15f, r * 0.4f, r * 0.35f);
			g->FillEllipse(%crater, cx - r * 0.15f, cy + r * 0.45f, r * 0.3f, r * 0.25f);
			g->ResetClip();
		}
		else if (indice == 1) {
			// VENUS: nubes densas color crema
			SolidBrush base(Color::FromArgb(218, 180, 98));
			g->FillEllipse(%base, cx - r, cy - r, r * 2.0f, r * 2.0f);
			g->SetClip(%recorte);
			Pen banda(Color::FromArgb(120, 240, 215, 150), r * 0.16f);
			g->DrawArc(%banda, cx - r * 1.1f, cy - r * 0.9f, r * 2.2f, r * 1.2f, 20.0f, 140.0f);
			g->DrawArc(%banda, cx - r * 1.05f, cy - r * 0.1f, r * 2.1f, r * 1.1f, 200.0f, 140.0f);
			g->ResetClip();
		}
		else if (indice == 2) {
			// TIERRA: oceanos, continentes y nubes
			SolidBrush base(Color::FromArgb(52, 110, 200));
			g->FillEllipse(%base, cx - r, cy - r, r * 2.0f, r * 2.0f);
			g->SetClip(%recorte);
			SolidBrush continente(Color::FromArgb(80, 170, 90));
			array<PointF>^ tierra1 = gcnew array<PointF>(5);
			tierra1[0] = PointF(cx - r * 0.7f, cy - r * 0.4f);
			tierra1[1] = PointF(cx - r * 0.1f, cy - r * 0.6f);
			tierra1[2] = PointF(cx + r * 0.1f, cy - r * 0.2f);
			tierra1[3] = PointF(cx - r * 0.25f, cy + r * 0.1f);
			tierra1[4] = PointF(cx - r * 0.65f, cy);
			g->FillClosedCurve(%continente, tierra1);
			array<PointF>^ tierra2 = gcnew array<PointF>(4);
			tierra2[0] = PointF(cx + r * 0.25f, cy + r * 0.15f);
			tierra2[1] = PointF(cx + r * 0.75f, cy + r * 0.05f);
			tierra2[2] = PointF(cx + r * 0.6f, cy + r * 0.55f);
			tierra2[3] = PointF(cx + r * 0.2f, cy + r * 0.6f);
			g->FillClosedCurve(%continente, tierra2);
			SolidBrush nube(Color::FromArgb(120, 255, 255, 255));
			g->FillEllipse(%nube, cx - r * 0.5f, cy + r * 0.3f, r * 0.9f, r * 0.3f);
			g->FillEllipse(%nube, cx - r * 0.2f, cy - r * 0.85f, r * 0.8f, r * 0.28f);
			g->ResetClip();
			// luna orbitando
			double a = tick * 0.02;
			float lunaX = cx + (float)Math::Cos(a) * r * 1.7f;
			float lunaY = cy + (float)Math::Sin(a) * r * 0.6f;
			SolidBrush luna(Color::FromArgb(200, 200, 205));
			g->FillEllipse(%luna, lunaX - r * 0.18f, lunaY - r * 0.18f, r * 0.36f, r * 0.36f);
		}
		else if (indice == 3) {
			// MARTE: rojo con casquete polar
			SolidBrush base(Color::FromArgb(200, 95, 55));
			g->FillEllipse(%base, cx - r, cy - r, r * 2.0f, r * 2.0f);
			g->SetClip(%recorte);
			SolidBrush mancha(Color::FromArgb(160, 70, 40));
			g->FillEllipse(%mancha, cx - r * 0.6f, cy + r * 0.05f, r * 0.7f, r * 0.5f);
			g->FillEllipse(%mancha, cx + r * 0.15f, cy - r * 0.35f, r * 0.55f, r * 0.4f);
			SolidBrush polo(Color::FromArgb(230, 240, 245));
			g->FillEllipse(%polo, cx - r * 0.4f, cy - r * 1.05f, r * 0.8f, r * 0.45f);
			g->ResetClip();
		}
		else if (indice == 4) {
			// JUPITER: bandas y la gran mancha roja
			SolidBrush base(Color::FromArgb(205, 170, 125));
			g->FillEllipse(%base, cx - r, cy - r, r * 2.0f, r * 2.0f);
			g->SetClip(%recorte);
			SolidBrush banda1(Color::FromArgb(150, 175, 130, 85));
			SolidBrush banda2(Color::FromArgb(120, 230, 205, 165));
			g->FillRectangle(%banda1, cx - r, cy - r * 0.75f, r * 2.0f, r * 0.28f);
			g->FillRectangle(%banda2, cx - r, cy - r * 0.35f, r * 2.0f, r * 0.22f);
			g->FillRectangle(%banda1, cx - r, cy - r * 0.02f, r * 2.0f, r * 0.30f);
			g->FillRectangle(%banda2, cx - r, cy + r * 0.42f, r * 2.0f, r * 0.22f);
			SolidBrush manchaRoja(Color::FromArgb(195, 85, 60));
			g->FillEllipse(%manchaRoja, cx + r * 0.12f, cy + r * 0.05f, r * 0.5f, r * 0.3f);
			g->ResetClip();
		}
		else if (indice == 5) {
			// SATURNO: dorado palido con bandas
			SolidBrush base(Color::FromArgb(216, 186, 120));
			g->FillEllipse(%base, cx - r, cy - r, r * 2.0f, r * 2.0f);
			g->SetClip(%recorte);
			SolidBrush banda(Color::FromArgb(110, 190, 155, 95));
			g->FillRectangle(%banda, cx - r, cy - r * 0.5f, r * 2.0f, r * 0.25f);
			g->FillRectangle(%banda, cx - r, cy + r * 0.1f, r * 2.0f, r * 0.3f);
			g->ResetClip();
		}
		else if (indice == 6) {
			// URANO: turquesa helado con anillo casi vertical
			SolidBrush base(Color::FromArgb(130, 212, 225));
			g->FillEllipse(%base, cx - r, cy - r, r * 2.0f, r * 2.0f);
			g->SetClip(%recorte);
			SolidBrush bruma(Color::FromArgb(70, 255, 255, 255));
			g->FillEllipse(%bruma, cx - r * 0.8f, cy - r * 0.7f, r * 1.2f, r * 0.5f);
			g->ResetClip();
			g->TranslateTransform(cx, cy);
			g->RotateTransform(75.0f);
			Pen anilloFino(Color::FromArgb(120, 220, 240, 250), 2.0f);
			g->DrawEllipse(%anilloFino, -r * 1.55f, -r * 0.4f, r * 3.1f, r * 0.8f);
			g->ResetTransform();
		}
		else {
			// NEPTUNO: azul profundo con tormenta oscura
			SolidBrush base(Color::FromArgb(45, 75, 200));
			g->FillEllipse(%base, cx - r, cy - r, r * 2.0f, r * 2.0f);
			g->SetClip(%recorte);
			SolidBrush tormenta(Color::FromArgb(25, 40, 120));
			g->FillEllipse(%tormenta, cx - r * 0.45f, cy - r * 0.2f, r * 0.6f, r * 0.4f);
			Pen viento(Color::FromArgb(130, 220, 235, 255), r * 0.07f);
			g->DrawArc(%viento, cx - r * 0.9f, cy + r * 0.25f, r * 1.8f, r * 0.5f, 10.0f, 160.0f);
			g->ResetClip();
		}

		// sombra (lado contrario al sol)
		g->SetClip(%recorte);
		SolidBrush sombra(Color::FromArgb(80, 0, 0, 25));
		g->FillEllipse(%sombra, cx - r + r * 0.45f, cy - r + r * 0.35f, r * 2.0f, r * 2.0f);
		g->ResetClip();

		// anillo delantero de Saturno
		if (indice == 5) {
			g->TranslateTransform(cx, cy);
			g->RotateTransform(-18.0f);
			Pen anilloDel(Color::FromArgb(230, 225, 200, 150), r * 0.20f);
			g->DrawArc(%anilloDel, -r * 1.9f, -r * 0.62f, r * 3.8f, r * 1.24f, 0.0f, 180.0f);
			Pen surco(Color::FromArgb(160, 150, 125, 85), r * 0.05f);
			g->DrawArc(%surco, -r * 1.65f, -r * 0.5f, r * 3.3f, r * 1.0f, 0.0f, 180.0f);
			g->ResetTransform();
		}
	}
}
