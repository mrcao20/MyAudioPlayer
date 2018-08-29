#pragma once
#pragma execution_character_set("UTF-8")
#include <QBitmap>
#include <QWidget>

class QPainter;
class QPainterPath;
class QRegion;
class QTimer;
class QFont;
class Widget : public QWidget
{
	Q_OBJECT

public:
	Widget(QWidget *parent = 0);
	~Widget();
protected:
	virtual void paintEvent(QPaintEvent *e);
	protected slots:
	void drawMusicWord();
private:
	QStringList list;
	QBitmap bmp;
	QPainter *pBmp;
	QPainterPath *path;
	QRect left;
	QRect right;
	QTimer *timer;
	QFont *font;
	QRect r;
	int index;
	double sp;
	int m;
};