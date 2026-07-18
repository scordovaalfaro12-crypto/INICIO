#pragma once
#include <vector>
#include <string>
#include "Guardabosques.h"
#include "Dinosaurio.h"
#include "Velocirraptor.h"
#include "Pteranodonte.h"
#include "DinosaurioAlfa.h"
#include "Dardo.h"
#include "Caja.h"
#include "Helicoptero.h"

using namespace System;
using namespace System::Drawing;

// Controlador: administra el buffer de dibujo (para evitar el parpadeo),
// el ciclo de animacion (metodo actualizar, invocado por el Timer del
// formulario) y la lectura de teclas descrita en el enunciado.
// Tambien crea/libera todos los objetos del juego, resuelve las colisiones
// y genera el archivo RESULTADOS.TXT mediante manejo de archivos de texto.
class Controlador
{
public:
	static const int ANCHO_VENTANA = 1100;
	static const int ALTO_VENTANA = 640;
	static const int HUD_ALTO = 45;
	static const int INTERVALO_MS = 30;

	// Estados del juego
	static const int JUGANDO = 0;
	static const int GANADO = 1;
	static const int PERDIDO = 2;

private:
	Guardabosques* guarda;
	Helicoptero* helicoptero;
	DinosaurioAlfa* alfa;                    // tambien registrado en "dinosaurios"
	std::vector<Dinosaurio*> dinosaurios;    // manejo polimorfico de las especies
	std::vector<Dardo*> dardos;
	std::vector<Caja*> cajas;

	int estado;
	int ticks;                 // ticks transcurridos (tiempo de juego)
	bool arriba, abajo, izquierda, derecha;
	bool pteranodontesGenerados;
	bool alfaGenerado;
	int totalGenerados;        // velocirraptores + pteranodones (sin el alfa)
	int invulnerableTicks;     // tiempo de gracia tras un golpe del alfa

	// Estadisticas para RESULTADOS.TXT
	int capturadosRaptor;
	int capturadosPtera;
	int capturadosAlfa;
	int dardosRecogidos;
	int dardosDisparados;
	int aciertos;
	int fallos;

	std::string mensaje;
	int mensajeTicks;

public:
	Controlador();
	~Controlador();               // libera correctamente todos los objetos

	Rectangle zonaJuego();
	Rectangle zonaCapturados();

	void teclaPresionada(int codigo);
	void teclaLiberada(int codigo);

	void actualizar();            // un paso del ciclo de animacion
	void dibujar(BufferedGraphics^ buffer, Bitmap^ bmpGuarda, Bitmap^ bmpRaptor,
		Bitmap^ bmpPtera, Bitmap^ bmpAlfa, Bitmap^ bmpDardo, Bitmap^ bmpHeli);

	int getEstado();
	int getTiempoSegundos();
	void guardarResultados();     // genera RESULTADOS.TXT en modo texto
	String^ getResumenPartida();  // resumen para mostrar al finalizar el juego

private:
	void generarVelocirraptores();
	void generarPteranodontes();
	void generarCajas(int minimo, int maximo);
	void disparar();
	void moverDardos();
	void revisarColisiones();
	void revisarAlfaYFinDeJuego();
	void mostrarMensaje(const std::string& texto);
	int capturadosNormales();

	void dibujarFondo(Graphics^ g);
	void dibujarHUD(Graphics^ g);
	void dibujarMensajes(Graphics^ g);
};
