#pragma once
#pragma execution_character_set("UTF-8")
#include "LyricWidget.h"
#include <qscopedpointer.h>
#include <qmap.h>

struct TranslatedLyricWidgetDataPrivate;
class LyricLabel;

class TranslatedLyricWidget : public LyricWidget {
	Q_OBJECT

public:
	explicit TranslatedLyricWidget(QWidget *parent = 0);
	virtual ~TranslatedLyricWidget();

	void setLyric(const QString &lyric, const qint64 &currentTime);
	void stop();
	void restart();
	void setTime(const qint64 &playTime);

private slots:
	void stopPlayLyric();
	void playLyric();

private:
	void reset();
	void init();
	void setText(LyricLabel *lyricLabel, const QString &text, int flag);
	void setLyric();

private:
	QScopedPointer<TranslatedLyricWidgetDataPrivate> d;

};

