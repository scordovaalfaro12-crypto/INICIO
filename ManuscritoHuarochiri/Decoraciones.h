#pragma once
#include "Entidad.h"

// Familia de decoraciones animadas del escenario. Demuestra herencia en
// varios niveles (Entidad -> Decoracion -> cada decoracion concreta) y
// polimorfismo: Juego las guarda en un vector<Decoracion*> y llama a
// animar()/mostrar() sin saber el tipo concreto.
class Decoracion : public Entidad {
protected:
	int semilla; // desfase aleatorio para que no se muevan todas igual

public:
	Decoracion(int px, int py);
	void animar();
};

// Antorcha de piedra con llama parpadeante y halo de luz.
class AntorchaDecorativa : public Decoracion {
public:
	AntorchaDecorativa(int px, int py);
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};

// Inti, el sol andino: disco dorado con rayos que giran lentamente.
class SolAndino : public Decoracion {
public:
	SolAndino(int px, int py);
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};

// Burbujas que crecen y revientan sobre la lava.
class BurbujaLava : public Decoracion {
public:
	BurbujaLava(int px, int py);
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};

// Destello que brilla de forma intermitente sobre el agua.
class DestelloAgua : public Decoracion {
public:
	DestelloAgua(int px, int py);
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};

// Espiritu ancestral: personaje secundario que flota en el mapa y, cuando
// el jugador se acerca, Juego dibuja su globo de texto con datos reales
// sobre el Manuscrito de Huarochiri. Usa el sprite de la deidad dibujado
// como fantasma (translucido y calido) mediante una matriz de color.
class PersonajeSecundario : public Entidad {
private:
	int frameAnim; // frame del reposo (0..3), aparte del flote

public:
	PersonajeSecundario(int px, int py, int anchoFrame, int altoFrame);
	void animar();
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};

// Bola de fuego que lanza Huallallo: viaja hacia donde estaba el jugador.
class BolaFuego : public Entidad {
private:
	int vidaUtil;

public:
	BolaFuego(int px, int py, int objetivoX, int objetivoY);
	bool terminado();
	void mover(Graphics^ g);
	Rectangle hitbox();
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};
