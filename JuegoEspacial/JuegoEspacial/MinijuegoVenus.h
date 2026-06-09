#pragma once
#include "Minijuego.h"
#include "Estrella.h"
#include "Proyectil.h"
#include "Nave.h"

namespace JuegoEspacial {

	// VENUS - "Atrapa estrellas": recoge 15 estrellas que caen del cielo
	// y evita las gotas de lluvia acida.
	public ref class MinijuegoVenus : public Minijuego {
	public:
		MinijuegoVenus(Astronauta^ pjugador);

		virtual void iniciar() override;
		virtual void actualizar(Entrada^ entrada) override;
		virtual void dibujar(Graphics^ g) override;
		virtual String^ progreso() override;

	private:
		List<Estrella^>^ estrellas;
		List<Proyectil^>^ gotas;
		Nave^ naveEstacionada;
		int atrapadas;
		int enfriamientoEstrella;
		int enfriamientoGota;
	};
}
