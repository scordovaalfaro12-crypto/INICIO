#pragma once
#include "Entidad.h"

// Enemigo basico: llama digital de Huallallo Carhuincho (los "firewalls").
// Patrulla alrededor de su punto base y persigue al jugador si se acerca.
// Se dibuja por codigo como una flama glitch, sin sprite externo.
class LlamaDigital : public Entidad {
protected:
	int velocidad;
	int radioPersecucion;
	int baseX, baseY;       // centro de su zona de patrulla
	int radioPatrulla;
	int objetivoX, objetivoY; // punto de patrulla actual
	int pausa;              // ticks que descansa entre caminatas
	int semilla;            // variacion del dibujo entre enemigos

public:
	LlamaDigital(int px, int py, int vel, int radioPers);

	void irHacia(int px, int py);                 // avanza un paso hacia un punto
	virtual void perseguir(int jugadorX, int jugadorY); // IA de cada tick
	Rectangle hitbox();
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};
