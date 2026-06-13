#pragma once
#include <string>
#include <vector>

// Gestion de archivos del juego (requisito obligatorio del curso).
//   FILES/PARAMETERS.txt : configuracion en TEXTO, leida al iniciar el juego
//                          (vidas, tiempos, numero de enemigos/aliados, etc.).
//   FILES/SCORES.bin     : puntuaciones en BINARIO (nombre, puntaje, fecha),
//                          leidas y escritas cada vez que se juega.
// Clase nativa que usa las librerias estandar de C++ (fstream). Valida la
// existencia e integridad de ambos archivos y reporta el problema.

// Parametros de configuracion leidos del archivo de texto.
struct Configuracion {
	int vidas;
	int tiempo[3];     // segundos disponibles por nivel
	int enemigos[3];   // enemigos de patrulla por nivel
	int aliados;       // aliados que acompanan
	int nodosNivel1;   // fragmentos a recuperar en el nivel 1
	int pilaresNivel3; // pilares a compilar en el nivel 3
};

// Un registro de puntuacion del archivo binario.
struct Puntaje {
	char nombre[24];
	int puntaje;
	char fecha[12];    // formato aaaa-mm-dd
};

class GestorArchivos {
private:
	std::string carpeta;
	std::string rutaParametros;
	std::string rutaPuntajes;
	std::string ultimoError;

	void registrarError(const std::string& mensaje);

public:
	GestorArchivos();

	bool huboError();
	std::string getUltimoError();
	void limpiarError();

	void asegurarCarpeta();                          // crea FILES/ si no existe
	Configuracion valoresPorDefecto();
	void escribirParametros(const Configuracion& c); // (re)genera PARAMETERS.txt
	Configuracion cargarParametros();                // lee y valida PARAMETERS.txt

	bool guardarPuntaje(const Puntaje& p);           // agrega un registro a SCORES.bin
	std::vector<Puntaje> cargarPuntajes();           // lee y valida SCORES.bin (ordenado)
};
