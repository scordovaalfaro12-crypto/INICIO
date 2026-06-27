//==============================================================================
//  MyForm.cpp  -  PROGRAMA PRINCIPAL e implementacion de los eventos del form
//------------------------------------------------------------------------------
//  Crea el formulario, lo ejecuta y lo libera correctamente al cerrar.
//==============================================================================
#include "MyForm.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace LB2_Bathero {

	// Inicia el cronometro al cargar el formulario.
	void MyForm::MyForm_Load(System::Object^ sender, System::EventArgs^ e) {
		this->cronometro->Start();
		this->Focus();
	}

	// Bucle principal de la animacion.
	void MyForm::cronometro_Tick(System::Object^ sender, System::EventArgs^ e) {
		ctrl->Actualizar();   // logica (movimiento, colisiones)
		ctrl->Dibujar();      // dibuja todo sobre el buffer

		// Volcado del buffer al lienzo en una sola operacion (sin parpadeo).
		Graphics^ gl = lienzo->CreateGraphics();
		gl->DrawImage(ctrl->Buffer, 0, 0);
		delete gl;

		// Fin de la simulacion: se muestra el reporte.
		if (ctrl->Terminado) {
			cronometro->Stop();
			MessageBox::Show(ctrl->Reporte(), "Reporte final",
				MessageBoxButtons::OK, MessageBoxIcon::Information);
			this->Close();
		}
	}

	void MyForm::MyForm_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e) {
		ctrl->TeclaAbajo(e->KeyCode);
	}

	void MyForm::MyForm_KeyUp(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e) {
		ctrl->TeclaArriba(e->KeyCode);
	}

	// Permite que las teclas de flecha lleguen al evento KeyDown.
	void MyForm::MyForm_PreviewKeyDown(System::Object^ sender, System::Windows::Forms::PreviewKeyDownEventArgs^ e) {
		if (e->KeyCode == Keys::Up || e->KeyCode == Keys::Down ||
			e->KeyCode == Keys::Left || e->KeyCode == Keys::Right)
			e->IsInputKey = true;
	}
}

//------------------------------ PUNTO DE ENTRADA ------------------------------
[STAThreadAttribute]
int main(array<System::String^>^ args) {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	LB2_Bathero::MyForm form;     // se crea el objeto formulario
	Application::Run(% form);      // se ejecuta (la animacion corre dentro)
	return 0;                      // al cerrar, los recursos se liberan
}
