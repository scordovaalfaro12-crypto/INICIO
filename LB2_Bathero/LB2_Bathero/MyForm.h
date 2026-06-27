#pragma once
//==============================================================================
//  MyForm.h  -  Declaracion del formulario principal (Windows Forms / C++ CLR)
//------------------------------------------------------------------------------
//  Crea el lienzo, el temporizador (cronometro) y delega TODA la logica del
//  juego al objeto Controlador. La captura del teclado se hace en WndProc para
//  que funcione de forma fiable (independiente del foco de los controles).
//==============================================================================
#include "Controlador.h"

namespace LB2_Bathero {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class MyForm : public System::Windows::Forms::Form {
	public:
		MyForm(void) {
			InitializeComponent();
			ctrl = gcnew Controlador(ANCHO, ALTO);
		}

	protected:
		~MyForm() {
			if (components) delete components;
			if (ctrl != nullptr) ctrl->Liberar(); // liberar recursos del juego
		}

		// Captura de TODAS las teclas a nivel de mensajes de Windows.
		virtual void WndProc(System::Windows::Forms::Message% m) override;

	private:
		static const int ANCHO = 1000;
		static const int ALTO = 600;

		Controlador^ ctrl;
		System::Windows::Forms::Panel^ lienzo;
		System::Windows::Forms::Timer^ cronometro;
		System::ComponentModel::IContainer^ components;

		// Manejadores de eventos (implementados en MyForm.cpp).
		void MyForm_Load(System::Object^ sender, System::EventArgs^ e);
		void cronometro_Tick(System::Object^ sender, System::EventArgs^ e);

#pragma region Windows Form Designer generated code
		void InitializeComponent(void) {
			this->components = gcnew System::ComponentModel::Container();
			this->lienzo = gcnew System::Windows::Forms::Panel();
			this->cronometro = gcnew System::Windows::Forms::Timer(this->components);
			this->SuspendLayout();
			//
			// lienzo
			//
			this->lienzo->BackColor = System::Drawing::Color::Black;
			this->lienzo->Location = System::Drawing::Point(0, 0);
			this->lienzo->Name = L"lienzo";
			this->lienzo->Size = System::Drawing::Size(ANCHO, ALTO);
			this->lienzo->TabStop = false;
			//
			// cronometro
			//
			this->cronometro->Interval = 20; // ~50 cuadros por segundo
			this->cronometro->Tick += gcnew System::EventHandler(this, &MyForm::cronometro_Tick);
			//
			// MyForm
			//
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::None;
			this->ClientSize = System::Drawing::Size(ANCHO, ALTO);
			this->Controls->Add(this->lienzo);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->KeyPreview = true;
			this->MaximizeBox = false;
			this->Name = L"MyForm";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"LB2 - Bathero atrapa villanos con su bumeran";
			this->Load += gcnew System::EventHandler(this, &MyForm::MyForm_Load);
			this->ResumeLayout(false);
		}
#pragma endregion
	};
}
