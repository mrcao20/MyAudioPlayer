#pragma once

#include <qlabel.h>
#include <qtimer.h>

class LyricLabel : public QLabel{
	Q_OBJECT

public:
	LyricLabel(QWidget *parent = 0);

	const QFont &font() const { return m_font; }
	bool isPlayed() { return m_isPlayed; }
	void setText(const QString &text);
	void start_lrc_mask(qint64 intervaltime);
	void set_lrc_mask(qint64 intervaltime);
	void stop_lrc_mask();
	void pause_lrc_mask() { m_timer->stop(); }
	void resume_lrc_mask() { m_timer->start(); }
	int showProgress() { return m_showProgress; }
	void reset();
protected:
	void paintEvent(QPaintEvent *);

signals:
	void lyricPlayedOver(LyricLabel *);

private slots :
	void timeout();

private:
	QLinearGradient m_linear_gradient;
	QLinearGradient m_mask_linear_gradient;
	QFont m_font;
	QTimer *m_timer;
	qreal m_lrc_mask_width;
	qreal m_lrc_mask_width_interval;
	qint64 m_intervaltime;
	bool m_isPlayed;
	int m_showProgress;			// 显示进度，当前宽度除以每一次增加的宽度乘间隔时间30ms
};
