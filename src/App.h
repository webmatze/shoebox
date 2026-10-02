#ifndef SHOEBOX_APP_H
#define SHOEBOX_APP_H

#include <Application.h>

class MainWindow;

class App : public BApplication {
public:
	App();

	virtual void ReadyToRun();

private:
	MainWindow* fMainWindow;
};

#endif
