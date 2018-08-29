#include "LyricWidget.h"
#include <qevent.h>
#include <qfontmetrics.h>
#include <QBitmap>
#include <QTime>
#include <qregularexpression.h>
#include <qdebug.h>

LyricWidget::LyricWidget(QWidget *parent)
	: QWidget(parent){

	setupUi(this);

	setAttribute(Qt::WA_TranslucentBackground, true); //±³¾°Í¸Ã÷
	setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool); //È¥³ý±ß¿ò //Tool

	move(400, 720);
	show();
}

void LyricWidget::getLyricMap(const QString &lyric, QMap<qint64, LyricInfo> &lyricMap, const qint64 &currentTime) {
	QString tempLyric = lyric;
	tempLyric.replace(QRegularExpression("&apos;"), "'");
	QStringList lyricList = tempLyric.split(QRegularExpression("\\n"));
	QRegularExpression re(R"(\[(\d.*\d)\])");
	qint64 lastTime;
	for (QString str : lyricList) {
		QRegularExpressionMatch match = re.match(str);
		if (match.capturedTexts().size() < 2)
			continue;
		QTime time = QTime::fromString(match.capturedTexts().at(1), "mm:ss.z");
		qint64 startTime = time.minute() * 60 * 1000 + time.second() * 1000 + time.msec();
		if (lyricMap.size() > 0) {
			auto itr = lyricMap.find(lastTime);
			itr != lyricMap.end() ? itr->m_endTime = startTime : false;
		}
		lastTime = startTime;
		QString timeStr = match.capturedTexts().at(0);
		QString currentLyric = str.remove(timeStr);
		if (currentLyric.isEmpty())
			continue;
		LyricInfo lyricInfo;
		lyricInfo.m_startTime = startTime;
		lyricInfo.m_lyric = currentLyric;
		lyricMap.insert(startTime, lyricInfo);
	}
	if (!lyricMap.isEmpty()) {
		auto itr = --lyricMap.end();
		itr->m_endTime = itr->m_startTime + 2 * 1000;
	}
	for (auto itr = lyricMap.begin(); itr != lyricMap.end(); ) {
		if (lyricMap.size() <= 2 || (itr + 1).key() > currentTime)
			break;
		itr = lyricMap.erase(itr);
	}
}

bool LyricWidget::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::MouseButtonPress) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->button() == Qt::LeftButton) {
			setCursor(Qt::ClosedHandCursor);
			m_pressFlag = true;
			m_position = mouseEvent->globalPos() - pos();
			m_beginMousePos = mouseEvent->globalPos();
		}
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->button() == Qt::LeftButton) {
			setCursor(Qt::OpenHandCursor);
			QPoint pos = mouseEvent->globalPos() - m_beginMousePos;
			if (abs(pos.x()) <= 5 && abs(pos.y()) <= 5) {
			}
			m_pressFlag = false;
		}
	}
	else if (event->type() == QEvent::MouseMove) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		if (m_pressFlag && (mouseEvent->buttons() && Qt::LeftButton)) {
			QPoint point = mouseEvent->globalPos() - m_position;
			move(point);
		}
	}
	else if (event->type() == QEvent::Enter) {
		setCursor(Qt::OpenHandCursor);
		setStyleSheet("background-color:rgba(0, 0, 0, 20)");
	}
	else if (event->type() == QEvent::Leave) {
		setStyleSheet("background-color:rgba(0, 0, 0, 0)");
	}
	return QWidget::eventFilter(obj, event);
}
