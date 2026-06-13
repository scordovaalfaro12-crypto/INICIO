#pragma once
#include "Planeta.h"

// Los 8 planetas concretos. Cada uno HEREDA de Planeta y solo reimplementa
// su aspecto (POLIMORFISMO). Tierra y Saturno son obligatorios y los mas
// detallados. Constructor: (solX, solY, radioOrbita, anguloIni, velOrbita,
// velRotacion, radio).
namespace PoliNavis {

	public ref class Mercurio : public Planeta {
	public:
		Mercurio(float sx, float sy, float ro, float a0);
		virtual void dibujarSuperficie(Graphics^ g, float r, float rot) override;
	};

	public ref class Venus : public Planeta {
	public:
		Venus(float sx, float sy, float ro, float a0);
		virtual void dibujarSuperficie(Graphics^ g, float r, float rot) override;
	};

	public ref class Tierra : public Planeta {
	public:
		Tierra(float sx, float sy, float ro, float a0);
		virtual void dibujarSuperficie(Graphics^ g, float r, float rot) override;
		virtual void dibujarExtra(Graphics^ g, float r) override;   // la Luna
	private:
		float lunaAng;
	};

	public ref class Marte : public Planeta {
	public:
		Marte(float sx, float sy, float ro, float a0);
		virtual void dibujarSuperficie(Graphics^ g, float r, float rot) override;
	};

	public ref class Jupiter : public Planeta {
	public:
		Jupiter(float sx, float sy, float ro, float a0);
		virtual void dibujarSuperficie(Graphics^ g, float r, float rot) override;
	};

	public ref class Saturno : public Planeta {
	public:
		Saturno(float sx, float sy, float ro, float a0);
		virtual void dibujarSuperficie(Graphics^ g, float r, float rot) override;
		virtual void dibujarExtra(Graphics^ g, float r) override;   // los anillos
	};

	public ref class Urano : public Planeta {
	public:
		Urano(float sx, float sy, float ro, float a0);
		virtual void dibujarSuperficie(Graphics^ g, float r, float rot) override;
		virtual void dibujarExtra(Graphics^ g, float r) override;   // anillo vertical
	};

	public ref class Neptuno : public Planeta {
	public:
		Neptuno(float sx, float sy, float ro, float a0);
		virtual void dibujarSuperficie(Graphics^ g, float r, float rot) override;
	};
}
