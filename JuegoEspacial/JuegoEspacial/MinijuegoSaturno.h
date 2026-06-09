#pragma once
#include "Minijuego.h"
#include "Nave.h"
#include "Estrella.h"

namespace JuegoEspacial {

	// Una puerta de anillo: dos barras doradas con un hueco para pasar
	ref class AnilloCarrera {
	public:
		float x;
		float brechaY;       // centro del hueco
		float brechaAlto;    // alto del hueco
		bool pasado;
		bool golpeado;

		AnilloCarrera(float px, float pbrechaY, float pbrechaAlto) {
			x = px;
			brechaY = pbrechaY;
			brechaAlto = pbrechaAlto;
			pasado = false;
			golpeado = false;
		}
	};

	// SATURNO - "Carrera de anillos": pilota la nave a traves de los huecos.
	// Chocar dana el casco de la nave; las estrellas lo reparan.
	public ref class MinijuegoSaturno : public Minijuego {
	public:
		MinijuegoSaturno(Astronauta^ pjugador);

		virtual void iniciar() override;
		virtual void actualizar(Entrada^ entrada) override;
		virtual void dibujar(Graphics^ g) override;
		virtual String^ progreso() override;
		virtual void dibujarHUD(Graphics^ g) override;

	private:
		Nave^ nave;
		List<AnilloCarrera^>^ anillos;
		List<Estrella^>^ estrellas;
		int pasados;
		int enfriamientoEstrella;
	};
}
