#pragma once
#include "LlamaDigital.h"

// Jefe final: Huallallo Carhuincho, el devorador de fuego al servicio de
// Oblivion Corp. Hereda de LlamaDigital (herencia en dos niveles) y ademas
// de perseguir puede ir a corromper el pilar de informacion mas avanzado.
class HuallalloJefe : public LlamaDigital {
private:
	int vidaMax;
	bool corrompiendo; // true cuando esta drenando un pilar

public:
	HuallalloJefe(int px, int py);

	int getVidaMax();
	bool getCorrompiendo();
	void setCorrompiendo(bool value);

	void recibirRayo(); // dano del Ray-Trace

	void perseguir(int objetivoX, int objetivoY); // el objetivo lo decide Juego
	Rectangle hitbox();
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};
