#include "Astronauta.h"
#include "Item.h"
#include "Proyectil.h"
#include "Sonido.h"
#include "Estilos.h"
#include "Estrella.h"

using namespace System::Drawing::Drawing2D;

namespace JuegoEspacial {

	Astronauta::Astronauta(String^ pnombre, Color pcolorTraje, int pestilo, float px, float py)
		: EntidadEspacial(px, py) {
		nombre = pnombre;
		colorTraje = pcolorTraje;
		estilo = pestilo;
		puntaje = 0;
		escala = 1.0f;
		fase = 0.0f;
		caminando = false;
		mirandoIzquierda = false;
		jetpack = false;
		vidas = 3;
		invulnerable = 0;
	}

	void Astronauta::cambiarPerfil(String^ pnombre, Color pcolorTraje, int pestilo) {
		nombre = pnombre;
		colorTraje = pcolorTraje;
		estilo = pestilo;
	}

	void Astronauta::mover() {
		if (caminando) fase += 0.32f;
		caminando = false;            // se vuelve a activar cada tick que camina
		if (invulnerable > 0) invulnerable--;
	}

	void Astronauta::caminar(float dx, float minX, float maxX) {
		if (dx == 0.0f) return;
		x += dx;
		if (x < minX) x = minX;
		if (x > maxX) x = maxX;
		caminando = true;
		mirandoIzquierda = dx < 0.0f;
	}

	void Astronauta::volar(float dx, float dy, float minX, float minY, float maxX, float maxY) {
		if (dx == 0.0f && dy == 0.0f) return;
		x += dx;
		y += dy;
		if (x < minX) x = minX;
		if (x > maxX) x = maxX;
		if (y < minY) y = minY;
		if (y > maxY) y = maxY;
		caminando = true;
		if (dx != 0.0f) mirandoIzquierda = dx < 0.0f;
	}

	void Astronauta::recolectarItem(Item^ item) {
		if (item != nullptr) item->aplicarEfecto(this);
	}

	Proyectil^ Astronauta::disparar() {
		Sonido::disparo();
		return gcnew Proyectil(x, y - 52.0f * escala, 0.0f, -9.5f, Proyectil::LASER);
	}

	void Astronauta::sumarPuntaje(int cantidad) {
		puntaje += cantidad;
	}

	String^ Astronauta::getNombre() { return nombre; }
	int Astronauta::getPuntaje() { return puntaje; }
	Color Astronauta::getColorTraje() { return colorTraje; }
	int Astronauta::getEstilo() { return estilo; }
	int Astronauta::getVidas() { return vidas; }

	void Astronauta::setVidas(int v) {
		vidas = v;
		invulnerable = 0;
	}

	void Astronauta::perderVida() {
		vidas--;
		invulnerable = 90;
		Sonido::danio();
	}

	bool Astronauta::esInvulnerable() {
		return invulnerable > 0;
	}

	void Astronauta::setEscala(float e) { escala = e; }
	void Astronauta::setJetpack(bool activo) { jetpack = activo; }

	RectangleF Astronauta::zona() {
		return RectangleF(x - 14.0f * escala, y - 88.0f * escala, 28.0f * escala, 88.0f * escala);
	}

	void Astronauta::dibujar(Graphics^ g) {
		// parpadea mientras es invulnerable
		if (invulnerable > 0 && (invulnerable / 6) % 2 == 0) return;
		dibujarModelo(g, x, y, escala, colorTraje, estilo, fase, mirandoIzquierda, jetpack);
	}

	void Astronauta::dibujarModelo(Graphics^ g, float px, float py, float u,
		Color traje, int estilo, float fase, bool mirandoIzquierda, bool jetpack) {

		float balanceo = (float)Math::Sin(fase) * 9.0f * u;     // vaiven de piernas y brazos
		float lado = mirandoIzquierda ? -1.0f : 1.0f;
		Color trajeOscuro = Estilos::oscurecer(traje, 0.65f);

		// ---- mochila (detras del cuerpo) ----
		float mochilaX = px - lado * 22.0f * u;
		Estilos::rellenarRedondeado(g, Color::FromArgb(135, 140, 150), mochilaX, py - 68.0f * u, 14.0f * u, 32.0f * u, 4.0f * u);
		SolidBrush tanque(Color::FromArgb(190, 195, 205));
		g->FillEllipse(%tanque, mochilaX + 3.0f * u, py - 72.0f * u, 8.0f * u, 10.0f * u);

		// llama del jetpack
		if (jetpack) {
			array<PointF>^ fuego = gcnew array<PointF>(3);
			float fx = mochilaX + 7.0f * u;
			float flicker = (float)Math::Sin(fase * 2.0f) * 3.0f * u;
			fuego[0] = PointF(fx - 5.0f * u, py - 37.0f * u);
			fuego[1] = PointF(fx + 5.0f * u, py - 37.0f * u);
			fuego[2] = PointF(fx, py - 16.0f * u + flicker);
			SolidBrush llamaNaranja(Color::FromArgb(220, 255, 150, 50));
			g->FillPolygon(%llamaNaranja, fuego);
			fuego[0] = PointF(fx - 2.5f * u, py - 37.0f * u);
			fuego[1] = PointF(fx + 2.5f * u, py - 37.0f * u);
			fuego[2] = PointF(fx, py - 24.0f * u + flicker * 0.6f);
			SolidBrush llamaAmarilla(Color::FromArgb(240, 255, 235, 140));
			g->FillPolygon(%llamaAmarilla, fuego);
		}

		// ---- piernas (lineas gruesas con botas) ----
		Pen pierna(trajeOscuro, 7.0f * u);
		pierna.StartCap = LineCap::Round;
		pierna.EndCap = LineCap::Round;
		float caderaY = py - 34.0f * u;
		float pieY = py - 4.0f * u;
		float pie1X = px - 5.0f * u + balanceo;
		float pie2X = px + 5.0f * u - balanceo;
		g->DrawLine(%pierna, px - 5.0f * u, caderaY, pie1X, pieY);
		g->DrawLine(%pierna, px + 5.0f * u, caderaY, pie2X, pieY);
		SolidBrush bota(Color::FromArgb(225, 228, 235));
		g->FillEllipse(%bota, pie1X - 6.0f * u, pieY - 3.0f * u, 12.0f * u, 7.0f * u);
		g->FillEllipse(%bota, pie2X - 6.0f * u, pieY - 3.0f * u, 12.0f * u, 7.0f * u);

		// ---- brazo de atras ----
		Pen brazo(trajeOscuro, 6.0f * u);
		brazo.StartCap = LineCap::Round;
		brazo.EndCap = LineCap::Round;
		float hombroY = py - 58.0f * u;
		float manoY = py - 38.0f * u;
		float mano1X = px - 11.0f * u - balanceo * 0.7f;
		g->DrawLine(%brazo, px - 9.0f * u, hombroY, mano1X, manoY);

		// ---- torso ----
		Estilos::rellenarRedondeado(g, traje, px - 14.0f * u, py - 66.0f * u, 28.0f * u, 36.0f * u, 8.0f * u);
		Estilos::bordeRedondeado(g, trajeOscuro, 1.5f * u, px - 14.0f * u, py - 66.0f * u, 28.0f * u, 36.0f * u, 8.0f * u);

		// panel de control del pecho
		SolidBrush panel(Color::FromArgb(45, 50, 65));
		g->FillRectangle(%panel, px - 8.0f * u, py - 60.0f * u, 16.0f * u, 11.0f * u);
		SolidBrush led1(Color::FromArgb(255, 90, 90));
		SolidBrush led2(Color::FromArgb(255, 220, 90));
		SolidBrush led3(Color::FromArgb(110, 230, 110));
		g->FillEllipse(%led1, px - 6.0f * u, py - 57.5f * u, 3.6f * u, 3.6f * u);
		g->FillEllipse(%led2, px - 1.5f * u, py - 57.5f * u, 3.6f * u, 3.6f * u);
		g->FillEllipse(%led3, px + 3.0f * u, py - 57.5f * u, 3.6f * u, 3.6f * u);

		// cinturon
		SolidBrush cinturon(Color::FromArgb(70, 75, 90));
		g->FillRectangle(%cinturon, px - 14.0f * u, py - 38.0f * u, 28.0f * u, 4.5f * u);

		// ---- brazo delantero ----
		float mano2X = px + 11.0f * u + balanceo * 0.7f;
		g->DrawLine(%brazo, px + 9.0f * u, hombroY, mano2X, manoY);
		SolidBrush guante(Color::FromArgb(225, 228, 235));
		g->FillEllipse(%guante, mano1X - 4.5f * u, manoY - 4.5f * u, 9.0f * u, 9.0f * u);
		g->FillEllipse(%guante, mano2X - 4.5f * u, manoY - 4.5f * u, 9.0f * u, 9.0f * u);

		// ---- casco ----
		SolidBrush casco(Color::FromArgb(235, 238, 245));
		g->FillEllipse(%casco, px - 13.5f * u, py - 94.0f * u, 27.0f * u, 27.0f * u);
		SolidBrush cuello(Color::FromArgb(160, 165, 175));
		g->FillRectangle(%cuello, px - 8.0f * u, py - 69.5f * u, 16.0f * u, 4.5f * u);

		// visor (el color depende del estilo)
		Color colorVisor = Color::FromArgb(30, 45, 85);
		if (estilo == 2) colorVisor = Color::FromArgb(25, 70, 50);
		if (estilo == 3) colorVisor = Color::FromArgb(95, 50, 15);
		SolidBrush visor(colorVisor);
		float visorX = px - 9.0f * u + lado * 2.5f * u;
		g->FillEllipse(%visor, visorX, py - 89.5f * u, 18.0f * u, 16.0f * u);
		SolidBrush reflejo(Color::FromArgb(140, 200, 230, 255));
		g->FillEllipse(%reflejo, visorX + 3.0f * u, py - 87.0f * u, 5.5f * u, 4.0f * u);

		// ---- detalles segun el personaje ----
		if (estilo == 0) {
			// comandante: franja roja en el casco e insignia de estrella
			SolidBrush franja(Color::FromArgb(210, 60, 60));
			g->FillRectangle(%franja, px - 2.0f * u, py - 96.0f * u, 4.0f * u, 7.0f * u);
			Estrella::dibujarForma(g, px + lado * -9.0f * u, py - 62.0f * u, 4.2f * u, 0.0f, Color::FromArgb(255, 210, 70));
		}
		else if (estilo == 1) {
			// ingeniero: antena con luz y llave inglesa en el cinturon
			Pen antena(Color::FromArgb(180, 185, 195), 2.0f * u);
			g->DrawLine(%antena, px + 8.0f * u, py - 92.0f * u, px + 13.0f * u, py - 103.0f * u);
			SolidBrush foco(Color::FromArgb(255, 120, 120));
			g->FillEllipse(%foco, px + 11.0f * u, py - 107.0f * u, 5.0f * u, 5.0f * u);
			SolidBrush herramienta(Color::FromArgb(200, 205, 215));
			g->FillRectangle(%herramienta, px + 8.0f * u, py - 37.0f * u, 4.0f * u, 12.0f * u);
			g->FillEllipse(%herramienta, px + 6.5f * u, py - 41.0f * u, 7.0f * u, 7.0f * u);
		}
		else if (estilo == 2) {
			// cientifico: matraz en el pecho y hombreras
			array<PointF>^ matraz = gcnew array<PointF>(3);
			matraz[0] = PointF(px - 12.0f * u, py - 43.0f * u);
			matraz[1] = PointF(px - 4.0f * u, py - 43.0f * u);
			matraz[2] = PointF(px - 8.0f * u, py - 51.0f * u);
			SolidBrush vidrio(Color::FromArgb(120, 240, 200));
			g->FillPolygon(%vidrio, matraz);
			SolidBrush hombrera(Estilos::oscurecer(traje, 0.5f));
			g->FillEllipse(%hombrera, px - 16.0f * u, hombroY - 5.0f * u, 10.0f * u, 9.0f * u);
			g->FillEllipse(%hombrera, px + 6.0f * u, hombroY - 5.0f * u, 10.0f * u, 9.0f * u);
		}
		else {
			// explorador: linterna en el casco y mochila grande
			SolidBrush linterna(Color::FromArgb(255, 230, 120));
			g->FillRectangle(%linterna, px + lado * 8.0f * u - 3.0f * u, py - 99.0f * u, 6.0f * u, 5.0f * u);
			Estilos::rellenarRedondeado(g, Color::FromArgb(110, 115, 125), mochilaX - 3.0f * u, py - 64.0f * u, 8.0f * u, 22.0f * u, 3.0f * u);
		}
	}
}
