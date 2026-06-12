#include "MenuForm.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThread]
int main(cli::array<String^>^ args)
{
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	Application::Run(gcnew ManuscritoHuarochiri::MenuForm());
	return 0;
}
