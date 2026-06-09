#pragma once
#include "Nave.h"
#include "Astronauta.h"
#include "Particula.h"

namespace JuegoEspacial {

	// Animacion cinematica: el cohete aterriza, se abre la escotilla y
	// el astronauta baja a explorar. En modo despegue ocurre al reves.
	public ref class Aterrizaje {
	public:
		Aterrizaje(int pplaneta, bool pdespegue, Astronauta^ pjugador);

		void actualizar();
		void dibujar(Graphics^ g, int tickGlobal);
		bool haTerminado();
		void saltar();       // el jugador puede saltarse la animacion

	private:
		int planeta;
		bool despegue;
		int tick;
		bool terminado;
		bool sonoAterrizaje;
		bool sonoDespegue;
		float naveX, naveY;
		Nave^ nave;
		Astronauta^ jugador;
		List<Particula^>^ particulas;
		float suavizar(float t);
		void dibujarEscalera(Graphics^ g, float progreso);
	};
}
