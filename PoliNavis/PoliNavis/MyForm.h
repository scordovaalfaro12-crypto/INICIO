#pragma once
#include "Juego.h"

namespace PoliNavis {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Windows::Forms;
	using namespace System::Drawing;

	// Ventana UNICA del juego. El temporizador avanza la logica y el evento
	// Paint dibuja cada cuadro. Reenvia teclado y raton al controlador Juego.
	public ref class MyForm : public System::Windows::Forms::Form {
	public:
		MyForm(void) {
			InitializeComponent();
			this->SetStyle(ControlStyles::AllPaintingInWmPaint
				| ControlStyles::UserPaint
				| ControlStyles::OptimizedDoubleBuffer, true);
			this->UpdateStyles();
			juego = gcnew Juego();
		}

	protected:
		~MyForm() { if (components) delete components; }

		// dejar pasar las flechas al evento KeyDown
		virtual bool ProcessDialogKey(Keys keyData) override {
			Keys c = keyData & Keys::KeyCode;
			if (c == Keys::Up || c == Keys::Down || c == Keys::Left || c == Keys::Right)
				return false;
			return Form::ProcessDialogKey(keyData);
		}

	private:
		System::Windows::Forms::Timer^ temporizador;
		System::ComponentModel::IContainer^ components;
		Juego^ juego;

		void InitializeComponent(void) {
			this->components = (gcnew System::ComponentModel::Container());
			this->temporizador = (gcnew System::Windows::Forms::Timer(this->components));
			this->SuspendLayout();
			this->temporizador->Enabled = true;
			this->temporizador->Interval = 16;     // ~60 cuadros por segundo
			this->temporizador->Tick += gcnew System::EventHandler(this, &MyForm::alTick);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::None;
			this->BackColor = System::Drawing::Color::Black;
			this->ClientSize = System::Drawing::Size(1000, 700);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->MaximizeBox = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->KeyPreview = true;
			this->Name = L"MyForm";
			this->Text = L"PoliNavis - Viaje Interestelar";
			this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MyForm::alPintar);
			this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &MyForm::alTeclaAbajo);
			this->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &MyForm::alTeclaArriba);
			this->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::alMoverRaton);
			this->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::alClic);
			this->ResumeLayout(false);
		}

		System::Void alTick(System::Object^ sender, System::EventArgs^ e) {
			juego->actualizar();
			this->Invalidate();
		}
		System::Void alPintar(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
			e->Graphics->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;
			e->Graphics->TextRenderingHint = System::Drawing::Text::TextRenderingHint::AntiAlias;
			juego->dibujar(e->Graphics);
		}
		System::Void alTeclaAbajo(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e) {
			juego->teclaAbajo(e->KeyCode);
		}
		System::Void alTeclaArriba(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e) {
			juego->teclaArriba(e->KeyCode);
		}
		System::Void alMoverRaton(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
			juego->ratonMovido(e->X, e->Y);
		}
		System::Void alClic(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
			juego->clic(e->X, e->Y);
		}
	};
}
