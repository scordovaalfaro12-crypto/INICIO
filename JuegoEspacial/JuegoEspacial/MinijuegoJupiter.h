#pragma once
#include "Minijuego.h"
#include "Nave.h"

namespace JuegoEspacial {

	// Un rayo de la tormenta: primero avisa y luego descarga
	ref class RayoTormenta {
	public:
		float x;
		int t;          // < AVISO: advertencia | AVISO..FIN: descarga
		literal int AVISO = 50;
		literal int FIN = 72;

		RayoTormenta(float px) {
			x = px;
			t = 0;
		}
	};

	// JUPITER - "Tormenta electrica": sobrevive 25 segundos sobre las nubes
	// esquivando los rayos; el viento huracanado te empuja.
	public ref class MinijuegoJupiter : public Minijuego {
	public:
		MinijuegoJupiter(Astronauta^ pjugador);

		virtual void iniciar() override;
		virtual void actualizar(Entrada^ entrada) override;
		virtual void dibujar(Graphics^ g) override;
		virtual String^ progreso() override;

	private:
		List<RayoTormenta^>^ rayos;
		Nave^ naveEstacionada;
		int restante;
		int enfriamientoRayo;
		float viento;
		int cicloViento;
	};
}
