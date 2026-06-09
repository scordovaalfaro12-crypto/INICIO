#pragma once
#include "EntidadEspacial.h"

namespace JuegoEspacial {

	// Disparos del juego: laser del astronauta, rayo del jefe y gotas acidas
	public ref class Proyectil : public EntidadEspacial {
	public:
		literal int LASER = 0;     // disparo del jugador (sube)
		literal int RAYO = 1;      // rayo de energia del jefe final
		literal int GOTA = 2;      // gota acida de las nubes de Venus

		Proyectil(float px, float py, float pvx, float pvy, int ptipo);

		virtual void mover() override;
		virtual void dibujar(Graphics^ g) override;

		bool fuera();              // salio de la pantalla
		RectangleF zona();
		int getTipo();

	private:
		float vx, vy;
		int tipo;
	};
}
