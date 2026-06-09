#pragma once
#include "Enemigo.h"
#include "Proyectil.h"
#include "Asteroide.h"

namespace JuegoEspacial {

	// El jefe final: un gran platillo alienigena que custodia Neptuno.
	// Ataca con rayos de energia y lanzando asteroides (segun el UML).
	public ref class JefeFinal : public Enemigo {
	public:
		JefeFinal(float px, float py, int pvida);

		virtual void mover() override;          // flota de lado a lado
		virtual void atacar() override;         // se carga de energia antes de disparar
		virtual void dibujar(Graphics^ g) override;
		virtual RectangleF zona() override;

		Proyectil^ dispararRayo(float objetivoX, float objetivoY);
		Asteroide^ lanzarAsteroide();

		void dibujarBarraVida(Graphics^ g);
		bool enfurecido();                      // con poca vida se vuelve mas rapido
		int getVidaMax();
		void setObjetivo(float px, float py);   // hacia donde mira su ojo

	private:
		int vidaMax;
		float velocidadX;
		float objetivoX, objetivoY;
		float giro;          // rotacion de las puas
		int ticks;
	};
}
