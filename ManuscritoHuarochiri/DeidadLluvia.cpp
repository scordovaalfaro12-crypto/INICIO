#include "DeidadLluvia.h"

DeidadLluvia::DeidadLluvia(Bitmap^ img) {
	ancho = img->Width / 9;
	alto = img->Height / 7;
	x = y = 1000;
	vida = 3;
	accion = reposo;
	invulnerable = 0;
	animTerminada = false;
	conPoder = false;
	conOfrenda = false;
	miradaX = 0;
	miradaY = 1; // mirando hacia abajo al iniciar
}

DeidadLluvia::SpriteDeidadLluvia DeidadLluvia::getAccion() { return accion; }

void DeidadLluvia::setAccion(SpriteDeidadLluvia value) {
	if (accion == muerte) return; // la muerte no se interrumpe
	if (accion != value) {
		accion = value;
		IDx = 0;
		delay = 0;
		animTerminada = false;
	}
}

bool DeidadLluvia::getAnimTerminada() { return animTerminada; }
bool DeidadLluvia::getConPoder() { return conPoder; }
void DeidadLluvia::setConPoder(bool value) { conPoder = value; }
bool DeidadLluvia::getConOfrenda() { return conOfrenda; }
void DeidadLluvia::setConOfrenda(bool value) { conOfrenda = value; }
bool DeidadLluvia::esInvulnerable() { return invulnerable > 0; }
int DeidadLluvia::getMiradaX() { return miradaX; }
int DeidadLluvia::getMiradaY() { return miradaY; }

// Mapeo de cada accion a su fila real en el sprite sheet (medido sobre la imagen):
// fila 0 = caminar de espaldas (arriba), fila 1 = caminar lateral (mira izquierda),
// fila 3 = caminar de frente (abajo), fila 4 = reposo, fila 5 = ataque, fila 6 = muerte.
int DeidadLluvia::filaDe(SpriteDeidadLluvia a) {
	switch (a) {
	case caminarArriba:    return 0;
	case caminarIzquierda: return 1;
	case caminarDerecha:   return 1; // misma fila, dibujada en espejo
	case caminarAbajo:     return 3;
	case reposo:           return 4;
	case ataque:           return 5;
	case muerte:           return 6;
	}
	return 4;
}

int DeidadLluvia::framesDe(SpriteDeidadLluvia a) {
	switch (a) {
	case reposo: return 4;  // los primeros frames de la fila son la respiracion
	case ataque: return 6;  // el golpe de hacha usa 6 frames
	case muerte: return 8;  // la fila de muerte solo tiene 8 frames dibujados
	default:     return 9;
	}
}

bool DeidadLluvia::espejadaDe(SpriteDeidadLluvia a) { return a == caminarDerecha; }

bool DeidadLluvia::repiteDe(SpriteDeidadLluvia a) { return a != ataque && a != muerte; }

int DeidadLluvia::velocidadAnimDe(SpriteDeidadLluvia a) {
	switch (a) {
	case reposo: return 9;
	case ataque: return 4;
	case muerte: return 6;
	default:     return 4;
	}
}

void DeidadLluvia::recibirGolpe() {
	if (invulnerable > 0 || accion == muerte) return;
	vida--;
	if (vida <= 0) {
		vida = 0;
		accion = muerte;
		IDx = 0;
		delay = 0;
		animTerminada = false;
	}
	else {
		invulnerable = 50; // ~1.5 segundos parpadeando
	}
}

void DeidadLluvia::actualizarAccionPorMovimiento() {
	if (accion == muerte || accion == ataque) return;
	if (dx < 0)      { setAccion(caminarIzquierda); miradaX = -1; miradaY = 0; }
	else if (dx > 0) { setAccion(caminarDerecha);   miradaX = 1;  miradaY = 0; }
	else if (dy < 0) { setAccion(caminarArriba);    miradaX = 0;  miradaY = -1; }
	else if (dy > 0) { setAccion(caminarAbajo);     miradaX = 0;  miradaY = 1; }
	else             { setAccion(reposo); }
}

void DeidadLluvia::moverEnMundo(int mundoAncho, int mundoAlto) {
	if (accion == muerte) return;
	int margen = 20;
	if (x + dx >= -margen && x + dx + ancho <= mundoAncho + margen) x += dx;
	if (y + dy >= -margen && y + dy + alto <= mundoAlto + margen) y += dy;
}

Rectangle DeidadLluvia::hitbox() {
	// El dibujo tiene mucho margen transparente: la caja real es el torso
	return Rectangle(x + ancho / 4, y + alto / 4, ancho / 2, alto * 3 / 5);
}

void DeidadLluvia::actualizarAnimacion() {
	if (invulnerable > 0) invulnerable--;
	int total = framesDe(accion);
	delay++;
	if (delay >= velocidadAnimDe(accion)) {
		delay = 0;
		if (IDx + 1 >= total) {
			if (repiteDe(accion)) IDx = 0;
			else { animTerminada = true; IDx = total - 1; }
		}
		else IDx++;
	}
}

void DeidadLluvia::mostrar(Graphics^ g, Bitmap^ img, Bitmap^ imgEspejo, int camaraX, int camaraY) {
	// Parpadeo mientras es invulnerable
	if (invulnerable > 0 && (invulnerable / 4) % 2 == 0 && accion != muerte) return;

	int fila = filaDe(accion);
	Rectangle destino = Rectangle(x - camaraX, y - camaraY, ancho, alto);
	Rectangle corte;
	if (espejadaDe(accion)) {
		// En la hoja espejada, la columna IDx queda contada desde la derecha
		corte = Rectangle(imgEspejo->Width - (IDx + 1) * ancho, fila * alto, ancho, alto);
		g->DrawImage(imgEspejo, destino, corte, GraphicsUnit::Pixel);
	}
	else {
		corte = Rectangle(IDx * ancho, fila * alto, ancho, alto);
		g->DrawImage(img, destino, corte, GraphicsUnit::Pixel);
	}

	// Si lleva la ofrenda digital, se dibuja brillando sobre su cabeza
	if (conOfrenda) {
		int ox = x - camaraX + ancho / 2;
		int oy = y - camaraY - 14;
		int pulso = (IDx % 4) * 2;
		g->FillEllipse(Brushes::Gold, ox - 16 - pulso / 2, oy - 16 - pulso / 2, 32 + pulso, 32 + pulso);
		g->FillEllipse(Brushes::Yellow, ox - 9, oy - 9, 18, 18);
		g->DrawEllipse(Pens::Orange, ox - 16 - pulso / 2, oy - 16 - pulso / 2, 32 + pulso, 32 + pulso);
	}
}
