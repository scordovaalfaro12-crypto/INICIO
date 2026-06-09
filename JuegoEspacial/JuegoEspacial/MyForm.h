#pragma once
#include "Juego.h"

namespace JuegoEspacial {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Ventana principal: TODO el juego ocurre dentro de este unico formulario.
	/// El temporizador avanza la logica y el evento Paint dibuja cada cuadro.
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();
			// doble buffer para que el dibujo no parpadee
			this->SetStyle(ControlStyles::AllPaintingInWmPaint
				| ControlStyles::UserPaint
				| ControlStyles::OptimizedDoubleBuffer, true);
			this->UpdateStyles();
			juego = gcnew Juego();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}

		// las flechas tambien deben llegar al evento KeyDown
		virtual bool ProcessDialogKey(System::Windows::Forms::Keys keyData) override
		{
			Keys codigo = keyData & Keys::KeyCode;
			if (codigo == Keys::Up || codigo == Keys::Down || codigo == Keys::Left || codigo == Keys::Right)
				return false;
			return Form::ProcessDialogKey(keyData);
		}

	private: System::Windows::Forms::Timer^ temporizador;
	private: System::ComponentModel::IContainer^ components;

	private:
		Juego^ juego;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->temporizador = (gcnew System::Windows::Forms::Timer(this->components));
			this->SuspendLayout();
			//
			// temporizador (~60 cuadros por segundo)
			//
			this->temporizador->Enabled = true;
			this->temporizador->Interval = 16;
			this->temporizador->Tick += gcnew System::EventHandler(this, &MyForm::alTick);
			//
			// MyForm
			//
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::None;
			this->BackColor = System::Drawing::Color::Black;
			this->ClientSize = System::Drawing::Size(1000, 700);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->MaximizeBox = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->KeyPreview = true;
			this->Name = L"MyForm";
			this->Text = L"Odisea Espacial - 100% figuras geometricas";
			this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MyForm::alPintar);
			this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &MyForm::alTeclaAbajo);
			this->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &MyForm::alTeclaArriba);
			this->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::alMoverRaton);
			this->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::alClic);
			this->ResumeLayout(false);
		}
#pragma endregion

	private: System::Void alTick(System::Object^ sender, System::EventArgs^ e) {
		juego->actualizar();
		this->Invalidate();
	}

	private: System::Void alPintar(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
		e->Graphics->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;
		e->Graphics->TextRenderingHint = System::Drawing::Text::TextRenderingHint::AntiAlias;
		juego->dibujar(e->Graphics);
	}

	private: System::Void alTeclaAbajo(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e) {
		juego->teclaAbajo(e->KeyCode);
	}

	private: System::Void alTeclaArriba(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e) {
		juego->teclaArriba(e->KeyCode);
	}

	private: System::Void alMoverRaton(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
		juego->ratonMovido(e->X, e->Y);
	}

	private: System::Void alClic(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
		juego->clic(e->X, e->Y);
	}
	};
}
