#pragma once
#include "Entidad.h"

// Enemigo base: llama digital de Huallallo Carhuincho (los "firewalls").
// Patrulla alrededor de su punto base y persigue al jugador si se acerca.
// Es la raiz del bestiario: GolemLava y LoboLava heredan de aqui y
// HuallalloJefe tambien, demostrando herencia en varios niveles.
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
	virtual bool recibirImpacto();                // resta vida; true si murio
	virtual int colorParticula();                 // tono de su estallido (0 fuego)
	Rectangle hitbox();
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};

// Golem de lava: enemigo tanque, lento pero resistente (aguanta 3 impactos).
// Avanza sin descanso hacia el jugador. Se dibuja por codigo como una
// figura rocosa con grietas de lava palpitantes.
class GolemLava : public LlamaDigital {
public:
	GolemLava(int px, int py);
	void perseguir(int jugadorX, int jugadorY);
	int colorParticula();
	Rectangle hitbox();
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};

// Lobo de lava: enemigo rapido. Acecha y, cuando el jugador entra en su
// rango, se abalanza con una embestida veloz. Cuadrupedo con melena de fuego.
class LoboLava : public LlamaDigital {
private:
	int embestida;   // ticks restantes de la embestida
	int recarga;     // espera antes de poder volver a embestir

public:
	LoboLava(int px, int py);
	void perseguir(int jugadorX, int jugadorY);
	Rectangle hitbox();
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};
