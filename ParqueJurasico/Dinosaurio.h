#pragma once
#include <cstdlib>

using namespace System;
using namespace System::Drawing;

// Clase base abstracta de todas las especies de dinosaurios.
// Cada subclase implementa su propio algoritmo de desplazamiento (polimorfismo
// genuino: Velocirraptor se mueve en un solo eje, Pteranodonte en diagonal y
// DinosaurioAlfa en horizontal con doble impacto para ser capturado).
class Dinosaurio
{
protected:
	int x, y;                 // posicion en pantalla
	int ancho, alto;          // tamano con el que se dibuja
	int velocidad;
	int dx, dy;               // direccion/velocidad actual por eje
	int fila;                 // fila del sprite segun hacia donde mira
	int frame;                // columna de animacion
	int frameAncho, frameAlto;// tamano de cada frame dentro de la hoja
	int totalFrames;          // columnas de la hoja de sprites
	int contadorAnim;
	bool capturado;

public:
	Dinosaurio(int x, int y, int ancho, int alto, int velocidad,
		int frameAncho, int frameAlto, int totalFrames);
	virtual ~Dinosaurio();

	// Metodos polimorficos
	virtual void mover(Rectangle zona) = 0;   // algoritmo propio de cada especie
	virtual int getTipo() = 0;                // 0 velocirraptor, 1 pteranodonte, 2 alfa
	virtual bool recibirImpacto();            // true => queda capturado (el Alfa necesita 2)
	virtual void dibujar(Graphics^ g, Bitmap^ bmp);

	// Comportamiento comun
	void moverCapturado(Rectangle zona);      // desplazamiento lento en la zona de capturados
	void capturar(Rectangle zonaCapturados);
	bool estaCapturado();
	Rectangle getRectangulo();

protected:
	void avanzarAnimacion();
};
