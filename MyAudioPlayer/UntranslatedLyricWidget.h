#pragma once
#include "LyricWidget.h"
#include <qscopedpointer.h>

struct UntranslatedLyricDataPrivate;
class LyricLabel;

class UntranslatedLyricWidget : public LyricWidget{
	Q_OBJECT

public:

	enum LyricPosition {
		Top,
		Bottom
	};

	explicit UntranslatedLyricWidget(QWidget *parent = 0);
	virtual ~UntranslatedLyricWidget();

	void setLyric(const QString &lyric, const qint64 &currentTime);
	void stop();
	void restart();
	void setTime(const qint64 &playTime);

private slots:
	void stopPlayLyric(LyricLabel *lyricLabel);
	void playLyric();

private:
	void reset();
	void initLabel();
	void setLyric(UntranslatedLyricWidget::LyricPosition);
	void setText(LyricLabel *lyricLabel, const QString &text);
	LyricLabel *getLyricLabel(UntranslatedLyricWidget::LyricPosition);

private:
	QScopedPointer<UntranslatedLyricDataPrivate> d;

};