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

// Chispa de un estallido (muerte de enemigo, impacto). Sube/cae con un
// poco de gravedad y se apaga. Pura nativa para poder vivir en un vector.
class Particula : public Entidad {
private:
	double fx, fy, fvx, fvy; // posicion y velocidad con decimales (suavidad)
	int vidaUtil, vidaMax;
	int tono;                // 0 fuego, 1 cian (rayo), 2 verde, 3 dorado

public:
	Particula(int px, int py, double vx, double vy, int vida, int tono);
	bool terminado();
	void animar();
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};

// Texto que sube y se desvanece (ej. "+100"). Guarda el valor como int para
// no tener miembros manejados (String^) dentro de una clase nativa.
class TextoFlotante {
private:
	int x, y;
	int valor;
	int vidaUtil, vidaMax;
	int tono; // 0 dorado, 1 cian, 2 verde

public:
	TextoFlotante(int px, int py, int valor, int tono);
	bool terminado();
	void animar();
	void mostrar(Graphics^ g, Font^ fuente, int camaraX, int camaraY);
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
