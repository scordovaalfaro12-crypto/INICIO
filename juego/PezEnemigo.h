#pragma once
#include <vector>
using namespace System;
using namespace System::Drawing;
using namespace std;

class PezEnemigo {
private:
	int x, y, dx;
	int ancho, alto;
	int IDx = 0;
	int delay = 0;
	int a;
	enum acciones { derecha, izquierda };
	acciones accion;

public:
	PezEnemigo(Bitmap^ img);
	void mover(Graphics^ g);
	void mostrar(Graphics^ g, Bitmap^ img);
	Rectangle area() { return Rectangle(x, y, ancho, alto); }
};

class PezEnemigoList {
private:
	vector<PezEnemigo*> pezEnemigo;

public:
	PezEnemigoList(int cant, Bitmap^ img);
	~PezEnemigoList();
	void mover(Graphics^ g);
	void mostrar(Graphics^ g, Bitmap^ img);
	void agregar(int cant, Bitmap^ img);
};
