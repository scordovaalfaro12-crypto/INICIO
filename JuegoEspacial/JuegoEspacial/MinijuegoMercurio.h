#pragma once
#include "Minijuego.h"
#include "Asteroide.h"
#include "Nave.h"

namespace JuegoEspacial {

	// MERCURIO - "Lluvia de fuego": esquiva los meteoritos ardientes
	// durante 30 segundos moviendote por el suelo.
	public ref class MinijuegoMercurio : public Minijuego {
	public:
		MinijuegoMercurio(Astronauta^ pjugador);

		virtual void iniciar() override;
		virtual void actualizar(Entrada^ entrada) override;
		virtual void dibujar(Graphics^ g) override;
		virtual String^ progreso() override;

	private:
		List<Asteroide^>^ meteoros;
		Nave^ naveEstacionada;
		int restante;
		int enfriamientoSpawn;
	};
}
