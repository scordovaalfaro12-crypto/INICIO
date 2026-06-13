#include "Planeta.h"

using namespace System::Drawing::Drawing2D;

namespace PoliNavis {

	Planeta::Planeta(float psolX, float psolY, float pradioOrbita, float panguloIni,
		float pvelOrbita, float pvelRotacion, float pradio, String^ pnombre, String^ pdato)
		: Entidad(psolX + pradioOrbita, psolY) {
		solX = psolX; solY = psolY;
		radioOrbita = pradioOrbita;
		anguloOrbita = panguloIni;
		velOrbita = pvelOrbita;
		rotacion = 0.0f;
		velRotacion = pvelRotacion;
		radio = pradio;
		impulso = 1.0f;
		nombre = pnombre;
		dato = pdato;
		colorLeyenda = Color::White;
	}

	void Planeta::mover() {
		// TRASLACION: avanza en la orbita y recalcula su posicion
		anguloOrbita += velOrbita * impulso;
		double a = anguloOrbita * Math::PI / 180.0;
		x = solX + (float)Math::Cos(a) * radioOrbita;
		y = solY + (float)Math::Sin(a) * radioOrbita * 0.62f;  // orbita en perspectiva

		// ROTACION: gira sobre su eje
		rotacion += velRotacion * impulso;

		// el impulso por evento decae poco a poco hasta 1
		if (impulso > 1.0f) {
			impulso -= 0.04f;
			if (impulso < 1.0f) impulso = 1.0f;
		}
	}

	void Planeta::acelerar() {
		impulso = 6.0f;   // al hacer clic, gira y orbita mucho mas rapido
	}

	RectangleF Planeta::area() {
		return RectangleF(x - radio * 0.85f, y - radio * 0.85f, radio * 1.7f, radio * 1.7f);
	}

	void Planeta::dibujarOrbita(Graphics^ g) {
		Pen orbita(Color::FromArgb(35, 200, 220, 255), 1.0f);
		g->DrawEllipse(%orbita, solX - radioOrbita, solY - radioOrbita * 0.62f,
			radioOrbita * 2.0f, radioOrbita * 0.62f * 2.0f);
	}

	void Planeta::dibujar(Graphics^ g) {
		// halo
		SolidBrush halo(Color::FromArgb(30, 200, 220, 255));
		g->FillEllipse(%halo, x - radio * 1.2f, y - radio * 1.2f, radio * 2.4f, radio * 2.4f);

		// recorte circular: la superficie no se sale del planeta
		GraphicsPath recorte;
		recorte.AddEllipse(x - radio, y - radio, radio * 2.0f, radio * 2.0f);
		g->SetClip(%recorte);
		dibujarSuperficie(g, radio, rotacion);
		// sombra esferica (da volumen 3D y marca el terminador)
		SolidBrush sombra(Color::FromArgb(70, 0, 0, 25));
		g->FillEllipse(%sombra, x - radio + radio * 0.5f, y - radio + radio * 0.35f,
			radio * 2.0f, radio * 2.0f);
		g->ResetClip();

		// borde
		Pen borde(Color::FromArgb(90, 255, 255, 255), 1.0f);
		g->DrawEllipse(%borde, x - radio, y - radio, radio * 2.0f, radio * 2.0f);

		dibujarExtra(g, radio);
	}

	void Planeta::marca(Graphics^ g, float cx, float cy, float r,
		float longitud, float rot, float lat, float tam, Color color) {
		double ang = (longitud - rot) * Math::PI / 180.0;
		double frente = Math::Cos(ang);
		if (frente <= 0.05) return;                    // esta en la cara oculta
		float mx = cx + (float)Math::Sin(ang) * r * 0.78f;
		float my = cy + lat * r;
		float escala = (float)frente;                  // perspectiva al borde
		float t = tam * (0.5f + 0.5f * escala);
		SolidBrush pincel(color);
		g->FillEllipse(%pincel, mx - t, my - t * 0.85f, t * 2.0f, t * 1.7f);
	}

	String^ Planeta::getNombre() { return nombre; }
	String^ Planeta::getDato() { return dato; }
	float Planeta::getRadio() { return radio; }
}
