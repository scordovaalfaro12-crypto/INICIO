#include "Item.h"

namespace JuegoEspacial {

	Item::Item(float px, float py, int pvalor) : EntidadEspacial(px, py) {
		valor = pvalor;
	}

	int Item::getValor() {
		return valor;
	}
}
