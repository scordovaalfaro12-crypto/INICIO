#pragma once
#include "Dependencias.h"

// Clase base de todas las entidades del juego (jugador, enemigos, objetos).
// Demuestra herencia y polimorfismo: cada hija redefine mover() y mostrar().
class Entidad {
protected:
	int x, y;        // posicion en el mundo
	int dx, dy;      // velocidad actual
	int ancho, alto; // tamano en pixeles
	int vida;
	int IDx;         // frame actual de la animacion
	int delay;       // contador para regular la velocidad de animacion
	bool activo;     // si sigue participando en el nivel

public:
	Entidad();
	virtual ~Entidad();

	int getX();
	int getY();
	int getDX();
	int getDY();
	int getAncho();
	int getAlto();
	int getVida();
	bool getActivo();

	void setX(int value);
	void setY(int value);
	void setDX(int value);
	void setDY(int value);
	void setAncho(int value);
	void setAlto(int value);
	void setVida(int value);
	void setActivo(bool value);

	int centroX();
	int centroY();
	double distanciaA(Entidad* otra);

	Rectangle area();
	virtual Rectangle hitbox(); // caja de colision (mas justa que el area total)

	virtual void mover(Graphics^ g);
	virtual void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};
