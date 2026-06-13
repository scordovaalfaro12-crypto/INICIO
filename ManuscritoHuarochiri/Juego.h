#pragma once
#include "DeidadLluvia.h"
#include "LlamaDigital.h"
#include "HuallalloJefe.h"
#include "ObjetosNivel.h"
#include "RayoTrace.h"
#include "Decoraciones.h"

// Clase controladora: administra los 3 niveles, el estado del juego,
// la camara, el HUD, el audio y todas las entidades.
ref class Juego {
public:
	enum class Estado {
		IntroNivel,      // panel con la historia del nivel
		Jugando,
		Pausa,
		NivelCompletado, // panel con la pregunta reflexiva
		Victoria,        // YOU WIN
		Derrota          // GAME OVER
	};

private:
	// ----- Recursos -----
	Bitmap^ imgDeidadLluvia;
	Bitmap^ imgDeidadEspejo;   // hoja espejada para caminar a la derecha
	Bitmap^ imgEscenario1;     // mapa oscuro (niveles 1 y 3)
	Bitmap^ imgEscenario2;     // mapa claro (nivel 2)
	Bitmap^ fondoActual;

	WMPLib::WindowsMediaPlayerClass^ reproductorMusica;
	WMPLib::WindowsMediaPlayerClass^ reproductorEfectos;

	Font^ fuenteTitulo;
	Font^ fuenteGrande;
	Font^ fuenteNormal;
	Font^ fuenteChica;
	SolidBrush^ velOscuro;     // velos y paneles
	SolidBrush^ velRojo;
	SolidBrush^ velCorrupcion;
	SolidBrush^ brochaGlobo;       // globo de dialogo de los espiritus
	SolidBrush^ brochaTextoGlobo;
	Pen^ plumaGlobo;
	StringFormat^ centrado;

	// ----- Entidades (clases nativas, demuestran herencia) -----
	DeidadLluvia* jugador;
	vector<LlamaDigital*>* enemigos;
	vector<NodoDato*>* nodos;
	vector<PilarInformacion*>* pilares;
	vector<RayoTrace*>* rayos;
	vector<Decoracion*>* decoraciones;        // ambiente animado (polimorfico)
	vector<PersonajeSecundario*>* espiritus;  // NPCs que ensenan la historia
	vector<BolaFuego*>* bolasFuego;           // proyectiles de Huallallo
	vector<Particula*>* particulas;           // chispas de estallidos
	vector<TextoFlotante*>* textosFlotantes;  // "+100" al ganar puntos
	HuallalloJefe* jefe;
	SantuarioMacahuisa* santuario;
	AliadoAlgoritmo* aliado;

	// ----- Estado general -----
	Estado estado;
	int nivelActual;
	String^ tituloNivel;
	int score;
	int scoreInicioNivel;  // para reintentar el nivel sin duplicar puntos
	int tiempoRestante;    // en ticks (33 por segundo aprox.)
	int tickGlobal;
	int ticksTrasMuerte;
	int cooldownRayo;
	int nodosRecogidos;
	int spawnEnemigos;     // contador para invocar esbirros en el nivel 3
	int contadorBolaFuego; // cadencia de disparo del jefe
	bool poderDesbloqueado;
	bool salirAlMenu;

	// ----- Movimiento con aceleracion y camara -----
	int velJugadorX, velJugadorY;
	int camaraX, camaraY, offsetX, offsetY;
	int sacudida;          // ticks de vibracion de camara al recibir dano

	// ----- Punto de reaparicion del nivel -----
	int spawnX, spawnY;

	// ----- Entrada -----
	bool teclaArriba, teclaAbajo, teclaIzquierda, teclaDerecha, teclaInteractuar;

	// ----- Mensajes y textos -----
	String^ mensaje;
	int mensajeTicks;
	String^ fragmentoQuechua;
	String^ fragmentoTraduccion;
	int fragmentoTicks;
	array<String^>^ quechua;
	array<String^>^ traducciones;
	array<String^>^ textosEspiritus; // dialogo de los NPCs del nivel actual
	array<int>^ lluviaY;   // efecto de texto cayendo en la pantalla final
	array<int>^ lluviaVel;

	// ----- Internos -----
	void cargarTextos();
	void limpiarNivel();
	void reproducirEfecto(String^ ruta);
	void tocarMusicaNivel();
	void detenerMusica();
	void mostrarMensaje(String^ texto);
	void estallido(int px, int py, int cantidad, int tono);
	void sumarPuntos(int px, int py, int valor, int tono);
	void perderVidaJugador(int golpeDesdeX, int golpeDesdeY);
	void dispararRayo();
	void completarNivel();
	void actualizarJugando();
	void actualizarNivel1();
	void actualizarNivel2();
	void actualizarNivel3();
	void calcularCamara(int anchoPantalla, int altoPantalla);
	void dibujarMundo(Graphics^ g, int anchoPantalla, int altoPantalla);
	void dibujarFlechaGuia(Graphics^ g);
	void dibujarGlobo(Graphics^ g, int anchoPantalla, int centroX, int baseY, String^ texto);
	void dibujarHUD(Graphics^ g, int anchoPantalla, int altoPantalla);
	void dibujarMinimapa(Graphics^ g, int anchoPantalla, int altoPantalla);
	void dibujarCorazon(Graphics^ g, int px, int py, bool lleno);
	void dibujarPanel(Graphics^ g, int anchoPantalla, int altoPantalla,
		String^ titulo, array<String^>^ lineas, String^ pie, Brush^ colorTitulo);
	void dibujarOverlays(Graphics^ g, int anchoPantalla, int altoPantalla);

public:
	Juego();
	~Juego();

	void iniciarNivel(int numero);
	void teclaCambiada(Keys tecla, bool presionada);
	void actualizar();
	void dibujar(Graphics^ g, int anchoPantalla, int altoPantalla);

	bool getSalirAlMenu();
	Estado getEstado();
};
