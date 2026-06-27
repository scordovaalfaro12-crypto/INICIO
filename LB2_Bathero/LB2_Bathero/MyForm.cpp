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

	// Captura TODAS las teclas (flechas, E, F, ESPACIO, X) directamente desde los
	// mensajes de Windows. Es mas fiable que el evento KeyDown del formulario.
	void MyForm::WndProc(System::Windows::Forms::Message% m) {
		const int WM_KEYDOWN = 0x0100;
		const int WM_KEYUP = 0x0101;
		const int WM_SYSKEYDOWN = 0x0104;
		const int WM_SYSKEYUP = 0x0105;

		if (ctrl != nullptr) {
			if (m.Msg == WM_KEYDOWN || m.Msg == WM_SYSKEYDOWN)
				ctrl->TeclaAbajo((System::Windows::Forms::Keys)m.WParam.ToInt32());
			else if (m.Msg == WM_KEYUP || m.Msg == WM_SYSKEYUP)
				ctrl->TeclaArriba((System::Windows::Forms::Keys)m.WParam.ToInt32());
		}

		Form::WndProc(m); // procesamiento normal
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
