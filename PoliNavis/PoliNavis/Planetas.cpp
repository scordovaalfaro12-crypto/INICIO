#include "Planetas.h"
#include "Azar.h"

using namespace System::Drawing::Drawing2D;

namespace PoliNavis {

	// ---------------- MERCURIO ----------------
	Mercurio::Mercurio(float sx, float sy, float ro, float a0)
		: Planeta(sx, sy, ro, a0, 1.30f, 2.2f, 12.0f, L"Mercurio",
			L"El mas cercano al Sol. Traslacion: 88 dias.") {
		colorLeyenda = Color::FromArgb(150, 140, 130);
	}
	void Mercurio::dibujarSuperficie(Graphics^ g, float r, float rot) {
		SolidBrush base(Color::FromArgb(150, 140, 130));
		g->FillEllipse(%base, x - r, y - r, r * 2.0f, r * 2.0f);
		// crateres que giran con el planeta
		marca(g, x, y, r, 20.0f, rot, -0.3f, r * 0.30f, Color::FromArgb(110, 100, 92));
		marca(g, x, y, r, 90.0f, rot, 0.15f, r * 0.34f, Color::FromArgb(105, 96, 88));
		marca(g, x, y, r, 160.0f, rot, 0.4f, r * 0.24f, Color::FromArgb(115, 104, 96));
		marca(g, x, y, r, 250.0f, rot, -0.15f, r * 0.28f, Color::FromArgb(100, 92, 84));
	}

	// ---------------- VENUS ----------------
	Venus::Venus(float sx, float sy, float ro, float a0)
		: Planeta(sx, sy, ro, a0, 0.95f, 1.0f, 16.0f, L"Venus",
			L"Rota al reves y muy lento. El mas caliente.") {
		colorLeyenda = Color::FromArgb(218, 180, 98);
	}
	void Venus::dibujarSuperficie(Graphics^ g, float r, float rot) {
		SolidBrush base(Color::FromArgb(218, 180, 98));
		g->FillEllipse(%base, x - r, y - r, r * 2.0f, r * 2.0f);
		Pen banda(Color::FromArgb(120, 240, 215, 150), r * 0.16f);
		g->DrawArc(%banda, x - r * 1.1f, y - r * 0.9f, r * 2.2f, r * 1.2f, 20.0f, 140.0f);
		g->DrawArc(%banda, x - r * 1.05f, y - r * 0.1f, r * 2.1f, r * 1.1f, 200.0f, 140.0f);
		marca(g, x, y, r, 60.0f, rot, -0.2f, r * 0.3f, Color::FromArgb(120, 235, 205, 140));
		marca(g, x, y, r, 200.0f, rot, 0.25f, r * 0.26f, Color::FromArgb(120, 235, 205, 140));
	}

	// ---------------- TIERRA (obligatorio) ----------------
	Tierra::Tierra(float sx, float sy, float ro, float a0)
		: Planeta(sx, sy, ro, a0, 0.78f, 2.6f, 17.0f, L"Tierra",
			L"Nuestro hogar. Rotacion: 24 h. Traslacion: 365 dias.") {
		colorLeyenda = Color::FromArgb(60, 130, 210);
		lunaAng = 0.0f;
	}
	void Tierra::dibujarSuperficie(Graphics^ g, float r, float rot) {
		// oceano
		SolidBrush oceano(Color::FromArgb(52, 110, 200));
		g->FillEllipse(%oceano, x - r, y - r, r * 2.0f, r * 2.0f);
		// continentes verdes que ROTAN (detalle identificable)
		marca(g, x, y, r, 30.0f, rot, -0.25f, r * 0.42f, Color::FromArgb(80, 170, 90));
		marca(g, x, y, r, 80.0f, rot, 0.2f, r * 0.34f, Color::FromArgb(70, 160, 80));
		marca(g, x, y, r, 175.0f, rot, 0.05f, r * 0.4f, Color::FromArgb(85, 175, 95));
		marca(g, x, y, r, 250.0f, rot, 0.35f, r * 0.3f, Color::FromArgb(75, 165, 85));
		// nubes blancas (giran un poco distinto)
		marca(g, x, y, r, 120.0f, rot * 1.15f, -0.45f, r * 0.3f, Color::FromArgb(130, 255, 255, 255));
		marca(g, x, y, r, 300.0f, rot * 1.15f, 0.5f, r * 0.28f, Color::FromArgb(120, 255, 255, 255));
		// casquete polar
		SolidBrush polo(Color::FromArgb(180, 240, 245, 255));
		g->FillEllipse(%polo, x - r * 0.35f, y - r * 1.02f, r * 0.7f, r * 0.4f);
	}
	void Tierra::dibujarExtra(Graphics^ g, float r) {
		lunaAng += 3.0f;
		double a = lunaAng * Math::PI / 180.0;
		float lx = x + (float)Math::Cos(a) * r * 1.9f;
		float ly = y + (float)Math::Sin(a) * r * 1.9f * 0.6f;
		SolidBrush luna(Color::FromArgb(205, 205, 210));
		g->FillEllipse(%luna, lx - r * 0.22f, ly - r * 0.22f, r * 0.44f, r * 0.44f);
	}

	// ---------------- MARTE ----------------
	Marte::Marte(float sx, float sy, float ro, float a0)
		: Planeta(sx, sy, ro, a0, 0.62f, 2.5f, 14.0f, L"Marte",
			L"El planeta rojo. Tiene casquetes polares de hielo.") {
		colorLeyenda = Color::FromArgb(200, 95, 55);
	}
	void Marte::dibujarSuperficie(Graphics^ g, float r, float rot) {
		SolidBrush base(Color::FromArgb(200, 95, 55));
		g->FillEllipse(%base, x - r, y - r, r * 2.0f, r * 2.0f);
		marca(g, x, y, r, 40.0f, rot, 0.1f, r * 0.36f, Color::FromArgb(160, 70, 40));
		marca(g, x, y, r, 150.0f, rot, -0.2f, r * 0.3f, Color::FromArgb(150, 65, 38));
		marca(g, x, y, r, 240.0f, rot, 0.3f, r * 0.28f, Color::FromArgb(165, 72, 42));
		SolidBrush polo(Color::FromArgb(230, 240, 245, 255));
		g->FillEllipse(%polo, x - r * 0.4f, y - r * 1.02f, r * 0.8f, r * 0.42f);
	}

	// ---------------- JUPITER ----------------
	Jupiter::Jupiter(float sx, float sy, float ro, float a0)
		: Planeta(sx, sy, ro, a0, 0.42f, 3.6f, 30.0f, L"Jupiter",
			L"El mas grande. Rotacion muy rapida: ~10 horas.") {
		colorLeyenda = Color::FromArgb(205, 170, 125);
	}
	void Jupiter::dibujarSuperficie(Graphics^ g, float r, float rot) {
		SolidBrush base(Color::FromArgb(205, 170, 125));
		g->FillEllipse(%base, x - r, y - r, r * 2.0f, r * 2.0f);
		SolidBrush b1(Color::FromArgb(150, 175, 130, 85));
		SolidBrush b2(Color::FromArgb(120, 230, 205, 165));
		g->FillRectangle(%b1, x - r, y - r * 0.75f, r * 2.0f, r * 0.26f);
		g->FillRectangle(%b2, x - r, y - r * 0.38f, r * 2.0f, r * 0.20f);
		g->FillRectangle(%b1, x - r, y - r * 0.02f, r * 2.0f, r * 0.28f);
		g->FillRectangle(%b2, x - r, y + r * 0.42f, r * 2.0f, r * 0.20f);
		// gran mancha roja que rota
		marca(g, x, y, r, 90.0f, rot, 0.12f, r * 0.32f, Color::FromArgb(195, 85, 60));
	}

	// ---------------- SATURNO (obligatorio, con anillos) ----------------
	Saturno::Saturno(float sx, float sy, float ro, float a0)
		: Planeta(sx, sy, ro, a0, 0.32f, 3.2f, 24.0f, L"Saturno",
			L"Famoso por sus anillos de hielo y roca.") {
		colorLeyenda = Color::FromArgb(216, 186, 120);
	}
	void Saturno::dibujarSuperficie(Graphics^ g, float r, float rot) {
		SolidBrush base(Color::FromArgb(216, 186, 120));
		g->FillEllipse(%base, x - r, y - r, r * 2.0f, r * 2.0f);
		SolidBrush banda(Color::FromArgb(110, 190, 155, 95));
		g->FillRectangle(%banda, x - r, y - r * 0.5f, r * 2.0f, r * 0.24f);
		g->FillRectangle(%banda, x - r, y + r * 0.1f, r * 2.0f, r * 0.28f);
		marca(g, x, y, r, 70.0f, rot, -0.2f, r * 0.22f, Color::FromArgb(150, 175, 140, 80));
	}
	void Saturno::dibujarExtra(Graphics^ g, float r) {
		// anillos: dos arcos (el de atras ya quedaria tapado; dibujamos elipse completa)
		g->TranslateTransform(x, y);
		g->RotateTransform(-18.0f);
		Pen anillo(Color::FromArgb(230, 225, 200, 150), r * 0.20f);
		g->DrawEllipse(%anillo, -r * 1.9f, -r * 0.62f, r * 3.8f, r * 1.24f);
		Pen surco(Color::FromArgb(150, 150, 125, 85), r * 0.06f);
		g->DrawEllipse(%surco, -r * 1.6f, -r * 0.5f, r * 3.2f, r * 1.0f);
		g->ResetTransform();
	}

	// ---------------- URANO ----------------
	Urano::Urano(float sx, float sy, float ro, float a0)
		: Planeta(sx, sy, ro, a0, 0.24f, 2.0f, 19.0f, L"Urano",
			L"Gira inclinado de lado. Gigante helado.") {
		colorLeyenda = Color::FromArgb(130, 212, 225);
	}
	void Urano::dibujarSuperficie(Graphics^ g, float r, float rot) {
		SolidBrush base(Color::FromArgb(130, 212, 225));
		g->FillEllipse(%base, x - r, y - r, r * 2.0f, r * 2.0f);
		marca(g, x, y, r, 50.0f, rot, -0.2f, r * 0.34f, Color::FromArgb(90, 255, 255, 255));
		marca(g, x, y, r, 210.0f, rot, 0.25f, r * 0.3f, Color::FromArgb(70, 180, 230, 235));
	}
	void Urano::dibujarExtra(Graphics^ g, float r) {
		g->TranslateTransform(x, y);
		g->RotateTransform(72.0f);
		Pen anillo(Color::FromArgb(120, 220, 240, 250), 2.0f);
		g->DrawEllipse(%anillo, -r * 1.55f, -r * 0.4f, r * 3.1f, r * 0.8f);
		g->ResetTransform();
	}

	// ---------------- NEPTUNO ----------------
	Neptuno::Neptuno(float sx, float sy, float ro, float a0)
		: Planeta(sx, sy, ro, a0, 0.18f, 2.4f, 18.0f, L"Neptuno",
			L"El mas lejano. Vientos de hasta 2000 km/h.") {
		colorLeyenda = Color::FromArgb(45, 75, 200);
	}
	void Neptuno::dibujarSuperficie(Graphics^ g, float r, float rot) {
		SolidBrush base(Color::FromArgb(45, 75, 200));
		g->FillEllipse(%base, x - r, y - r, r * 2.0f, r * 2.0f);
		Pen viento(Color::FromArgb(110, 220, 235, 255), r * 0.07f);
		g->DrawArc(%viento, x - r * 0.9f, y + r * 0.2f, r * 1.8f, r * 0.5f, 10.0f, 160.0f);
		// gran mancha oscura que rota
		marca(g, x, y, r, 110.0f, rot, -0.1f, r * 0.3f, Color::FromArgb(160, 25, 40, 120));
	}
}
