#pragma once
#include "Entidad.h"

// Personaje jugable: el Ciber-Guardian encarnado en la deidad de la lluvia.
// El sprite sheet tiene 9 columnas x 7 filas. Las filas laterales miran a la
// izquierda; para caminar a la derecha se usa la hoja espejada.
class DeidadLluvia : public Entidad {
public:
	enum SpriteDeidadLluvia {
		caminarIzquierda,
		caminarDerecha,
		caminarArriba,
		caminarAbajo,
		reposo,
		ataque,
		muerte
	};

private:
	SpriteDeidadLluvia accion;
	int invulnerable;   // ticks restantes sin recibir dano
	bool animTerminada; // true cuando una animacion sin bucle (ataque/muerte) acabo
	bool conPoder;      // Poder de los Apus (Ray-Trace) desbloqueado
	bool conOfrenda;    // lleva la ofrenda digital (nivel 2)
	int miradaX, miradaY; // ultima direccion de la mirada (para disparar)

	int filaDe(SpriteDeidadLluvia a);       // fila real en el sprite sheet
	int framesDe(SpriteDeidadLluvia a);     // cantidad de frames de esa fila
	bool espejadaDe(SpriteDeidadLluvia a);  // si se dibuja con la hoja espejo
	bool repiteDe(SpriteDeidadLluvia a);    // si la animacion es en bucle
	int velocidadAnimDe(SpriteDeidadLluvia a);

public:
	DeidadLluvia(Bitmap^ img);

	SpriteDeidadLluvia getAccion();
	void setAccion(SpriteDeidadLluvia value);

	bool getAnimTerminada();
	bool getConPoder();
	void setConPoder(bool value);
	bool getConOfrenda();
	void setConOfrenda(bool value);
	bool esInvulnerable();
	int getMiradaX();
	int getMiradaY();

	void recibirGolpe();                       // resta vida y activa invulnerabilidad
	void actualizarAccionPorMovimiento();      // elige animacion segun dx/dy
	void moverEnMundo(int mundoAncho, int mundoAlto);
	Rectangle hitbox();

	void actualizarAnimacion();
	void mostrar(Graphics^ g, Bitmap^ img, Bitmap^ imgEspejo, int camaraX, int camaraY);
};
