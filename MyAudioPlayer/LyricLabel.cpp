#include "LyricLabel.h"
#include <qtimer.h>
#include <QPainter>

LyricLabel::LyricLabel(QWidget *parent)
	: QLabel(parent) 
	, m_isPlayed(true){

	//歌词的线性渐变填充
	m_linear_gradient.setStart(0, 10);//填充的起点坐标
	m_linear_gradient.setFinalStop(0, 40);//填充的终点坐标
										//第一个参数终点坐标，相对于我们上面的区域而言，按照比例进行计算
	m_linear_gradient.setColorAt(0.1, QColor(14, 179, 255));
	m_linear_gradient.setColorAt(0.5, QColor(114, 232, 255));
	m_linear_gradient.setColorAt(0.9, QColor(14, 179, 255));

	// 遮罩的线性渐变填充
	m_mask_linear_gradient.setStart(0, 10);
	m_mask_linear_gradient.setFinalStop(0, 40);
	m_mask_linear_gradient.setColorAt(0.1, QColor(222, 54, 4));
	m_mask_linear_gradient.setColorAt(0.5, QColor(255, 72, 16));
	m_mask_linear_gradient.setColorAt(0.9, QColor(222, 54, 4));

	// 设置字体
	m_font.setFamily("Times New Roman");
	m_font.setBold(true);
	m_font.setPointSize(24);
	
	// 设置定时器
	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
	m_timer->setInterval(30);
	m_lrc_mask_width = 0;
	m_lrc_mask_width_interval = 0;
	show();
}

void LyricLabel::setText(const QString &text) {
	m_isPlayed = false;
	m_lrc_mask_width = 0; 
	QLabel::setText(text);
	update();
}

void LyricLabel::reset() {
	clear();
	m_isPlayed = true;
	m_timer->stop();
	m_lrc_mask_width = 0;
	m_lrc_mask_width_interval = 0;
	update();
}

// 传入当前歌词剩余的播放时间
void LyricLabel::start_lrc_mask(qint64 intervaltime)
{
	qreal count = (m_intervaltime - intervaltime) / 30;
	m_lrc_mask_width = m_lrc_mask_width_interval * count;
	m_timer->start();
	update();
}

// 传入整段歌词播放所需的时间
void LyricLabel::set_lrc_mask(qint64 intervaltime) {
	m_intervaltime = intervaltime;
	qreal count = intervaltime / 30;
	m_lrc_mask_width_interval = width() / count;
	m_lrc_mask_width = 0;
}

void LyricLabel::stop_lrc_mask()
{
	m_isPlayed = true;
	m_timer->stop();
	m_lrc_mask_width = width();
	update();
}

void LyricLabel::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setFont(m_font);

	// 先绘制底层文字，作为阴影，这样会使显示效果更加清晰，且更有质感
	painter.setPen(QColor(0, 0, 0, 200));
	painter.drawText(1, 1, width(), height(), Qt::AlignLeft | Qt::AlignVCenter, text());//左对齐

														   // 再在上面绘制渐变文字
	painter.setPen(QPen(m_linear_gradient, 0));
	painter.drawText(0, 0, width(), height(), Qt::AlignLeft | Qt::AlignVCenter, text());

	// 设置歌词遮罩
	painter.setPen(QPen(m_mask_linear_gradient, 0));
	painter.drawText(0, 0, m_lrc_mask_width, height(), Qt::AlignLeft | Qt::AlignVCenter, text());

	m_showProgress = m_lrc_mask_width / m_lrc_mask_width_interval * 30;
}

void LyricLabel::timeout()
{
	if (m_lrc_mask_width > width()) {
		emit lyricPlayedOver(this);
	}
	//每隔一段固定的时间笼罩的长度就增加一点
	m_lrc_mask_width += m_lrc_mask_width_interval;
	update();//更新widget，但是并不立即重绘，而是安排一个Paint事件，当返回主循环时由系统来重绘
}