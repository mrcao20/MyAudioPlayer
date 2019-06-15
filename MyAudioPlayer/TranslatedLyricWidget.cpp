#include "TranslatedLyricWidget.h"
#include "LyricLabel.h"
#include <qtextstream.h>
#include <QTime>
#include <QRegularExpression>

QTextStream cout(stdout);

struct TranslatedLyricWidgetDataPrivate {
	LyricLabel *m_topLyricLabel;
	LyricLabel *m_bottomLyricLabel;

	qint64 m_passTime;				// 如果未暂停过，则为零，否则暂停之前所经过的时长
	qint64 m_playTime;				// 歌曲已经播放的总时长
	qint64 m_currentTime;			// 当前歌词播放所经历的时间
	bool m_isPlay;
	bool m_isRunning;
	QTime m_time;
	QTimer *m_timeTimer;
	LyricInfo m_currentLyricInfo;
	QMap<qint64, LyricInfo> m_histroyUntranslateMap;
	QMap<qint64, LyricInfo> m_histroyTranslateMap;
	QMap<qint64, LyricInfo> m_lyricUntranslateMap;
	QMap<qint64, LyricInfo> m_lyricTranslateMap;
};

TranslatedLyricWidget::TranslatedLyricWidget(QWidget *parent)
	: LyricWidget(parent)
	,d (new TranslatedLyricWidgetDataPrivate){

	d->m_isPlay = false;
	d->m_isRunning = false;
	d->m_passTime = 0;
	d->m_currentTime = 0;

	d->m_topLyricLabel = new LyricLabel(this);
	d->m_bottomLyricLabel = new LyricLabel(this);

	d->m_topLyricLabel->installEventFilter(this);
	d->m_bottomLyricLabel->installEventFilter(this);

	setText(d->m_topLyricLabel, "my audio player", 0);
	d->m_topLyricLabel->stop_lrc_mask();

	connect(d->m_topLyricLabel, &LyricLabel::lyricPlayedOver, this, &TranslatedLyricWidget::stopPlayLyric);

	d->m_timeTimer = new QTimer(this);
	connect(d->m_timeTimer, &QTimer::timeout, this, &TranslatedLyricWidget::playLyric);
	d->m_timeTimer->setInterval(30);
}

void TranslatedLyricWidget::reset() {
	d->m_topLyricLabel->reset();
	d->m_bottomLyricLabel->reset();
	d->m_lyricUntranslateMap.clear();
	d->m_lyricTranslateMap.clear();
	d->m_histroyUntranslateMap.clear();
	d->m_histroyTranslateMap.clear();
	d->m_passTime = 0;
	d->m_currentTime = 0;
}

void TranslatedLyricWidget::stop() {
	d->m_isPlay = false;
	d->m_passTime = ((d->m_time.elapsed() + d->m_passTime) + d->m_playTime) / 2;
	d->m_timeTimer->stop();
	d->m_topLyricLabel->pause_lrc_mask();
	d->m_bottomLyricLabel->pause_lrc_mask();
}

void TranslatedLyricWidget::stopPlayLyric() {
	d->m_isRunning = false;
	if (d->m_lyricUntranslateMap.isEmpty()) {
		d->m_topLyricLabel->stop_lrc_mask();
		d->m_bottomLyricLabel->stop_lrc_mask();
	}
	else {
		d->m_topLyricLabel->reset();
		d->m_bottomLyricLabel->reset();
	}
	if (d->m_lyricUntranslateMap.isEmpty() || d->m_lyricTranslateMap.isEmpty()) {
		d->m_timeTimer->stop();
	}
	setLyric();
}

void TranslatedLyricWidget::restart() {
	d->m_isPlay = true;
	d->m_time.restart();
	d->m_timeTimer->start();
	if (d->m_isRunning) {
		d->m_topLyricLabel->resume_lrc_mask();
		d->m_bottomLyricLabel->resume_lrc_mask();
	}
}

void TranslatedLyricWidget::init() {
	d->m_topLyricLabel->reset();
	d->m_bottomLyricLabel->reset();
	setLyric();
}

void TranslatedLyricWidget::setLyric(const QString &lyric, const qint64 &currentTime) {
	reset();
	QStringList lyricList = lyric.split(QRegularExpression("--"), QString::SkipEmptyParts);
	if (lyricList.size() != 2) {
		setText(d->m_topLyricLabel, "not found translate", 0);
		d->m_topLyricLabel->stop_lrc_mask();
		return;
	}
	QRegularExpression re(R"(\[offset:(\d+)\])");
	QRegularExpressionMatch match = re.match(lyricList.at(0));
	QStringList tempList = match.capturedTexts();
	if (tempList.size() >= 2) {
		d->m_passTime = tempList.at(1).toInt();
	}
	getLyricMap(lyricList.at(0), d->m_lyricUntranslateMap, currentTime);
	getLyricMap(lyricList.at(1), d->m_lyricTranslateMap, currentTime);
	init();
	restart();
}

void TranslatedLyricWidget::setLyric() {
	if (d->m_lyricUntranslateMap.isEmpty() || d->m_lyricTranslateMap.isEmpty())
		return;
	if (!d->m_topLyricLabel->isPlayed() || !d->m_bottomLyricLabel->isPlayed())
		return;
	const qint64 startTime = d->m_lyricUntranslateMap.firstKey();
	LyricInfo lyricUntranslateInfo = d->m_lyricUntranslateMap.take(startTime);
	d->m_currentLyricInfo = lyricUntranslateInfo;
	setText(d->m_topLyricLabel, lyricUntranslateInfo.m_lyric, 0);
	LyricInfo lyricTranslateInfo = d->m_lyricTranslateMap.take(startTime);
	QString translateLyric = lyricTranslateInfo.m_lyric;
	if (translateLyric.isEmpty()) {
		lyricTranslateInfo = d->m_lyricTranslateMap.take(startTime + 10);
		translateLyric = lyricTranslateInfo.m_lyric;
	}
	if (translateLyric.isEmpty()) {
		lyricTranslateInfo = d->m_lyricTranslateMap.take(startTime - 10);
		translateLyric = lyricTranslateInfo.m_lyric;
	}
	if (translateLyric == "//" || translateLyric.isEmpty())
		translateLyric = lyricUntranslateInfo.m_lyric;
	setText(d->m_bottomLyricLabel, translateLyric, 1);
	d->m_histroyUntranslateMap.insert(lyricUntranslateInfo.m_startTime, lyricUntranslateInfo);
	d->m_histroyTranslateMap.insert(lyricTranslateInfo.m_startTime, lyricTranslateInfo);
}

void TranslatedLyricWidget::playLyric() {
	d->m_currentTime = d->m_time.elapsed() + d->m_passTime;
	if (d->m_isPlay && d->m_currentTime >= d->m_currentLyricInfo.m_startTime) {
		if (d->m_lyricUntranslateMap.contains(d->m_currentLyricInfo.m_startTime)) {
			stopPlayLyric();
		}
		d->m_topLyricLabel->set_lrc_mask(d->m_currentLyricInfo.m_endTime - d->m_currentLyricInfo.m_startTime);
		d->m_topLyricLabel->start_lrc_mask(d->m_currentLyricInfo.m_endTime - d->m_currentTime);
		d->m_bottomLyricLabel->set_lrc_mask(d->m_currentLyricInfo.m_endTime - d->m_currentLyricInfo.m_startTime);
		d->m_bottomLyricLabel->start_lrc_mask(d->m_currentLyricInfo.m_endTime - d->m_currentTime);
		if (d->m_lyricUntranslateMap.isEmpty())
			d->m_isPlay = false;
		else
			d->m_currentLyricInfo = d->m_lyricUntranslateMap.first();
		d->m_isRunning = true;
	}
	if (d->m_isRunning) {
		int progress_label = d->m_topLyricLabel->showProgress();
		LyricInfo lyricInfo = d->m_histroyUntranslateMap.last();
		int progress_widget = lyricInfo.m_endTime - lyricInfo.m_startTime - (lyricInfo.m_endTime - d->m_currentTime);
		if (abs(progress_label - progress_widget) > 50) {
			d->m_topLyricLabel->start_lrc_mask(lyricInfo.m_endTime - d->m_currentTime);
			d->m_bottomLyricLabel->start_lrc_mask(lyricInfo.m_endTime - d->m_currentTime);
		}
	}
}

void TranslatedLyricWidget::setTime(const qint64 &playTime) {
	d->m_playTime = playTime;
	if (abs(d->m_playTime - d->m_currentTime) < 1000)
		return;
	if (d->m_playTime < d->m_currentTime) {
		bool isAdd = true;
		for (auto itr = d->m_histroyUntranslateMap.begin(); itr != d->m_histroyUntranslateMap.end();) {
			if (itr.key() < d->m_playTime) {
				itr++;
				continue;
			}
			else {
				if (isAdd) {
					isAdd = false;
					if (itr != d->m_histroyUntranslateMap.begin()) {
						auto tempItr = itr - 1;
						d->m_lyricUntranslateMap.insert(tempItr.key(), tempItr.value());
						d->m_lyricTranslateMap.insert(tempItr.key(), d->m_histroyTranslateMap.value(tempItr.key()));
					}
				}
				d->m_lyricUntranslateMap.insert(itr.key(), itr.value());
				d->m_lyricTranslateMap.insert(itr.key(), d->m_histroyTranslateMap.value(itr.key()));
				d->m_histroyTranslateMap.remove(itr.key());
				itr = d->m_histroyUntranslateMap.erase(itr);
			}
		}
		init();
	}
	d->m_passTime = d->m_playTime;
	d->m_time.restart();
	playLyric();
	if (d->m_isRunning && !d->m_isPlay) {
		d->m_topLyricLabel->pause_lrc_mask();
		d->m_bottomLyricLabel->pause_lrc_mask();
	}
}

void TranslatedLyricWidget::setText(LyricLabel *lyricLabel, const QString &text, int flag) {
	QFontMetrics fontMetrics(lyricLabel->font());
	QRect rect = fontMetrics.boundingRect(text);
	lyricLabel->setText(text);
	lyricLabel->resize(rect.width(), height() / 2);
	lyricLabel->move(width() / 2 - d->m_topLyricLabel->width() / 2, height() * flag / 2);
}

TranslatedLyricWidget::~TranslatedLyricWidget(){

}
