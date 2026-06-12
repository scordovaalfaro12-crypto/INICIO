#pragma once
#include "Entidad.h"

// Nodo de datos: fragmento del manuscrito en quechua que flota en la red.
// Recogerlo lo "traduce" y suma puntos (nivel 1).
class NodoDato : public Entidad {
private:
	int fragmentoID; // que fragmento del manuscrito contiene

public:
	NodoDato(int px, int py, int id);
	int getFragmentoID();
	void animar();
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};

// Pilar de informacion del nivel 3: hay que activarlo manteniendo E cerca.
// El jefe puede corromperlo y drenar su progreso.
class PilarInformacion : public Entidad {
private:
	double progreso;   // 0..100
	bool corrompiendo; // el jefe lo esta drenando

public:
	PilarInformacion(int px, int py);
	double getProgreso();
	void cargar(double cantidad);
	void drenar(double cantidad);
	bool estaCompleto();
	bool getCorrompiendo();
	void setCorrompiendo(bool value);
	void animar();
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};

// Santuario de Macahuisa: meta de la ofrenda digital del nivel 2.
class SantuarioMacahuisa : public Entidad {
public:
	SantuarioMacahuisa(int px, int py);
	void animar();
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};

// Algoritmo ancestral: espiritu aliado que acompana y guia al jugador.
class AliadoAlgoritmo : public Entidad {
public:
	AliadoAlgoritmo();
	void seguir(int jugadorX, int jugadorY);
	void mostrar(Graphics^ g, Bitmap^ img, int camaraX, int camaraY);
};
