#include "GestorArchivos.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <direct.h>   // _mkdir

GestorArchivos::GestorArchivos() {
	carpeta = "FILES";
	rutaParametros = "FILES/PARAMETERS.txt";
	rutaPuntajes = "FILES/SCORES.bin";
	ultimoError = "";
}

void GestorArchivos::registrarError(const std::string& mensaje) {
	// Acumula los problemas encontrados para mostrarlos juntos al usuario
	if (!ultimoError.empty()) ultimoError += "\n";
	ultimoError += mensaje;
}

bool GestorArchivos::huboError() { return !ultimoError.empty(); }
std::string GestorArchivos::getUltimoError() { return ultimoError; }
void GestorArchivos::limpiarError() { ultimoError = ""; }

void GestorArchivos::asegurarCarpeta() {
	// _mkdir devuelve 0 si la crea; si ya existe no es un error real
	_mkdir(carpeta.c_str());
}

Configuracion GestorArchivos::valoresPorDefecto() {
	Configuracion c;
	c.vidas = 3;
	c.tiempo[0] = 180; c.tiempo[1] = 200; c.tiempo[2] = 260;
	c.enemigos[0] = 5; c.enemigos[1] = 5; c.enemigos[2] = 2;
	c.aliados = 1;
	c.nodosNivel1 = 7;
	c.pilaresNivel3 = 3;
	return c;
}

void GestorArchivos::escribirParametros(const Configuracion& c) {
	asegurarCarpeta();
	std::ofstream archivo(rutaParametros.c_str());
	if (!archivo.is_open()) {
		registrarError("No se pudo crear FILES/PARAMETERS.txt");
		return;
	}
	archivo << "# Configuracion del juego Manuscrito de Huarochiri\n";
	archivo << "# Edita estos valores para cambiar la dificultad.\n";
	archivo << "vidas=" << c.vidas << "\n";
	archivo << "tiempoNivel1=" << c.tiempo[0] << "\n";
	archivo << "tiempoNivel2=" << c.tiempo[1] << "\n";
	archivo << "tiempoNivel3=" << c.tiempo[2] << "\n";
	archivo << "enemigosNivel1=" << c.enemigos[0] << "\n";
	archivo << "enemigosNivel2=" << c.enemigos[1] << "\n";
	archivo << "enemigosNivel3=" << c.enemigos[2] << "\n";
	archivo << "aliados=" << c.aliados << "\n";
	archivo << "nodosNivel1=" << c.nodosNivel1 << "\n";
	archivo << "pilaresNivel3=" << c.pilaresNivel3 << "\n";
	archivo.close();
}

Configuracion GestorArchivos::cargarParametros() {
	asegurarCarpeta();
	Configuracion c = valoresPorDefecto();

	std::ifstream archivo(rutaParametros.c_str());
	if (!archivo.is_open()) {
		// No existe: lo creamos con valores por defecto (no es error fatal)
		escribirParametros(c);
		return c;
	}

	int leidos = 0;
	std::string linea;
	while (std::getline(archivo, linea)) {
		if (linea.empty() || linea[0] == '#') continue;
		std::string::size_type pos = linea.find('=');
		if (pos == std::string::npos) continue;
		std::string clave = linea.substr(0, pos);
		std::string valorTexto = linea.substr(pos + 1);

		int valor;
		std::istringstream conv(valorTexto);
		if (!(conv >> valor) || valor < 0) {
			registrarError("Valor invalido en PARAMETERS.txt: " + linea);
			continue;
		}

		if (clave == "vidas") { c.vidas = valor; leidos++; }
		else if (clave == "tiempoNivel1") { c.tiempo[0] = valor; leidos++; }
		else if (clave == "tiempoNivel2") { c.tiempo[1] = valor; leidos++; }
		else if (clave == "tiempoNivel3") { c.tiempo[2] = valor; leidos++; }
		else if (clave == "enemigosNivel1") { c.enemigos[0] = valor; leidos++; }
		else if (clave == "enemigosNivel2") { c.enemigos[1] = valor; leidos++; }
		else if (clave == "enemigosNivel3") { c.enemigos[2] = valor; leidos++; }
		else if (clave == "aliados") { c.aliados = valor; leidos++; }
		else if (clave == "nodosNivel1") { c.nodosNivel1 = valor; leidos++; }
		else if (clave == "pilaresNivel3") { c.pilaresNivel3 = valor; leidos++; }
	}
	archivo.close();

	// Integridad: rangos sensatos para no romper el diseno de los niveles
	if (c.vidas < 1 || c.vidas > 9) { c.vidas = 3; registrarError("vidas fuera de rango, usando 3"); }
	for (int i = 0; i < 3; i++) {
		if (c.tiempo[i] < 30 || c.tiempo[i] > 999) c.tiempo[i] = valoresPorDefecto().tiempo[i];
		if (c.enemigos[i] < 0 || c.enemigos[i] > 30) c.enemigos[i] = valoresPorDefecto().enemigos[i];
	}
	if (c.nodosNivel1 < 1 || c.nodosNivel1 > 7) c.nodosNivel1 = 7;
	if (c.pilaresNivel3 < 1 || c.pilaresNivel3 > 3) c.pilaresNivel3 = 3;

	if (leidos == 0) {
		registrarError("PARAMETERS.txt no contiene parametros validos; se regenero por defecto");
		c = valoresPorDefecto();
		escribirParametros(c);
	}
	return c;
}

bool GestorArchivos::guardarPuntaje(const Puntaje& p) {
	asegurarCarpeta();
	// Modo append binario: cada partida agrega su registro
	std::ofstream archivo(rutaPuntajes.c_str(), std::ios::binary | std::ios::app);
	if (!archivo.is_open()) {
		registrarError("No se pudo escribir FILES/SCORES.bin");
		return false;
	}
	archivo.write(reinterpret_cast<const char*>(&p), sizeof(Puntaje));
	archivo.close();
	return true;
}

std::vector<Puntaje> GestorArchivos::cargarPuntajes() {
	std::vector<Puntaje> lista;
	std::ifstream archivo(rutaPuntajes.c_str(), std::ios::binary | std::ios::ate);
	if (!archivo.is_open()) {
		return lista; // aun no hay puntajes: normal en la primera partida
	}

	std::streamoff tam = archivo.tellg();
	// Integridad: el tamano debe ser multiplo exacto del registro
	if (tam % (std::streamoff)sizeof(Puntaje) != 0) {
		registrarError("SCORES.bin esta corrupto (tamano invalido); se ignoraran los datos danados");
		tam -= (tam % (std::streamoff)sizeof(Puntaje));
	}

	archivo.seekg(0, std::ios::beg);
	int total = (int)(tam / (std::streamoff)sizeof(Puntaje));
	for (int i = 0; i < total; i++) {
		Puntaje p;
		archivo.read(reinterpret_cast<char*>(&p), sizeof(Puntaje));
		if (archivo.gcount() != (std::streamsize)sizeof(Puntaje)) break;
		p.nombre[23] = '\0';
		p.fecha[11] = '\0';
		lista.push_back(p);
	}
	archivo.close();

	// Mayor puntaje primero
	std::sort(lista.begin(), lista.end(),
		[](const Puntaje& a, const Puntaje& b) { return a.puntaje > b.puntaje; });
	return lista;
}
