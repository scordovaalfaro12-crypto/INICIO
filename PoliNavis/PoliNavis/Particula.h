#pragma once
using namespace System;
using namespace System::Drawing;
using namespace System::Collections::Generic;

namespace PoliNavis {

	// Particula simple: se usa para llamas, polvo, explosiones y destellos
	public ref class Particula {
	public:
		float x, y, vx, vy, radio;
		int vida, vidaMax;
		Color color;
		bool conGravedad;

		Particula(float px, float py, float pvx, float pvy, float pradio, int pvida, Color pcolor, bool pgravedad);
		void mover();
		void dibujar(Graphics^ g);
		bool muerta();
	};

	// Ayudas para trabajar con listas de particulas
	public ref class Particulas abstract sealed {
	public:
		static void actualizar(List<Particula^>^ lista);
		static void dibujar(Graphics^ g, List<Particula^>^ lista);
		static void explosion(List<Particula^>^ lista, float x, float y, Color color, int cantidad);
		static void polvo(List<Particula^>^ lista, float x, float y, Color color, int cantidad);
		static void llama(List<Particula^>^ lista, float x, float y, float fuerza);
	};
}
