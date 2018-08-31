#include "NetworkSearchFrame.h"
#include <qtablewidget.h>
#include <qdebug.h>
#include <qscrollbar.h>
#include <qmenu.h>
#include <qtablewidget.h>
#include <qheaderview.h>
#include "YApi.h"
#include "QQApi.h"
#include <qregularexpression.h>

struct NetworkSearchDataPrivate
{
	QTableWidget * m_tableWidget;
	NetworkApiBase *m_networkApi;
	int m_offset;
	int m_searchSongNum;
	QString m_songName;
	QList<SongSearchDetailedInfo> m_songSearchDetailedInfos;
};

NetworkSearchFrame::NetworkSearchFrame(QSize size, QStringList playlistList, QWidget *parent)
	: QFrame(parent)
	, d(new NetworkSearchDataPrivate) {
	d->m_offset = 0;
	d->m_searchSongNum = 20;
	d->m_songName = "";
	d->m_networkApi = nullptr;
	d->m_tableWidget = new QTableWidget(this);

	d->m_tableWidget->setColumnCount(4);
	d->m_tableWidget->setRowCount(d->m_searchSongNum);
	d->m_tableWidget->setColumnWidth(0, 250);
	d->m_tableWidget->setColumnWidth(1, 120);
	d->m_tableWidget->setColumnWidth(2, 120);
	d->m_tableWidget->setColumnWidth(3, 70);

	d->m_tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	d->m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	d->m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	d->m_tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
	d->m_tableWidget->setShowGrid(false);

	QHeaderView *horizontalHeaderView = d->m_tableWidget->horizontalHeader();
	horizontalHeaderView->setStretchLastSection(true);
	horizontalHeaderView->setSectionResizeMode(QHeaderView::Custom);		 // 设置表头不能使用鼠标拖动
	horizontalHeaderView->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	horizontalHeaderView->setHighlightSections(false);

	QHeaderView *verticalHeaderView = d->m_tableWidget->verticalHeader();
	verticalHeaderView->setVisible(false);
	verticalHeaderView->setDefaultAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	verticalHeaderView->setDefaultSectionSize(30);

	QStringList headerList;
	headerList << "歌曲" << "歌手" << "专辑" << "时长";
	d->m_tableWidget->setHorizontalHeaderLabels(headerList);

	d->m_tableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{background-color:rgba(107, 21, 0, 255);color:rgb(200, 200, 200)}");

	d->m_tableWidget->setStyleSheet("QTableWidget{background-color:rgb(107, 21, 0);}");

	d->m_tableWidget->resize(size);

	d->m_tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

	QMenu *songMenu = new QMenu(this);
	QAction *playAction = songMenu->addAction("play");
	QMenu *addMenu = new QMenu(songMenu);
	addMenu->setTitle("add to");
	songMenu->addMenu(addMenu);
	QAction *downloadAction = songMenu->addAction("download");
	for (QString playlist : playlistList) {
		QAction *playlistAction = addMenu->addAction(playlist);
		connect(playlistAction, &QAction::triggered, [this, playlistAction]() {
			QList<QTableWidgetSelectionRange>itemRanges = d->m_tableWidget->selectedRanges();
			QString table = "";
			QString className = d->m_networkApi->metaObject()->className();
			if (className == "YApi")
				table = "t_ymusic";
			else if (className == "QQApi")
				table = "t_qqmusic";
			for (int i = 0; i < itemRanges.size(); i++) {
				QTableWidgetSelectionRange itemRange = itemRanges.at(i);
				for (int j = itemRange.topRow(); j <= itemRange.bottomRow(); j++) {
					if (!d->m_tableWidget->item(j, 0)) {
						continue;
					}
					SongSearchDetailedInfo songSearchDetailedInfo = getSongInfo(j);
					emit addSong(songSearchDetailedInfo.song_id, songSearchDetailedInfo.song_name, table, playlistAction->text());
				}
			}
		});
	}

	connect(playAction, &QAction::triggered, [this]() {
		int index = d->m_tableWidget->currentRow();
		d->m_tableWidget->setCurrentCell(index, 0);
		playSong(index);
	});
	connect(downloadAction, &QAction::triggered, [this]() {
		int index = d->m_tableWidget->currentRow();
		d->m_tableWidget->setCurrentCell(index, 0);
		SongSearchDetailedInfo songSearchDetailedInfo = getSongInfo(index);
		QString songName = songSearchDetailedInfo.song_name;
		QString songId = songSearchDetailedInfo.song_id;
		QString songLink = d->m_networkApi->getDownloadLink(songId);
		d->m_networkApi->downloadSong(songName, getSongType(songLink), songLink);
	});
	
	connect(d->m_tableWidget, &QTableWidget::itemDoubleClicked, this, &NetworkSearchFrame::itemDoubleClicked);
	connect(d->m_tableWidget, &QTableWidget::customContextMenuRequested, [songMenu]() {
		songMenu->exec(QCursor::pos());
	});

	show();
}

QString NetworkSearchFrame::getSongType(const QString &songLink) {
	QString str = songLink + "-";
	int len = str.indexOf(QRegularExpression("//")) + 2;
	QString str2 = str.right(str.length() - len);
	int len2 = str2.indexOf(QRegularExpression("/")) + 1;
	QString str3 = str2.right(str2.length() - len2);
	QRegularExpression re(R"(.+?\.(.+?)[\?&/-].*)");
	QRegularExpressionMatch match = re.match(str3);
	QString type;
	if (re.captureCount() >= 1) {
		type = match.captured(1);
	}
	else {
		type = "m4a";
	}
	return type;
}

void NetworkSearchFrame::searchSong(const QString &songName) {
	d->m_offset = 0;
	d->m_songName = songName;
	d->m_songSearchDetailedInfos.swap(QList<SongSearchDetailedInfo>());
	d->m_tableWidget->clearContents();
	d->m_songSearchDetailedInfos = search();
	addItem();
}

void NetworkSearchFrame::previousSearch() {
	d->m_offset -= d->m_searchSongNum;
	if (d->m_offset < 0) {
		d->m_offset = 0;
		return;
	}
	addItem();
}

void NetworkSearchFrame::nextSearch() {
	d->m_offset += d->m_searchSongNum;
	if(d->m_offset >= d->m_songSearchDetailedInfos.size())
		d->m_songSearchDetailedInfos += search();
	addItem();
}

void NetworkSearchFrame::setNetworkApi(NetworkApiBase *networkApi) {
	d->m_networkApi = networkApi;
}

void NetworkSearchFrame::addItem() {
	if (d->m_offset >= d->m_songSearchDetailedInfos.size()) {
		d->m_offset -= d->m_searchSongNum;
		return;
	}
	d->m_tableWidget->clearContents();
	if (d->m_tableWidget->rowCount() < d->m_searchSongNum)
		d->m_tableWidget->setRowCount(d->m_searchSongNum);
	for (int i = 0; i < d->m_searchSongNum; i++) {
		int index = i + d->m_offset;
		if (index >= d->m_songSearchDetailedInfos.size()) {
			d->m_tableWidget->setRowCount(i);
			break;
		}
		SongSearchDetailedInfo songSearchDetailedInfo = d->m_songSearchDetailedInfos.at(index);
		QTableWidgetItem *item1 = new QTableWidgetItem(songSearchDetailedInfo.song_name);
		QTableWidgetItem *item2 = new QTableWidgetItem(songSearchDetailedInfo.song_artistName);
		QTableWidgetItem *item3 = new QTableWidgetItem(songSearchDetailedInfo.song_albumName);
		item1->setTextColor(QColor(200, 200, 200));
		item2->setTextColor(QColor(200, 200, 200));
		item3->setTextColor(QColor(200, 200, 200));
		d->m_tableWidget->setItem(i, 0, item1);
		d->m_tableWidget->setItem(i, 1, item2);
		d->m_tableWidget->setItem(i, 2, item3);
	}
}

inline const SongSearchDetailedInfo &NetworkSearchFrame::getSongInfo(int index) {
	index += d->m_offset;
	return d->m_songSearchDetailedInfos.at(index);
}

void NetworkSearchFrame::playSong(int index) {
	SongSearchDetailedInfo songSearchDetailedInfo = getSongInfo(index);
	QString songName = songSearchDetailedInfo.song_name;
	QString songId = songSearchDetailedInfo.song_id;
	QString songLink = d->m_networkApi->getSongLink(songId);
	QString lyricLink = d->m_networkApi->getLyricLink(songId);
	emit play(songName, songLink, lyricLink);
}

QList<SongSearchDetailedInfo> NetworkSearchFrame::search() {
	if (!d->m_networkApi) {
		printf("please setNetworkApi before search\n");
		fflush(stdout);
		return QList<SongSearchDetailedInfo>();
	}
	return d->m_networkApi->searchSong(d->m_songName, d->m_offset);
	/*QString className = d->m_networkApi->metaObject()->className();
	if (className == "YApi") {
		return qobject_cast<YApi *>(d->m_networkApi)->searchSong_new(d->m_songName, d->m_offset);
	}
	else if (className == "QQApi") {
		return qobject_cast<QQApi *>(d->m_networkApi)->searchSong(d->m_songName, d->m_offset);
	}
	return QList<SongSearchDetailedInfo>();*/
}

inline void NetworkSearchFrame::itemDoubleClicked(QTableWidgetItem *item) {
	int index = d->m_tableWidget->row(item);
	playSong(index);
}

NetworkSearchFrame::~NetworkSearchFrame() {
	delete d;
}