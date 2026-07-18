#pragma once
#include "Dinosaurio.h"

// Dinosaurio Alfa (T-Rex): amenaza final que aparece cuando se ha capturado
// el 70% del total de dinosaurios generados.
// Algoritmo propio: es mas veloz que cualquier otra especie, se desplaza en
// HORIZONTAL con sentido inicial aleatorio y rebota en los bordes.
// Requiere 2 impactos de dardo para ser capturado y quita 1 vida por colision.
// Sprite: DINOALFA.png (hoja 3x4, frames de 64x64).
class DinosaurioAlfa : public Dinosaurio
{
private:
	int impactos;

public:
	DinosaurioAlfa(int x, int y);
	void mover(Rectangle zona) override;
	int getTipo() override;
	bool recibirImpacto() override;   // solo captura al segundo impacto
};
