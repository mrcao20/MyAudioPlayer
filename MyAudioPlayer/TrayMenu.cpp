#include "TrayMenu.h"
#include <qapplication.h>
#include <qaction.h>

TrayMenu::TrayMenu(QWidget *parent) 
	: QMenu(parent) 
	, quit_action(new QAction(this))
	, show_action(new QAction(this)){

	show_action->setText("Show");
	quit_action->setText("Quit");

	addAction(show_action);
	addSeparator();
	addAction(quit_action);

	connect(show_action, &QAction::triggered, this, &TrayMenu::showWidget);
	connect(quit_action, SIGNAL(triggered(bool)), this, SLOT(quit()));
}

void TrayMenu::quit() {
	QApplication::exit(0);
}