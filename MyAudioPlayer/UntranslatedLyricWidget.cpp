#include "UntranslatedLyricWidget.h"
#include "LyricLabel.h"
#include <qmap.h>
#include <QTime>
#include <qtimer.h>

struct UntranslatedLyricDataPrivate {
	qint64 m_passTime;				// 如果未暂停过，则为零，否则暂停之前所经过的时长
	qint64 m_playTime;				// 歌曲已经播放的总时长
	qint64 m_currentTime;			// 当前歌词播放所经历的时间
	bool m_isPlay;
	QTime m_time;
	QTimer *m_timeTimer;
	LyricLabel *m_currentLabel;
	LyricInfo m_currentLyricInfo;
	UntranslatedLyricWidget::LyricPosition m_lyricPosition;
	QMap<qint64, LyricInfo> m_playMap;
	QMap<qint64, LyricInfo> m_histroyMap;
	QMap<qint64, LyricInfo> m_lyricMap;

	LyricLabel *m_topLyricLabel;
	LyricLabel *m_bottomLyricLabel;
};

UntranslatedLyricWidget::UntranslatedLyricWidget(QWidget *parent)
	: LyricWidget(parent)
	, d(new UntranslatedLyricDataPrivate){

	d->m_passTime = 0;
	d->m_currentTime = 0;
	d->m_isPlay = false;
	d->m_lyricPosition = UntranslatedLyricWidget::Top;
	d->m_currentLabel = nullptr;
	d->m_topLyricLabel = new LyricLabel(this);
	d->m_bottomLyricLabel = new LyricLabel(this);

	d->m_topLyricLabel->installEventFilter(this);
	d->m_bottomLyricLabel->installEventFilter(this);

	d->m_topLyricLabel->move(0, 0);
	setText(d->m_topLyricLabel, "my audio player");
	d->m_topLyricLabel->stop_lrc_mask();

	connect(d->m_topLyricLabel, &LyricLabel::lyricPlayedOver, this, &UntranslatedLyricWidget::stopPlayLyric);
	connect(d->m_bottomLyricLabel, &LyricLabel::lyricPlayedOver, this, &UntranslatedLyricWidget::stopPlayLyric);

	d->m_timeTimer = new QTimer(this);
	connect(d->m_timeTimer, &QTimer::timeout, this, &UntranslatedLyricWidget::playLyric);
	d->m_timeTimer->setInterval(30);
}

void UntranslatedLyricWidget::stop() {
	d->m_isPlay = false;
	d->m_passTime = ((d->m_time.elapsed() + d->m_passTime) + d->m_playTime) / 2;
	d->m_timeTimer->stop();
	if (d->m_currentLabel)
		d->m_currentLabel->pause_lrc_mask();
}

void UntranslatedLyricWidget::restart() {
	d->m_isPlay = true;
	d->m_time.restart();
	d->m_timeTimer->start();
	if (d->m_currentLabel)
		d->m_currentLabel->resume_lrc_mask();
}

void UntranslatedLyricWidget::setTime(const qint64 &playTime) {
	d->m_playTime = playTime;
	if (abs(d->m_playTime - d->m_currentTime) > 1000) {
		if (d->m_playTime < d->m_currentTime) {
			bool isAdd = true;
			for (auto itr = d->m_histroyMap.begin(); itr != d->m_histroyMap.end(); itr++) {
				if (d->m_playTime < itr.key()) {
					if (isAdd) {
						isAdd = false;
						if (itr != d->m_histroyMap.begin()) {
							auto tempItr = itr - 1;
							d->m_lyricMap.insert(tempItr.key(), tempItr.value());
						}
					}
					d->m_lyricMap.insert(itr.key(), itr.value());
				}
			}
			initLabel();
		}
		d->m_passTime = d->m_playTime;
		d->m_time.restart();
		playLyric();
		if (d->m_currentLabel && !d->m_isPlay)
			d->m_currentLabel->pause_lrc_mask();
	}
}

void UntranslatedLyricWidget::setLyric(const QString &lyric, const qint64 &currentTime) {
	reset();
	if (lyric.isEmpty()) {
		setText(d->m_topLyricLabel, "not found lyric");
		d->m_topLyricLabel->stop_lrc_mask();
		return;
	}
	QRegularExpression re("\\[offset:(\\d+)\\]");
	QRegularExpressionMatch match = re.match(lyric);
	QStringList tempList = match.capturedTexts();
	if (tempList.size() >= 2) {
		d->m_passTime = tempList.at(1).toInt();
	}
	getLyricMap(lyric, d->m_lyricMap, currentTime);
	initLabel();
	restart();
}

void UntranslatedLyricWidget::initLabel() {
	d->m_lyricPosition = UntranslatedLyricWidget::Top;
	d->m_currentLabel = nullptr;
	d->m_playMap.clear();
	d->m_topLyricLabel->reset();
	d->m_bottomLyricLabel->reset();
	setLyric(UntranslatedLyricWidget::Top);
	setLyric(UntranslatedLyricWidget::Bottom);
}

void UntranslatedLyricWidget::reset() {
	d->m_topLyricLabel->reset();
	d->m_bottomLyricLabel->reset();
	d->m_lyricMap.clear();
	d->m_playMap.clear();
	d->m_histroyMap.clear();
	d->m_passTime = 0;
	d->m_currentTime = 0;
	d->m_lyricPosition = UntranslatedLyricWidget::Top;
}

void UntranslatedLyricWidget::playLyric() {
	if (d->m_playMap.isEmpty())
		return;
	d->m_currentTime = d->m_time.elapsed() + d->m_passTime;
	if (d->m_currentTime >= d->m_playMap.firstKey()) {
		if (d->m_currentLabel)
			stopPlayLyric(d->m_currentLabel);
		LyricInfo lyricInfo = d->m_playMap.take(d->m_playMap.firstKey());
		d->m_currentLyricInfo = lyricInfo;
		LyricLabel *lyricLabel = getLyricLabel(d->m_lyricPosition);
		d->m_currentLabel = lyricLabel;
		if (lyricLabel) {
			lyricLabel->set_lrc_mask(lyricInfo.m_endTime - lyricInfo.m_startTime);
			lyricLabel->start_lrc_mask(lyricInfo.m_endTime - d->m_currentTime);
		}
		d->m_lyricPosition == UntranslatedLyricWidget::Top ? setLyric(UntranslatedLyricWidget::Bottom) : setLyric(UntranslatedLyricWidget::Top);
	}
	if (d->m_currentLabel) {
		int progress_label = d->m_currentLabel->showProgress();
		int progress_widget = d->m_currentLyricInfo.m_endTime - d->m_currentLyricInfo.m_startTime - (d->m_currentLyricInfo.m_endTime - d->m_currentTime);
		if (abs(progress_label - progress_widget) > 50) {
			d->m_currentLabel->start_lrc_mask(d->m_currentLyricInfo.m_endTime - d->m_currentTime);
		}
	}
}

inline void UntranslatedLyricWidget::stopPlayLyric(LyricLabel *lyricLabel) {
	d->m_currentLabel = nullptr;
	d->m_lyricPosition = (d->m_lyricPosition == UntranslatedLyricWidget::Top) ? UntranslatedLyricWidget::Bottom : UntranslatedLyricWidget::Top;
	lyricLabel->stop_lrc_mask();
	if (d->m_lyricMap.isEmpty()) {
		d->m_timeTimer->stop();
	}
}

void UntranslatedLyricWidget::setLyric(UntranslatedLyricWidget::LyricPosition lyricPosition) {
	if (d->m_lyricMap.isEmpty())
		return;
	LyricLabel *lyricLabel = getLyricLabel(lyricPosition);
	if (!lyricLabel->isPlayed())
		return;
	LyricInfo lyricInfo = d->m_lyricMap.take(d->m_lyricMap.firstKey());
	setText(lyricLabel, lyricInfo.m_lyric);
	d->m_histroyMap.insert(lyricInfo.m_startTime, lyricInfo);
	d->m_playMap.insert(lyricInfo.m_startTime, lyricInfo);
	if (lyricPosition == UntranslatedLyricWidget::Bottom)
		lyricLabel->move(width() - lyricLabel->width(), height() / 2);
}

LyricLabel *UntranslatedLyricWidget::getLyricLabel(UntranslatedLyricWidget::LyricPosition lyricPosition) {
	switch (lyricPosition)
	{
	case UntranslatedLyricWidget::Top:
		return d->m_topLyricLabel;
	case UntranslatedLyricWidget::Bottom:
		return d->m_bottomLyricLabel;
	}
	return nullptr;
}

void UntranslatedLyricWidget::setText(LyricLabel *lyricLabel, const QString &text) {
	QFontMetrics fontMetrics(lyricLabel->font());
	QRect rect = fontMetrics.boundingRect(text);
	lyricLabel->setText(text);
	lyricLabel->resize(rect.width(), height() / 2);
}

UntranslatedLyricWidget::~UntranslatedLyricWidget(){

}
