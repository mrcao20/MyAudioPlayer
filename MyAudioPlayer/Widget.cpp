#include "widget.h"
#include <QtGui>
#include <QDebug>
#include <QRect>
#include <QApplication>
Widget::Widget(QWidget *parent)
	: QWidget(parent)
{
	list << tr("最爱你的人是我") << tr("他说你说我也说") << tr("回头再说爱");
	index = 2;
	m = 1;
	font = new QFont(tr("黑体"), 20);
	QFontMetrics metrics(*font);
	r = metrics.boundingRect(list[index]);
	sp = r.width() / 30;
	this->resize(r.size());

	bmp = QBitmap(this->size());
	bmp.fill();
	QPainter p(&bmp);
	path = new QPainterPath;
	path->addText(0.0, metrics.ascent(), *font, list[index]);
	path->closeSubpath();
	p.setPen(Qt::black);
	p.setBrush(Qt::black);
	p.setFont(*font);
	p.drawPath(*path);
	setMask(bmp);
	//drawMusicWord();
	timer = new QTimer(this);
	timer->setInterval(30);
	connect(timer, SIGNAL(timeout()), this, SLOT(drawMusicWord()));
	timer->start();
}

Widget::~Widget()
{

}

void Widget::paintEvent(QPaintEvent *e)
{
	QPixmap pmp(r.size());
	QPainter p(&pmp);



	p.setFont(*font);
	p.setPen(Qt::red);
	p.setBrush(Qt::red);
	p.setClipRect(left);
	p.drawPath(*path);


	p.setPen(Qt::green);
	p.setBrush(Qt::green);
	p.setClipRect(right);
	p.drawPath(*path);

	QPainter pWidget(this);
	pWidget.drawPixmap(0, 0, pmp);
}

void Widget::drawMusicWord()
{
	m++;
	qDebug() << m << endl;

	left = QRect(0, 0, (int)(m*sp), r.height());
	right = QRect(left.width(), 0, r.width() - left.width(), r.height());
	if (r.width() - left.width() < 0)
	{
		m = 0;
		index++;
		if (index >= list.length())
			index = 0;
		QFontMetrics metrics(*font);
		r = metrics.boundingRect(list[index]);
		sp = r.width() / 50;
		this->resize(r.size());

		bmp = QBitmap(this->size());
		bmp.fill();
		QPainter p(&bmp);
		if (path)
			delete path;
		path = new QPainterPath;
		path->addText(0.0, metrics.ascent(), *font, list[index]);
		path->closeSubpath();
		p.setPen(Qt::black);
		p.setBrush(Qt::black);
		p.setFont(*font);
		p.drawPath(*path);
		setMask(bmp);
	}
	update();
	QApplication::processEvents();
}
