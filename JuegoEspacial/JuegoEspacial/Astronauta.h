#pragma once
#include "EntidadEspacial.h"

namespace JuegoEspacial {

	ref class Item;
	ref class Proyectil;

	// El personaje jugable (segun el UML): tiene nombre y puntaje,
	// recolecta items y dispara. Esta dibujado solo con figuras geometricas.
	public ref class Astronauta : public EntidadEspacial {
	public:
		Astronauta(String^ pnombre, Color pcolorTraje, int pestilo, float px, float py);

		virtual void mover() override;       // avanza la animacion y el parpadeo
		virtual void dibujar(Graphics^ g) override;

		void recolectarItem(Item^ item);     // aplica el efecto del item sobre si mismo
		Proyectil^ disparar();               // crea un laser que sube

		void cambiarPerfil(String^ pnombre, Color pcolorTraje, int pestilo);
		void caminar(float dx, float minX, float maxX);                                  // moverse por el suelo
		void volar(float dx, float dy, float minX, float minY, float maxX, float maxY);  // moverse con jetpack

		void sumarPuntaje(int cantidad);
		String^ getNombre();
		int getPuntaje();
		Color getColorTraje();
		int getEstilo();

		int getVidas();
		void setVidas(int v);
		void perderVida();
		bool esInvulnerable();

		void setEscala(float e);
		void setJetpack(bool activo);
		RectangleF zona();

		// dibuja el modelo del astronauta en cualquier punto (lo usan los menus).
		// (px, py) es el punto donde pisan sus botas.
		static void dibujarModelo(Graphics^ g, float px, float py, float escala,
			Color traje, int estilo, float fase, bool mirandoIzquierda, bool jetpack);

	private:
		String^ nombre;
		int puntaje;
		Color colorTraje;
		int estilo;          // 0 comandante, 1 ingeniero, 2 cientifico, 3 explorador
		float escala;
		float fase;          // animacion de caminar
		bool caminando;
		bool mirandoIzquierda;
		bool jetpack;
		int vidas;
		int invulnerable;    // ticks de parpadeo tras recibir danio
	};
}
