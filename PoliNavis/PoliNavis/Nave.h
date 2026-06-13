#pragma once
#include "Entidad.h"

namespace PoliNavis {

	ref class Particula;

	// La nave que controla el jugador. Construida SOLO con poligonos, lineas
	// y rectangulos (sin sprites). Se mueve en las 4 direcciones con el
	// teclado y apunta hacia donde viaja. Tiene vidas y energia; al chocar
	// regresa al Punto A perdiendo una vida (regla de la rubrica).
	public ref class Nave : public Entidad {
	public:
		Nave(float px, float py);

		virtual void mover() override;
		virtual void dibujar(Graphics^ g) override;
		virtual RectangleF area() override;

		void acelerar(float ax, float ay);   // fija la velocidad deseada (input)
		void reiniciarEn(float px, float py); // vuelve al origen, parpadea
		void perderVida();
		void gastarEnergia(int cantidad);
		void recargarEnergia(int cantidad);

		int getVidas();
		int getEnergia();
		bool sinVidas();
		bool esInvulnerable();
		void setColorTrazo(Color c);          // paleta (Nivel 1 vs Nivel 2)
		bool propulsando();

	private:
		int vidas;
		int energia;
		int invulnerable;     // ticks de parpadeo tras reiniciar
		float angulo;         // hacia donde apunta (grados)
		float faseLlama;      // animacion del propulsor
		Color colorTrazo;     // color de detalle segun el nivel
	};
}
