#pragma once
#include "Minijuego.h"
#include "Asteroide.h"
#include "Artefacto.h"
#include "Nave.h"

namespace JuegoEspacial {

	// MARTE - "Caza de artefactos": vuela con el jetpack y recoge
	// 6 artefactos antiguos esquivando las rocas rodantes.
	public ref class MinijuegoMarte : public Minijuego {
	public:
		MinijuegoMarte(Astronauta^ pjugador);

		virtual void iniciar() override;
		virtual void actualizar(Entrada^ entrada) override;
		virtual void dibujar(Graphics^ g) override;
		virtual String^ progreso() override;

	private:
		List<Asteroide^>^ rocas;
		Artefacto^ actual;
		Nave^ naveEstacionada;
		int recogidos;
		void nuevoArtefacto();
	};
}
