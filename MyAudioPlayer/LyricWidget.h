#pragma once
#include <qwidget.h>
#include "ui_Lyric.h"

struct LyricInfo {
	qint64 m_startTime;
	QString m_lyric;
	qint64 m_endTime;
};

class LyricWidget : public QWidget, public Ui::LyricWidget{
	Q_OBJECT

public:
	explicit LyricWidget(QWidget *parent = 0);
	
	virtual void setLyric(const QString &lyric, const qint64 &currentTime) = 0;
	virtual void stop() = 0;
	virtual void restart() = 0;
	virtual void setTime(const qint64 &playTime) = 0;

protected:
	bool eventFilter(QObject *obj, QEvent *event);
	void getLyricMap(const QString &lyric, QMap<qint64, LyricInfo> &lyricMap, const qint64 &currentTime);

private:
	bool m_pressFlag;
	QPoint m_position;
	QPoint m_beginMousePos;

};