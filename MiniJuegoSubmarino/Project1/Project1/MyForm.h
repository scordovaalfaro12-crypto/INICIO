#pragma once
// ============================================================
//  MyForm.h
//  Formulario principal: crea el Juego, redibuja con un Timer
//  y le envia las teclas presionadas / soltadas.
// ============================================================
#include "Juego.h"

namespace Project1 {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Resumen de MyForm
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
	private:
		Juego^ juego;
		System::Windows::Forms::Timer^ temporizador;

	public:
		MyForm(void)
		{
			InitializeComponent();

			// dibujo con doble buffer para que no parpadee
			this->SetStyle(ControlStyles::AllPaintingInWmPaint |
				ControlStyles::UserPaint |
				ControlStyles::OptimizedDoubleBuffer, true);
			this->UpdateStyles();

			juego = gcnew Juego(this->ClientSize.Width, this->ClientSize.Height);

			temporizador = gcnew System::Windows::Forms::Timer();
			temporizador->Interval = 30;   // ~33 cuadros por segundo
			temporizador->Tick += gcnew EventHandler(this, &MyForm::Temporizador_Tick);
			temporizador->Start();
		}

	protected:
		/// <summary>
		/// Limpiar los recursos que se estan usando.
		/// </summary>
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}

		// un ciclo del juego por cada tick del timer
		void Temporizador_Tick(Object^ sender, EventArgs^ e)
		{
			juego->Actualizar();
			this->Invalidate();
		}

		virtual void OnPaint(PaintEventArgs^ e) override
		{
			Form::OnPaint(e);
			juego->Dibujar(e->Graphics);
		}

		virtual void OnKeyDown(KeyEventArgs^ e) override
		{
			Form::OnKeyDown(e);
			juego->TeclaPresionada(e->KeyCode);
		}

		virtual void OnKeyUp(KeyEventArgs^ e) override
		{
			Form::OnKeyUp(e);
			juego->TeclaSoltada(e->KeyCode);
		}

		// permite que las flechas lleguen a OnKeyDown
		virtual bool IsInputKey(Keys keyData) override
		{
			return true;
		}

	private:
		/// <summary>
		/// Variable del disenador necesaria.
		/// </summary>
		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Metodo necesario para admitir el Disenador. No se puede modificar
		/// el contenido de este metodo con el editor de codigo.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = gcnew System::ComponentModel::Container();
			this->ClientSize = System::Drawing::Size(1000, 640);
			this->Text = L"Mini Juego - Submarino";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->MaximizeBox = false;
			this->KeyPreview = true;
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
		}
#pragma endregion
	};
}
