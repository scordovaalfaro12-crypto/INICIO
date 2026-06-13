#include "Estilos.h"

namespace PoliNavis {

	StringFormat^ Estilos::crearFormato(StringAlignment horizontal, StringAlignment vertical) {
		StringFormat^ formato = gcnew StringFormat();
		formato->Alignment = horizontal;
		formato->LineAlignment = vertical;
		return formato;
	}

	void Estilos::agregarRectRedondeado(GraphicsPath^ ruta, float x, float y, float ancho, float alto, float radio) {
		float d = radio * 2.0f;
		ruta->AddArc(x, y, d, d, 180.0f, 90.0f);
		ruta->AddArc(x + ancho - d, y, d, d, 270.0f, 90.0f);
		ruta->AddArc(x + ancho - d, y + alto - d, d, d, 0.0f, 90.0f);
		ruta->AddArc(x, y + alto - d, d, d, 90.0f, 90.0f);
		ruta->CloseFigure();
	}

	void Estilos::rellenarRedondeado(Graphics^ g, Color color, float x, float y, float ancho, float alto, float radio) {
		GraphicsPath ruta;
		agregarRectRedondeado(%ruta, x, y, ancho, alto, radio);
		SolidBrush pincel(color);
		g->FillPath(%pincel, %ruta);
	}

	void Estilos::bordeRedondeado(Graphics^ g, Color color, float grosor, float x, float y, float ancho, float alto, float radio) {
		GraphicsPath ruta;
		agregarRectRedondeado(%ruta, x, y, ancho, alto, radio);
		Pen pluma(color, grosor);
		g->DrawPath(%pluma, %ruta);
	}

	void Estilos::textoCentrado(Graphics^ g, String^ texto, Font^ fuente, Color color, float cx, float cy) {
		SolidBrush pincel(color);
		g->DrawString(texto, fuente, %pincel, PointF(cx, cy), centrado);
	}

	void Estilos::textoSombra(Graphics^ g, String^ texto, Font^ fuente, Color color, float cx, float cy) {
		SolidBrush sombra(Color::FromArgb(170, 0, 0, 0));
		g->DrawString(texto, fuente, %sombra, PointF(cx + 3.0f, cy + 3.0f), centrado);
		SolidBrush pincel(color);
		g->DrawString(texto, fuente, %pincel, PointF(cx, cy), centrado);
	}

	void Estilos::textoIzquierda(Graphics^ g, String^ texto, Font^ fuente, Color color, float x, float y) {
		SolidBrush pincel(color);
		g->DrawString(texto, fuente, %pincel, PointF(x, y), izquierda);
	}

	Color Estilos::oscurecer(Color color, float factor) {
		return Color::FromArgb(color.A,
			(int)(color.R * factor),
			(int)(color.G * factor),
			(int)(color.B * factor));
	}

	Color Estilos::aclarar(Color color, float factor) {
		return Color::FromArgb(color.A,
			(int)(color.R + (255 - color.R) * factor),
			(int)(color.G + (255 - color.G) * factor),
			(int)(color.B + (255 - color.B) * factor));
	}
}
