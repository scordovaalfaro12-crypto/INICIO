//==============================================================================
//  MyForm.cpp  -  PROGRAMA PRINCIPAL (punto de entrada de la aplicacion)
//------------------------------------------------------------------------------
//  Crea el formulario, lo ejecuta y lo libera correctamente al cerrar.
//==============================================================================
#include "MyForm.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThreadAttribute]
int main(array<System::String^>^ args) {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	LB2_Bathero::MyForm form;     // se crea el objeto formulario
	Application::Run(% form);      // se ejecuta (la animacion corre dentro)
	return 0;                      // al cerrar, los recursos se liberan
}
