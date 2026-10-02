#include "App.h"

#include "IndexSetup.h"
#include "MainWindow.h"

static const char* kAppSignature = "application/x-vnd.user-Shoebox";

App::App()
	: BApplication(kAppSignature),
	  fMainWindow(nullptr)
{
}


void
App::ReadyToRun()
{
	SetupShoeboxIndices();

	fMainWindow = new MainWindow();
	fMainWindow->Show();
}


int
main()
{
	App app;
	app.Run();
	return 0;
}
