#include "Nave.h"
#include "Azar.h"
#include "Sonido.h"
#include "Estilos.h"

namespace JuegoEspacial {

	Nave::Nave(float px, float py) : EntidadEspacial(px, py) {
		integridad = 100;
		velocidad = 0.0f;
		inclinacion = 0.0f;
		llama = false;
		patas = true;
		faseLlama = 0.0f;
		parpadeoDanio = 0;
	}

	void Nave::mover() {
		faseLlama += 0.5f;
		if (parpadeoDanio > 0) parpadeoDanio--;
	}

	void Nave::recibirDanio(int cantidad) {
		integridad -= cantidad;
		if (integridad < 0) integridad = 0;
		parpadeoDanio = 20;
		Sonido::danio();
	}

	void Nave::reparar(int cantidad) {
		integridad += cantidad;
		if (integridad > 100) integridad = 100;
	}

	int Nave::getIntegridad() { return integridad; }
	float Nave::getVelocidad() { return velocidad; }
	void Nave::setVelocidad(float v) { velocidad = v; }
	void Nave::setLlama(bool encendida) { llama = encendida; }
	void Nave::setPatas(bool visibles) { patas = visibles; }
	void Nave::setInclinacion(float grados) { inclinacion = grados; }

	bool Nave::destruida() {
		return integridad <= 0;
	}

	RectangleF Nave::zona() {
		// volando horizontal: la nave ocupa desde su base (x) hasta la punta (x + 112)
		return RectangleF(x + 10.0f, y - 15.0f, 94.0f, 30.0f);
	}

	void Nave::dibujar(Graphics^ g) {
		g->TranslateTransform(x, y);
		g->RotateTransform(inclinacion);
		dibujarCuerpo(g);
		g->ResetTransform();
	}

	void Nave::dibujarHorizontal(Graphics^ g) {
		g->TranslateTransform(x, y);
		g->RotateTransform(90.0f + inclinacion);
		dibujarCuerpo(g);
		g->ResetTransform();
	}

	void Nave::dibujarCuerpo(Graphics^ g) {
		// patas de aterrizaje
		if (patas) {
			Pen pata(Color::FromArgb(120, 125, 135), 3.5f);
			g->DrawLine(%pata, -13.0f, -26.0f, -24.0f, 0.0f);
			g->DrawLine(%pata, 13.0f, -26.0f, 24.0f, 0.0f);
			SolidBrush pie(Color::FromArgb(90, 95, 105));
			g->FillRectangle(%pie, -29.0f, -3.0f, 10.0f, 4.0f);
			g->FillRectangle(%pie, 19.0f, -3.0f, 10.0f, 4.0f);
		}

		// tobera
		array<PointF>^ tobera = gcnew array<PointF>(4);
		tobera[0] = PointF(-9.0f, -22.0f);
		tobera[1] = PointF(9.0f, -22.0f);
		tobera[2] = PointF(13.0f, -7.0f);
		tobera[3] = PointF(-13.0f, -7.0f);
		SolidBrush metalOscuro(Color::FromArgb(75, 78, 90));
		g->FillPolygon(%metalOscuro, tobera);

		// llama del motor
		if (llama) {
			float flicker = (float)Math::Sin(faseLlama) * 4.0f + Azar::entreF(0.0f, 3.0f);
			array<PointF>^ fuego = gcnew array<PointF>(3);
			fuego[0] = PointF(-9.0f, -8.0f);
			fuego[1] = PointF(9.0f, -8.0f);
			fuego[2] = PointF(0.0f, 26.0f + flicker);
			SolidBrush naranja(Color::FromArgb(230, 255, 150, 40));
			g->FillPolygon(%naranja, fuego);
			fuego[0] = PointF(-5.0f, -8.0f);
			fuego[1] = PointF(5.0f, -8.0f);
			fuego[2] = PointF(0.0f, 15.0f + flicker * 0.6f);
			SolidBrush amarillo(Color::FromArgb(240, 255, 240, 150));
			g->FillPolygon(%amarillo, fuego);
		}

		// cuerpo principal (capsula blanca)
		SolidBrush casco(Color::FromArgb(235, 238, 244));
		g->FillEllipse(%casco, -16.0f, -112.0f, 32.0f, 44.0f);     // nariz redondeada
		g->FillRectangle(%casco, -16.0f, -92.0f, 32.0f, 70.0f);    // fuselaje

		// punta roja
		SolidBrush rojo(Color::FromArgb(205, 60, 60));
		g->FillPie(%rojo, -16.0f, -112.0f, 32.0f, 44.0f, 180.0f, 180.0f);

		// sombreado lateral
		SolidBrush sombra(Color::FromArgb(45, 0, 0, 30));
		g->FillRectangle(%sombra, 7.0f, -90.0f, 9.0f, 66.0f);

		// aletas
		array<PointF>^ aletaIzq = gcnew array<PointF>(3);
		aletaIzq[0] = PointF(-16.0f, -46.0f);
		aletaIzq[1] = PointF(-30.0f, -6.0f);
		aletaIzq[2] = PointF(-16.0f, -16.0f);
		array<PointF>^ aletaDer = gcnew array<PointF>(3);
		aletaDer[0] = PointF(16.0f, -46.0f);
		aletaDer[1] = PointF(30.0f, -6.0f);
		aletaDer[2] = PointF(16.0f, -16.0f);
		g->FillPolygon(%rojo, aletaIzq);
		g->FillPolygon(%rojo, aletaDer);

		// franja decorativa
		g->FillRectangle(%rojo, -16.0f, -52.0f, 32.0f, 7.0f);

		// ventana circular
		SolidBrush marco(Color::FromArgb(150, 155, 168));
		g->FillEllipse(%marco, -10.0f, -78.0f, 20.0f, 20.0f);
		SolidBrush vidrio(Color::FromArgb(70, 160, 210));
		g->FillEllipse(%vidrio, -7.0f, -75.0f, 14.0f, 14.0f);
		SolidBrush reflejo(Color::FromArgb(160, 230, 250, 255));
		g->FillEllipse(%reflejo, -5.0f, -73.0f, 5.0f, 4.0f);

		// remaches
		SolidBrush remache(Color::FromArgb(170, 175, 188));
		g->FillEllipse(%remache, -13.0f, -42.0f, 3.0f, 3.0f);
		g->FillEllipse(%remache, 10.0f, -42.0f, 3.0f, 3.0f);
		g->FillEllipse(%remache, -13.0f, -30.0f, 3.0f, 3.0f);
		g->FillEllipse(%remache, 10.0f, -30.0f, 3.0f, 3.0f);

		// parpadeo rojo al recibir danio
		if (parpadeoDanio > 0 && (parpadeoDanio / 4) % 2 == 0) {
			SolidBrush alerta(Color::FromArgb(90, 255, 40, 40));
			g->FillRectangle(%alerta, -32.0f, -114.0f, 64.0f, 118.0f);
		}
	}

	void Nave::mostrarEstado(Graphics^ g) {
		float px = 12.0f, py = 54.0f;
		Estilos::rellenarRedondeado(g, Color::FromArgb(170, 10, 14, 30), px, py, 225.0f, 64.0f, 10.0f);
		Estilos::bordeRedondeado(g, Color::FromArgb(120, 120, 200, 255), 1.5f, px, py, 225.0f, 64.0f, 10.0f);

		Estilos::textoIzquierda(g, L"NAVE  Casco:", Estilos::hud, Color::FromArgb(170, 220, 255), px + 10.0f, py + 16.0f);

		// barra de integridad
		float bx = px + 118.0f, by = py + 9.0f, bancho = 95.0f, balto = 13.0f;
		SolidBrush fondoBarra(Color::FromArgb(40, 45, 60));
		g->FillRectangle(%fondoBarra, bx, by, bancho, balto);
		Color colorBarra = Color::FromArgb(90, 220, 110);
		if (integridad <= 60) colorBarra = Color::FromArgb(240, 210, 80);
		if (integridad <= 30) colorBarra = Color::FromArgb(235, 80, 70);
		SolidBrush barra(colorBarra);
		g->FillRectangle(%barra, bx, by, bancho * integridad / 100.0f, balto);
		Pen marco(Color::FromArgb(200, 205, 220), 1.0f);
		g->DrawRectangle(%marco, bx, by, bancho, balto);

		Estilos::textoIzquierda(g,
			String::Format(L"Velocidad: {0:F0} km/s   ({1}%)", velocidad, integridad),
			Estilos::hud, Color::FromArgb(170, 220, 255), px + 10.0f, py + 44.0f);
	}
}
