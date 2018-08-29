#pragma once
#pragma execution_character_set("utf-8")

#include <qmenu.h>

QT_BEGIN_NAMESPACE
class QAction;
QT_END_NAMESPACE

class TrayMenu : public QMenu {
	Q_OBJECT

public:
	TrayMenu(QWidget * = 0);

signals:
	void showWidget();

private slots:
	void quit();

private:
	QAction * quit_action;
	QAction * show_action;

};