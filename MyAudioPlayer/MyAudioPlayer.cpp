#include "MyAudioPlayer.h"
#include <qfiledialog.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qslider.h>
#include <QTime>
#include "TrayMenu.h"
#include <QCloseEvent>
#include <qmenu.h>
#include <QAction>
#include <qscrollbar.h>
#include <qmessagebox.h>
#include <qactiongroup.h>
#include <qsettings.h>
#include <Windows.h>
#include <vector>
#include "HookDll.h"
#include "UntranslatedLyricWidget.h"
#include "TranslatedLyricWidget.h"
#include "ApiBase.h"
#include "NetworkSearchFrame.h"
#include "LocalApi.h"
#include "QQApi.h"
#include "YApi.h"
#include "TcpClient.h"
#include "Global.h"

#include "ui_AudioPlayer.h"

struct AudioPlayerDataPrivate : public Ui::AudioPlayer{
	QMediaPlayer m_mediaPlayer;
	bool m_isPaused;				// 当前播放是否暂停
	int m_songCount;			// 歌曲总数量
	int m_currentMusicIndex;
	int m_currentSonglistIndex;
	QString m_currentSonglist;		// 当前歌单名
	QString m_oldSonglistName;		// 当前被修改的原歌单名
	QList<SongSrcInfo> m_songSrcInfoList;
	QList<int> m_hiddenItem;
	QMap<int, SongDetailedInfo> m_songLinks;			// 和m_alreadyPlayedIndex一样在每次歌单发生改变时，都应该清空
	QList<int> m_alreadyPlayedIndex;
	QMenu *m_songMenu;
	QMenu *m_songlistMenu;
	QMenu *m_addtoSonglistMenu;
	QActionGroup *m_actionGroup;
	QSystemTrayIcon *m_system_tray;
	MyAudioPlayer::PlayMode m_playMode;
	QScopedPointer<LyricWidget> m_lyricWidget;
	NetworkSearchFrame *m_networkSearchFrame;
	bool m_isChangeSearch;
	bool m_isTranslatedLyric;
	bool m_isShowLyric;
	QString m_searchSrc;
	QString m_lyric;
	QString m_translatedLyric;
	QScopedPointer<ApiBase> m_musicApi;
	QStringList m_songlistList;
	bool m_isAppQuit;
	TcpClient *m_tcpClient;
};

MyAudioPlayer::MyAudioPlayer(QWidget *parent)
	: QWidget(parent)
	, d(new AudioPlayerDataPrivate) {

	d->setupUi(this);

	QSettings setting(qApp->applicationDirPath() + "/data/init.ini", QSettings::IniFormat);
	setting.beginGroup("MyAudioPlayer");
	d->m_currentSonglistIndex = setting.value("currentSonglistIndex", 0).toInt();
	d->m_currentSonglist = setting.value("currentSonglist", "computer").toString();
	d->m_currentMusicIndex = setting.value("currentMusicIndex", -1).toInt();
	d->m_isTranslatedLyric = setting.value("isTranslatedLyric", false).toBool();
	d->m_isShowLyric = setting.value("isShowLyric", false).toBool();
	setting.endGroup();
	
	d->m_isAppQuit = false;
	d->m_songMenu = new QMenu(this);
	d->m_songlistMenu = new QMenu(this);
	d->m_actionGroup = new QActionGroup(this);
	d->m_isPaused = true;
	d->m_currentSonglistLabel->setText(d->m_currentSonglist);
	d->m_networkSearchFrame = nullptr;
	d->m_isChangeSearch = false;
	d->m_searchSrc = "ymusic";
	d->m_playMode = MyAudioPlayer::Shuffle;
	if (!d->m_isTranslatedLyric)
		d->m_lyricWidget.reset(new UntranslatedLyricWidget());
	else
		d->m_lyricWidget.reset(new TranslatedLyricWidget());
	d->m_tcpClient = new TcpClient(this);
	d->m_tcpClient->init();

	getMusicApi();
	d->m_musicApi->init();
	loadStyle(this, ":/QSS/data/QSS/MyAudioPlayer.qss");
	
	loadPlaylist();
	loadItem();
	initTrayMenu();
	initAudioPlayer();

	qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

	QAction *play_action = d->m_songMenu->addAction("play");
	d->m_addtoSonglistMenu = d->m_songMenu->addMenu("add to");
	addSonglistMenu();
	d->m_songMenu->addSeparator();
	QAction *del_action = d->m_songMenu->addAction("delete");
	d->m_actionGroup->setExclusive(true);

	connect(play_action, &QAction::triggered, [this]() {
		int index = d->m_songListWidget->currentRow();
		d->m_songListWidget->setCurrentRow(index);
		playNext(index);
	});
	connect(d->m_actionGroup, &QActionGroup::triggered, [this](QAction *action) {
		QString songlistName = action->text();
		QList<QListWidgetItem *> items = d->m_songListWidget->selectedItems();     // 获取所有被选择的歌曲
		for (QListWidgetItem *item : items) {
			int index = d->m_songListWidget->row(item);
			d->m_musicApi->addSong(songlistName, d->m_songSrcInfoList.at(index).m_songIndex);
		}
	});
	connect(del_action, &QAction::triggered, [this]() {
		if (!d->m_hiddenItem.isEmpty())
			d->m_searchLineEdit->clear();
		resetSongList();
		QList<QListWidgetItem *> items = d->m_songListWidget->selectedItems();     // 获取所有被选择的歌曲
		for (QListWidgetItem *item : items) {
			int index = d->m_songListWidget->row(item);
			if (index == d->m_currentMusicIndex)
				d->m_mediaPlayer.pause();
			item = d->m_songListWidget->takeItem(index);
			delete item;
			SongSrcInfo songSrcInfo = d->m_songSrcInfoList.takeAt(index);
			d->m_musicApi->deleteSong(songSrcInfo.m_songIndex, d->m_currentSonglist);
			d->m_songCount--;
		}
		if (!(d->m_mediaPlayer.state() == QMediaPlayer::PlayingState))
			playNext();
	});
}

void MyAudioPlayer::addSonglistMenu() {
	QList<QAction *> actions = d->m_addtoSonglistMenu->actions();
	for (QAction *action : actions) {
		d->m_addtoSonglistMenu->removeAction(action);
		d->m_actionGroup->removeAction(action);
		action->deleteLater();
	}
	for (QString songlist : d->m_songlistList) {
		if (songlist == d->m_currentSonglist)
			continue;
		addSonglistAction(songlist);
	}
}

inline void MyAudioPlayer::addSonglistAction(const QString &songlistName) {
	QAction *songlistAction = d->m_addtoSonglistMenu->addAction(songlistName);
	d->m_actionGroup->addAction(songlistAction);
}

void MyAudioPlayer::loadStyle(QWidget *widget, const QString &fileName) {
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly)) {
		printf("qss file name error!\n");
		fflush(stdout);
		return;
	}
	widget->setStyleSheet(file.readAll());
	file.close();
}

void MyAudioPlayer::initTrayMenu() {
	d->m_system_tray = new QSystemTrayIcon(this);
	d->m_system_tray->setToolTip(QString("I am tray"));
	d->m_system_tray->setIcon(QIcon(":/MyAudioPlayer/data/icon/format.ico"));
	TrayMenu *tray_menu = new TrayMenu(this);
	d->m_system_tray->setContextMenu(tray_menu);
	connect(tray_menu, &TrayMenu::showWidget, this, &QWidget::show);
	connect(d->m_system_tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconIsActived(QSystemTrayIcon::ActivationReason)));
	d->m_system_tray->show();
}

void MyAudioPlayer::initAudioPlayer() {
	setShortcutHook();

	d->m_volumeSlider->hide();
	d->m_networkSearchEdit->hide();
	d->m_networkSearchBackButton->hide();
	d->m_searchBackButton->hide();
	d->m_searchNextButton->hide();
	d->m_searchSrcBox->addItem("ymusic");
	d->m_searchSrcBox->addItem("qqmusic");
	d->m_searchSrcBox->hide();

	loadStyle(d->m_songListWidget->verticalScrollBar(), ":/QSS/data/QSS/Scrollbar.qss");
	loadStyle(d->m_songlistListWidget->verticalScrollBar(), ":/QSS/data/QSS/Scrollbar.qss");
	loadStyle(d->m_playButton, ":/QSS/data/QSS/PlayButton.qss");
	loadStyle(d->m_playOrderButton, ":/QSS/data/QSS/PlayOrderButton.qss");

	connect(d->m_searchSrcBox, QOverload<const QString &>::of(&QComboBox::activated), [this](const QString &text) {
		d->m_searchSrc = text;
		d->m_isChangeSearch = true;
	});
	connect(d->m_localSongButton, &QPushButton::clicked, this, &MyAudioPlayer::openFile);
	connect(d->m_searchLineEdit, &QLineEdit::returnPressed, this, &MyAudioPlayer::searchSong);
	connect(d->m_searchLineEdit, &QLineEdit::textChanged, this, &MyAudioPlayer::searchSong);
	connect(d->m_networkSearchButton, &QPushButton::clicked, [this]() {
		d->m_searchLineEdit->hide();
		d->m_networkSearchButton->hide();
		d->m_networkSearchEdit->show();
		d->m_networkSearchEdit->setFocus();
		d->m_networkSearchBackButton->show();
		d->m_searchSrcBox->show();
	});
	connect(d->m_networkSearchBackButton, &QPushButton::clicked, [this]() {
		d->m_isChangeSearch = false;
		d->m_searchLineEdit->show();
		d->m_networkSearchBackButton->hide();
		d->m_networkSearchEdit->hide();
		d->m_networkSearchEdit->clear();
		d->m_searchBackButton->hide();
		d->m_searchNextButton->hide();
		d->m_searchSrcBox->hide();
		d->m_networkSearchButton->show();
		if (d->m_networkSearchFrame) {
			d->m_networkSearchFrame->deleteLater();
			d->m_networkSearchFrame = nullptr;
		}
	});
	connect(d->m_networkSearchEdit, &QLineEdit::returnPressed, this, &MyAudioPlayer::searchNetworkSong);
	connect(d->m_volumeButton, &QPushButton::clicked, [this]() {
		if (d->m_volumeSlider->isVisible()) {
			d->m_volumeSlider->hide();
			d->m_songListWidget->setFocus();
		}
		else {
			d->m_volumeSlider->show();
			d->m_volumeSlider->setFocus();
		}
	});
	connect(d->m_volumeSlider, &QSlider::valueChanged, &d->m_mediaPlayer, &QMediaPlayer::setVolume);
	connect(d->m_songlistButton, &QPushButton::clicked, [this]() {
		for (int i = 0; i < d->m_songlistListWidget->count(); i++) {
			QListWidgetItem *item = d->m_songlistListWidget->item(i);
			if (item->isHidden())
				item->setHidden(false);
			else
				item->setHidden(true);
		}
	});
	connect(d->m_addSonglistButton, &QPushButton::clicked, [this]() {
		QString songlist = d->m_musicApi->getNewSonglist();
		QListWidgetItem *item = addItem(d->m_songlistListWidget, songlist);
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		d->m_songlistListWidget->setCurrentItem(item);
		d->m_songlistListWidget->editItem(item);
		d->m_oldSonglistName = item->text();
	});
	connect(d->m_songlistListWidget, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
		if (d->m_currentSonglist == item->text() || !d->m_songlistList.contains(item->text()))
			return;
		d->m_currentSonglist = item->text();
		d->m_currentSonglistLabel->setText(d->m_currentSonglist);
		d->m_currentSonglistIndex = d->m_songlistListWidget->row(item);
		loadItem();
		addSonglistMenu();
	});
	connect(d->m_songlistListWidget, &QListWidget::customContextMenuRequested, [this](const QPoint &pos) {
		execListWidgetMenu(d->m_songlistListWidget, pos, d->m_songlistMenu);
	});
	connect(d->m_songListWidget, &QListWidget::itemDoubleClicked, [this](QListWidgetItem *item) {
		d->m_alreadyPlayedIndex.clear();
		playNext(d->m_songListWidget->row(item));
		if (!d->m_searchLineEdit->text().isEmpty())
			d->m_searchLineEdit->clear();
	});
	connect(d->m_songListWidget, &QListWidget::customContextMenuRequested, [this](const QPoint &pos) {
		execListWidgetMenu(d->m_songListWidget, pos, d->m_songMenu);
	});
	connect(d->m_playButton, &QPushButton::clicked, this, &MyAudioPlayer::togglePlayback);
	connect(d->m_previousButton, &QPushButton::clicked, this, &MyAudioPlayer::playPrevious);
	connect(d->m_nextButton, &QPushButton::clicked, [this]() {
		playNext();
	});
	connect(&d->m_mediaPlayer, &QMediaPlayer::durationChanged, this, &MyAudioPlayer::updateDuration);
	connect(&d->m_mediaPlayer, &QMediaPlayer::positionChanged, this, &MyAudioPlayer::updatePosition);
	connect(&d->m_mediaPlayer, &QMediaPlayer::stateChanged, this, &MyAudioPlayer::updateState);
	connect(&d->m_mediaPlayer, qOverload<QMediaPlayer::Error>(&QMediaPlayer::error), [this]() {
		output << d->m_mediaPlayer.errorString();
	});
	connect(d->m_positionSlider, &QSlider::valueChanged, this, &MyAudioPlayer::setPosition);
	connect(d->m_playOrderButton, &QPushButton::clicked, [this]() {
		switch (d->m_playMode)
		{
		case MyAudioPlayer::Shuffle:
			d->m_playMode = MyAudioPlayer::Repeat;
			d->m_playOrderButton->setIcon(QIcon(":/MyAudioPlayer/data/icon/repeat.png"));
			d->m_playOrderButton->setAccessibleName("repeat");
			break;
		case MyAudioPlayer::Repeat:
			d->m_playMode = MyAudioPlayer::Single;
			d->m_playOrderButton->setIcon(QIcon(":/MyAudioPlayer/data/icon/single.png"));
			d->m_playOrderButton->setAccessibleName("single");
			break;
		case MyAudioPlayer::Single:
			d->m_playMode = MyAudioPlayer::Shuffle;
			d->m_playOrderButton->setIcon(QIcon(":/MyAudioPlayer/data/icon/shuffle.png"));
			d->m_playOrderButton->setAccessibleName("shuffle");
			break;
		default:
			break;
		}
		loadStyle(d->m_playOrderButton, ":/QSS/data/QSS/PlayOrderButton.qss");
	});
	connect(d->m_lyricButton, &QPushButton::clicked, [this]() {
		if (d->m_lyricWidget->isVisible())
			d->m_lyricWidget->hide();
		else
			d->m_lyricWidget->show();
		d->m_isShowLyric = d->m_lyricWidget->isVisible();
	});
	connect(d->m_lyricTranslatedButton, &QPushButton::clicked, [this]() {
		QString lyric = "";
		if (d->m_isTranslatedLyric) {
			d->m_lyricWidget.reset(new UntranslatedLyricWidget());
			lyric = d->m_lyric;
			d->m_isTranslatedLyric = false;
		}
		else {
			d->m_lyricWidget.reset(new TranslatedLyricWidget());
			lyric = d->m_lyric + "--" + d->m_translatedLyric;
			d->m_isTranslatedLyric = true;
		}
		d->m_lyricWidget->setLyric(lyric, d->m_mediaPlayer.position());
	});
}

void MyAudioPlayer::setShortcutHook() {
	std::vector<std::vector<int>> vk_keys;
	std::vector<std::function<void()>> functions;
	std::vector<int> vk_key;
	vk_key.reserve(2);
	vk_key.push_back(VK_CONTROL);
	vk_key.push_back('P');
	vk_keys.push_back(vk_key);
	functions.push_back(std::bind(&MyAudioPlayer::togglePlayback, this));

	vk_key.pop_back();
	vk_key.push_back(VK_LEFT);
	vk_keys.push_back(vk_key);
	functions.push_back(std::bind(&MyAudioPlayer::playPrevious, this));

	vk_key.pop_back();
	vk_key.push_back(VK_RIGHT);
	vk_keys.push_back(vk_key);
	functions.push_back([this]() {playNext(); });

	SetFunctions(vk_keys, functions);
	SetHook();
}

void MyAudioPlayer::iconIsActived(QSystemTrayIcon::ActivationReason reason) {
	switch (reason)
	{
		//点击托盘显示窗口
	case QSystemTrayIcon::Trigger:
	{
		show();
		break;
	}
	//双击托盘显示窗口
	case QSystemTrayIcon::DoubleClick:
	{
		if (isVisible())
			hide();
		else
			showNormal();
		break;
	}
	default:
		break;
	}
}

void MyAudioPlayer::execListWidgetMenu(QListWidget *listWidget, const QPoint &pos, QMenu *menu) {
	QList<QListWidgetItem *> items = listWidget->selectedItems();
	if (items.isEmpty())
		return;
	for (auto itr = items.begin(); itr != items.end(); itr++) {
		if ((*itr) == listWidget->itemAt(pos)) {
			menu->exec(QCursor::pos());
			return;
		}
	}
}

inline void MyAudioPlayer::resetSongList() {
	d->m_songLinks.swap(QMap<int, SongDetailedInfo>());
	d->m_alreadyPlayedIndex.swap(QList<int>());
}

// 从本地文件打开
void MyAudioPlayer::openFile()
{
	const QString musicPath = QString("E:\\KuGou");
	const QStringList filePaths =
		QFileDialog::getOpenFileNames(this, "Open File",
			musicPath,
			"MP3 files (*.mp3 *.ape *.flac);;All files (*.*)");
	if (!filePaths.isEmpty()) {
		if (!d->m_hiddenItem.isEmpty())
			d->m_searchLineEdit->clear();
		resetSongList();
		getMusicApi("local");
		for (QString filePath : filePaths) {
			QString songName = QFileInfo(filePath).baseName();
			//filePath.replace(QRegularExpression("\\"), "\\\\");			// 由于得到的路径为/故这里无需替换
			addSong(filePath, songName, "t_local", d->m_currentSonglist);
		}
		playNext(0);
	}
}

void MyAudioPlayer::addSong(const QString &songId, const QString &songName, const QString &table, const QString &songlist) {
	int index = d->m_musicApi->addSong(songName, songId, songlist);
	if (index < 0)
		return;
	insertItem(d->m_songListWidget, 0, songName);
	SongSrcInfo songSrcInfo;
	songSrcInfo.m_songIndex = index;
	songSrcInfo.m_songSrcTabel = table;
	d->m_songSrcInfoList.prepend(songSrcInfo);
	d->m_songCount++;
}

inline QListWidgetItem *MyAudioPlayer::addItem(QListWidget *listWidget, const QString &songName) {
	return insertItem(listWidget, listWidget->count(), songName);
}

inline QListWidgetItem *MyAudioPlayer::insertItem(QListWidget *listWidget, int index, const QString &songName) {
	QListWidgetItem *item = new QListWidgetItem(songName);
	item->setTextColor(QColor(200, 200, 200));
	listWidget->insertItem(index, item);
	return item;
}

void MyAudioPlayer::loadPlaylist() {
	d->m_songlistListWidget->installEventFilter(this);
	loadStyle(d->m_songlistListWidget, ":/QSS/data/QSS/SonglistListWidget.qss");
	d->m_songlistList = d->m_musicApi->loadSonglist();
	for (QString songlist : d->m_songlistList) {
		QListWidgetItem *item = addItem(d->m_songlistListWidget, songlist);
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		if (item->text() == d->m_currentSonglist)
			d->m_songlistListWidget->setCurrentItem(item);
	}
	QAction *renameAction = d->m_songlistMenu->addAction("rename");
	d->m_songlistMenu->addSeparator();
	QAction *delAction = d->m_songlistMenu->addAction("delete");

	connect(renameAction, &QAction::triggered, [this]() {
		QListWidgetItem *item = d->m_songlistListWidget->currentItem();
		d->m_songlistListWidget->editItem(item);
		d->m_oldSonglistName = item->text();
	});
	connect(delAction, &QAction::triggered, [this]() {
		QListWidgetItem *item = d->m_songlistListWidget->currentItem();
		if (d->m_songlistListWidget->count() <= 1 || d->m_songlistListWidget->row(item) == 0) {
			return;
		}
		int ret = QMessageBox::warning(this, "Confirm delete",
			QString("are you sure you want to delete this songlist named %1?").arg(item->text()),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
		if (ret == QMessageBox::No) {
			return;
		}
		d->m_songlistListWidget->removeItemWidget(item);
		d->m_musicApi->deleteSonglist(item->text());
		d->m_songlistList.removeOne(item->text());
		if(d->m_currentSonglist == item->text())
			d->m_songListWidget->clear();
		delete item;
		d->m_songlistListWidget->setCurrentRow(0);
		addSonglistMenu();
	});
}

void MyAudioPlayer::addSonglist(QListWidgetItem *item) {
	if (!d->m_musicApi->addSonglist(item->text())) {
		qDebug() << "this songlist already exists, add failed!";
		item->setText(d->m_oldSonglistName);
		d->m_songlistListWidget->editItem(item);
		return;
	}
	d->m_songlistList.append(item->text());
	d->m_songlistListWidget->setCurrentRow(d->m_currentSonglistIndex);
	addSonglistAction(item->text());
}

void MyAudioPlayer::renameSonglist(QListWidgetItem *item) {
	if (item->text() == d->m_oldSonglistName)
		return;
	if (!d->m_musicApi->renameSonglist(item->text(), d->m_oldSonglistName)) {
		qDebug() << "this songlist already exists, rename failed!";
		item->setText(d->m_oldSonglistName);
		d->m_songlistListWidget->editItem(item);
		return;
	}
	int index = d->m_songlistListWidget->row(item);
	d->m_songlistList.replace(index, item->text());
	d->m_currentSonglist = item->text();
	addSonglistMenu();
}

// 根据歌单加载歌曲
void MyAudioPlayer::loadItem() {
	resetSongList();
	d->m_songCount = 0;
	d->m_songListWidget->clear();
	QList<SongSrcInfo>().swap(d->m_songSrcInfoList);
	QList<SongBaseInfo> songBaseInfoList = d->m_musicApi->loadSong(d->m_currentSonglist);
	for (SongBaseInfo songBaseInfo : songBaseInfoList) {
		addItem(d->m_songListWidget, songBaseInfo.m_songName);
		SongSrcInfo songSrcInfo;
		songSrcInfo.m_songIndex = songBaseInfo.m_songIndex;
		songSrcInfo.m_songSrcTabel = songBaseInfo.m_songSrcTabel;
		d->m_songSrcInfoList.append(songSrcInfo);
		d->m_songCount++;
	}
	d->m_alreadyPlayedIndex.reserve(d->m_songCount);
	d->m_songListWidget->setCurrentRow(d->m_currentMusicIndex);
}

inline int MyAudioPlayer::getRandomNum() {
	int num = qrand() % d->m_songCount;
	return num;
}

// 根据播放模式获取id
int MyAudioPlayer::getPlayNum() {
	int playNum = 0;
	switch (d->m_playMode)
	{
	case MyAudioPlayer::Shuffle:
		playNum = getRandomNum();
		break;
	case MyAudioPlayer::Repeat:
		playNum = d->m_songListWidget->currentRow() + 1;
		playNum = playNum >= d->m_songListWidget->count() ? 0 : playNum;
		break;
	case MyAudioPlayer::Single:
		d->m_alreadyPlayedIndex.clear();
		playNum = d->m_songListWidget->currentRow();
		break;
	default:
		break;
	}
	return playNum;
}

// 下一曲
void MyAudioPlayer::playNext(int index) {
	int currentIndex = d->m_songListWidget->currentRow();
	if (currentIndex >= 0 && currentIndex < d->m_songCount) {
		if (!d->m_alreadyPlayedIndex.contains(currentIndex) && (currentIndex != index)) {
			if ((d->m_alreadyPlayedIndex.size() >= (d->m_songCount / 4 * 3)) && (d->m_alreadyPlayedIndex.size() > 0))
				d->m_alreadyPlayedIndex.removeLast();
			if(d->m_songCount > 1)
				d->m_alreadyPlayedIndex.prepend(currentIndex);
		}
	}
	replay(index);
}

void MyAudioPlayer::playPrevious() {
	int index = -1;
	if (d->m_alreadyPlayedIndex.size() > 0)
		index = d->m_alreadyPlayedIndex.takeFirst();
	replay(index);
}

// 切换歌曲, 强制终止上一首歌曲
void MyAudioPlayer::replay(int index) {
	if (d->m_songListWidget->count() < 1)
		return;
	stop();
	if (d->m_networkSearchFrame) {
		if(!d->m_isPaused)
			d->m_playButton->click();
		return;
	}
	if (index == -1) {
		do {
			index = getPlayNum();
		} while (d->m_alreadyPlayedIndex.contains(index));
	}
	d->m_currentMusicIndex = index;
	QListWidgetItem *item = d->m_songListWidget->item(index);
	d->m_songListWidget->setCurrentItem(item);
	QString songName = item->text();
	SongDetailedInfo songDetailedInfo = getSongDetailedInfo(index);
	play(songName, songDetailedInfo.m_songMediaLink, songDetailedInfo.m_songLyricLink);
}

void MyAudioPlayer::play(const QString &songName, const QString &songLink, const QString &lyricLink) {
	output << songName;
	d->m_infoLabel->setText(songName);
	d->m_lyric = d->m_musicApi->getLyric(lyricLink);
	d->m_translatedLyric = d->m_musicApi->getTranslatedLyric(lyricLink);
	playFile(songLink);
	QString lyric = "";
	if (d->m_isTranslatedLyric) {
		if (d->m_translatedLyric.isEmpty()) {
			lyric = d->m_lyric;
			if(QString(d->m_lyricWidget->metaObject()->className()) != "UntranslatedLyricWidget")
				d->m_lyricWidget.reset(new UntranslatedLyricWidget);
		}
		else {
			if (QString(d->m_lyricWidget->metaObject()->className()) != "TranslatedLyricWidget")
				d->m_lyricWidget.reset(new TranslatedLyricWidget);
			lyric = d->m_lyric + "--" + d->m_translatedLyric;
		}
	}
	else 
		lyric = d->m_lyric;
	d->m_lyricWidget->setLyric(lyric, 0);
	d->m_lyricWidget->setVisible(d->m_isShowLyric);
}

SongDetailedInfo &MyAudioPlayer::getSongDetailedInfo(int index) {
	SongSrcInfo songSrcInfo = d->m_songSrcInfoList.at(index);
	getMusicApi(dbTableNameToApiName(songSrcInfo.m_songSrcTabel));
	auto itr = d->m_songLinks.find(index);
	if (itr != d->m_songLinks.end()) {
		return itr.value();
	}
	SongDetailedInfo songDetailedInfo;
	int song_index = songSrcInfo.m_songIndex;
	songDetailedInfo.m_songIndex = song_index;
	songDetailedInfo.m_songMediaLink = d->m_musicApi->getSongLink(song_index);
	songDetailedInfo.m_songLyricLink = d->m_musicApi->getLyricLink(song_index);
	d->m_songLinks.insert(index, songDetailedInfo);
	return d->m_songLinks[index];
}

// 只由play调用
void MyAudioPlayer::playFile(const QString &filePath)
{
	d->m_playButton->setEnabled(true);

	QUrl url;
	if (filePath.startsWith("http"))
		url = QUrl(filePath);
	else {
		d->m_infoLabel->setText(QFileInfo(filePath).baseName());
		url = QUrl::fromLocalFile(filePath);
	}
	
	d->m_mediaPlayer.setMedia(url);
	d->m_mediaPlayer.play();
	d->m_isPaused = false;
}

// 暂停整个播放操作
void MyAudioPlayer::pause() {
	d->m_mediaPlayer.pause();		// 当播放新的歌曲时，强制暂停上一首歌
	d->m_lyricWidget->stop();
}

// 停止播放
void MyAudioPlayer::stop() {
	d->m_mediaPlayer.stop();
	d->m_lyricWidget->stop();
}

void MyAudioPlayer::getMusicApi(const QString &apiName) {
	QString className = "";
	if(d->m_musicApi)
		className = d->m_musicApi->metaObject()->className();
	if (apiName == "local" && className != "LocalApi") {
		d->m_musicApi.reset(new LocalApi());
	}
	else if (apiName == "ymusic" && className != "YApi") {
		d->m_musicApi.reset(new YApi());
	}
	else if (apiName == "qqmusic" && className != "QQApi") {
		d->m_musicApi.reset(new QQApi());
	}
	else if(apiName.isEmpty() && className != "ApiBase"){
		d->m_musicApi.reset(new ApiBase());
	}
}

inline QString MyAudioPlayer::dbTableNameToApiName(const QString &songSrcTabel) {
	return songSrcTabel.right(songSrcTabel.size() - 2);
}

void MyAudioPlayer::searchSong() {
	QString text = d->m_searchLineEdit->text();
	if (text.isEmpty()) {
		for (int i = 0; i < d->m_hiddenItem.size(); i++) {
			d->m_songListWidget->item(d->m_hiddenItem.at(i))->setHidden(false);
		}
		QList<int>().swap(d->m_hiddenItem);
		return;
	}
	if(d->m_hiddenItem.size() == 0)
		d->m_hiddenItem.reserve(d->m_songCount);
	for (int i = 0; i < d->m_songCount; i++) {
		QListWidgetItem *item = d->m_songListWidget->item(i);
		QString songName = item->text();
		if (!songName.contains(text, Qt::CaseInsensitive)) {
			if (!d->m_hiddenItem.contains(i)) {
				d->m_hiddenItem.append(i);
				item->setHidden(true);
			}
		}
		else {
			if (d->m_hiddenItem.contains(i)) {
				d->m_hiddenItem.removeAt(i);
				item->setHidden(false);
			}
		}
	}
}

void MyAudioPlayer::searchNetworkSong() {
	QString songName = d->m_networkSearchEdit->text();
	if (!d->m_networkSearchFrame) {
		d->m_searchBackButton->show();
		d->m_searchNextButton->show();
		d->m_networkSearchFrame = new NetworkSearchFrame(d->m_songListWidget->size(), d->m_songlistList, this);
		connect(d->m_networkSearchFrame, &NetworkSearchFrame::play, this, &MyAudioPlayer::play);
		connect(d->m_networkSearchFrame, &NetworkSearchFrame::addSong, [this](const QString &songId, const QString &songName, const QString &table, const QString &playlist) {
			resetSongList();
			addSong(songId, songName, table, playlist);
		});
		connect(d->m_searchBackButton, &QPushButton::clicked, d->m_networkSearchFrame, &NetworkSearchFrame::previousSearch);
		connect(d->m_searchNextButton, &QPushButton::clicked, d->m_networkSearchFrame, &NetworkSearchFrame::nextSearch);
		d->m_isChangeSearch = true;
	}
	if (d->m_isChangeSearch) {
		getMusicApi(d->m_searchSrc);
		d->m_networkSearchFrame->setNetworkApi(qobject_cast<NetworkApiBase *>(d->m_musicApi.data()));
		d->m_isChangeSearch = false;
	}
	d->m_networkSearchFrame->setGeometry(d->m_songListWidget->geometry());
	d->m_networkSearchFrame->searchSong(songName);
}

void MyAudioPlayer::togglePlayback()
{
	if (d->m_mediaPlayer.mediaStatus() == QMediaPlayer::NoMedia)
		d->m_currentMusicIndex < d->m_songCount ? playNext(d->m_currentMusicIndex) : playNext();
	else if (d->m_mediaPlayer.state() == QMediaPlayer::PlayingState) {
		d->m_isPaused = true;
		d->m_lyricWidget->stop();
		d->m_mediaPlayer.pause();
	}
	else {
		d->m_isPaused = false;
		d->m_lyricWidget->restart();
		d->m_mediaPlayer.play();
	}
}

inline void MyAudioPlayer::seekForward(){
	d->m_positionSlider->triggerAction(QSlider::SliderSingleStepAdd);
}

inline void MyAudioPlayer::seekBackward(){
	d->m_positionSlider->triggerAction(QSlider::SliderSingleStepSub);
}

void MyAudioPlayer::updateState(QMediaPlayer::State state)
{
	if (state == QMediaPlayer::PlayingState) {
		d->m_playButton->setToolTip(tr("Pause"));
		d->m_playButton->setIcon(QIcon(":/MyAudioPlayer/data/icon/pause.png"));
		d->m_playButton->setAccessibleName("pause");
	}
	else {
		d->m_playButton->setToolTip(tr("Play"));
		d->m_playButton->setIcon(QIcon(":/MyAudioPlayer/data/icon/play.png"));
		d->m_playButton->setAccessibleName("play");
	}
	loadStyle(d->m_playButton, ":/QSS/data/QSS/PlayButton.qss");
}

void MyAudioPlayer::updateDuration(qint64 duration)
{
	d->m_positionSlider->setRange(0, duration);
	d->m_positionSlider->setEnabled(duration>0);
	d->m_positionSlider->setPageStep(duration / 10);
	d->m_positionSlider->setSingleStep(duration / 100);
}

void MyAudioPlayer::updatePosition(qint64 position){
	d->m_positionSlider->setValue(position);
	// 由于秒钟最大值为59，故不能采用四舍五入
	//QTime duration(0, position / 60000, qRound((position % 60000) / 1000.0));
	QTime duration(0, position / 60000, (position % 60000) / 1000);
	d->m_lyricWidget->setTime(position);					// 将当前时间传递给lyricWidget，以处理歌词
	qint64 mediaAllDuration = d->m_mediaPlayer.duration();
	//QTime allDuration(0, mediaAllDuration / 60000, qRound((mediaAllDuration % 60000) / 1000.0));
	QTime allDuration(0, mediaAllDuration / 60000, (mediaAllDuration % 60000) / 1000);
	d->m_positionLabel->setText(duration.toString(tr("mm:ss")) + "/" + allDuration.toString("mm:ss"));
	if (position == d->m_mediaPlayer.duration() && position != 0 && !d->m_isPaused) {
		d->m_isPaused = true;
		playNext();
	}
}

void MyAudioPlayer::setPosition(int position)
{
	if (qAbs(d->m_mediaPlayer.position() - position) > (5 * 100))
		d->m_mediaPlayer.setPosition(position);
}

void MyAudioPlayer::updateInfo()
{
	QStringList info;
	QString author = d->m_mediaPlayer.metaData("Author").toString();
	if (!author.isEmpty())
		info += author;
	QString title = d->m_mediaPlayer.metaData("Title").toString();
	if (!title.isEmpty())
		info += title;
	if (!info.isEmpty())
		d->m_infoLabel->setText(info.join(tr(" - ")));
}

bool MyAudioPlayer::eventFilter(QObject *obj, QEvent *event) {
	if (obj == d->m_songlistListWidget) {
		QListWidgetItem *item = d->m_songlistListWidget->currentItem();
		static bool isChangedText = false;
		static bool isFocus = false;
		if (isChangedText && !d->m_songlistListWidget->isPersistentEditorOpen(item)) {
			if (d->m_songlistList.contains(d->m_oldSonglistName)) {
				renameSonglist(item);
			}
			else {
				addSonglist(item);
			}
			isChangedText = false;
		}
		if (event->type() == QEvent::FocusOut)
			isFocus = false;
		else if (event->type() == QEvent::FocusIn)
			isFocus = true;
		if(!isFocus && !isChangedText)
			isChangedText = d->m_songlistListWidget->isPersistentEditorOpen(item);
		/*if (event->type() == QEvent::KeyPress) {
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
			if (keyEvent->key() == Qt::Key_Return) {
				isChangedText = d->m_songlistListWidget->isPersistentEditorOpen(item);
			}
		}
		else if (event->type() == QEvent::KeyRelease) {
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
			if (keyEvent->key() == Qt::Key_Return) {
				if (isChangedText) {
					if (d->m_songlistList.contains(d->m_oldSonglistName)) {
						renameSonglist(item);
					}
					else {
						addSonglist(item);
					}
				}
			}
		}*/
	}
	return QWidget::eventFilter(obj, event);
}

// 若不调用ignore，或调用accept，则事件将被处理，即窗口关闭，此函数用作窗口被关闭之前的预处理
void MyAudioPlayer::closeEvent(QCloseEvent *event) {
	hide();
	event->ignore();    // ignore表示将此事件忽略不处理，交给其他窗口处理，但如果return，表示此事件将不被处理，accept表示此事件被处理，即窗口关闭
	return;
}

void MyAudioPlayer::keyPressEvent(QKeyEvent *event) {
	switch (event->key())
	{
	case Qt::Key_Left:
		seekBackward();
		break;
	case Qt::Key_Right:
		seekForward();
		break;
	default:
		break;
	}
}

void MyAudioPlayer::quit() {
	if (!d->m_isAppQuit) {
		UnsetHook();
		QSettings setting(qApp->applicationDirPath() + "/data/init.ini", QSettings::IniFormat);
		setting.beginGroup("MyAudioPlayer");
		setting.setValue("currentSonglistIndex", d->m_currentSonglistIndex);
		setting.setValue("currentSonglist", d->m_currentSonglist);
		setting.setValue("currentMusicIndex", d->m_currentMusicIndex);
		setting.setValue("isTranslatedLyric", d->m_isTranslatedLyric);
		setting.setValue("isShowLyric", d->m_isShowLyric);
		setting.endGroup();
		d->m_isAppQuit = true;
	}
}

MyAudioPlayer::~MyAudioPlayer() {
	quit();
}
