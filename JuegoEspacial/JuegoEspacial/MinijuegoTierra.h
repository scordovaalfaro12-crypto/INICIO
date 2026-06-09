#pragma once
#include "Minijuego.h"
#include "BasuraEspacial.h"
#include "Proyectil.h"

namespace JuegoEspacial {

	// TIERRA - "Limpieza orbital": desde una plataforma en orbita,
	// dispara a toda la basura espacial antes de que caiga a la atmosfera.
	public ref class MinijuegoTierra : public Minijuego {
	public:
		MinijuegoTierra(Astronauta^ pjugador);

		virtual void iniciar() override;
		virtual void actualizar(Entrada^ entrada) override;
		virtual void dibujar(Graphics^ g) override;
		virtual String^ progreso() override;

	private:
		List<BasuraEspacial^>^ piezas;
		List<Proyectil^>^ laseres;
		int totalPiezas;
		float direccion;          // 1 = derecha, -1 = izquierda
		int enfriamientoDisparo;
	};
}
