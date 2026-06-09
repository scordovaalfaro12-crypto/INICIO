#pragma once
#include "EntidadEspacial.h"

namespace JuegoEspacial {

	// La nave espacial (segun el UML): tiene integridad y velocidad,
	// recibe danio, se repara y muestra su estado en pantalla.
	// (x, y) es el centro de su base (donde apoyan las patas).
	public ref class Nave : public EntidadEspacial {
	public:
		Nave(float px, float py);

		virtual void mover() override;               // animacion de la llama y parpadeos
		virtual void dibujar(Graphics^ g) override;  // cohete vertical (en el suelo)
		void dibujarHorizontal(Graphics^ g);         // cohete volando hacia la derecha

		void recibirDanio(int cantidad);
		void reparar(int cantidad);
		void mostrarEstado(Graphics^ g);             // panel HUD con la integridad

		int getIntegridad();
		float getVelocidad();
		void setVelocidad(float v);
		void setLlama(bool encendida);
		void setPatas(bool visibles);
		void setInclinacion(float grados);
		bool destruida();
		RectangleF zona();                           // hitbox cuando vuela horizontal

	private:
		int integridad;
		float velocidad;
		float inclinacion;
		bool llama;
		bool patas;
		float faseLlama;
		int parpadeoDanio;
		void dibujarCuerpo(Graphics^ g);             // cohete hacia arriba con base en (0,0)
	};
}
