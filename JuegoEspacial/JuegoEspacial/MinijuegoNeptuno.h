#pragma once
#include "Minijuego.h"
#include "JefeFinal.h"
#include "Proyectil.h"
#include "Asteroide.h"
#include "Nave.h"

namespace JuegoEspacial {

	// NEPTUNO - "El Jefe Final": el platillo alienigena que domina la galaxia.
	// Esquiva sus rayos y asteroides y derribalo a disparos.
	public ref class MinijuegoNeptuno : public Minijuego {
	public:
		MinijuegoNeptuno(Astronauta^ pjugador);

		virtual void iniciar() override;
		virtual void actualizar(Entrada^ entrada) override;
		virtual void dibujar(Graphics^ g) override;
		virtual String^ progreso() override;
		virtual void dibujarHUD(Graphics^ g) override;

	private:
		JefeFinal^ jefe;
		List<Proyectil^>^ rayos;
		List<Asteroide^>^ asteroides;
		List<Proyectil^>^ laseres;
		Nave^ naveEstacionada;
		int enfriamientoDisparo;
		int enfriamientoAtaque;
		int avisoAtaque;          // cuenta regresiva entre la carga y el disparo
		int tipoAtaque;           // 0 = rayo, 1 = asteroide
	};
}
