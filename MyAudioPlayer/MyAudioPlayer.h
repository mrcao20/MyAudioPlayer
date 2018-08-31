#pragma once
#pragma execution_character_set("UTF-8")
#include <QWidget>  
#include <qmap.h>
#include <qlist.h>
#include <QMediaPlayer>  
#include <QSystemTrayIcon>
#include <qscopedpointer.h>

QT_BEGIN_NAMESPACE
class QLabel;
class QMenu;
class QSlider;
class QToolButton;
class QCloseEvent;
class QListWidget;
class QListWidgetItem;
QT_END_NAMESPACE

class NetworkSearchFrame;

struct SongSrcInfo{
	int m_songIndex = -1;
	QString m_songSrcTabel = "";
};

struct SongDetailedInfo {
	int m_songIndex = -1;
	QString m_songMediaLink = "";
	QString m_songLyricLink = "";
};

struct AudioPlayerDataPrivate;

class MyAudioPlayer : public QWidget
{
	Q_OBJECT
public:
	explicit MyAudioPlayer(QWidget *parent = 0);
	~MyAudioPlayer();

	enum PlayMode {
		Shuffle,
		Repeat,
		Single
	};

public:
	void pause();

protected:
	void closeEvent(QCloseEvent *event);
	void keyPressEvent(QKeyEvent *event);
	bool eventFilter(QObject *obj, QEvent *event);
	
public slots :
	void openFile();
	void playFile(const QString& filePath);
	void togglePlayback();

private slots:
	void updateState(QMediaPlayer::State state);
	void updatePosition(qint64 position);
	void updateDuration(qint64 duration);
	void setPosition(int position);
	void updateInfo();
	void iconIsActived(QSystemTrayIcon::ActivationReason reason);
	void searchSong();
	void searchNetworkSong();
	void play(const QString &songName, const QString &songLink, const QString &lyricLink = "");
	void playNext(int index = -1);
	void playPrevious();

private:
	void setShortcutHook();
	void seekForward();
	void seekBackward();
	void replay(int index = -1);
	SongDetailedInfo &getSongDetailedInfo(int index);
	void getMusicApi(const QString &apiName = "");
	QString dbTableNameToApiName(const QString &songSrcTabel);
	void addSong(const QString &songId, const QString &songName, const QString &table, const QString &songlist);
	QListWidgetItem *addItem(QListWidget *listWidget, const QString &songName);
	QListWidgetItem *insertItem(QListWidget *listWidget, int index, const QString &songName);
	void execListWidgetMenu(QListWidget *listWidget, const QPoint &pos, QMenu *menu);
	void addSonglistMenu();
	void addSonglistAction(const QString &songlistName);
	void loadPlaylist();
	void loadItem();
	void resetSongList();
	int getRandomNum();
	int getPlayNum();
	void initTrayMenu();
	void initAudioPlayer();
	void loadStyle(QWidget *widget, const QString &fileName);
	void addSonglist(QListWidgetItem *item);
	void renameSonglist(QListWidgetItem *item);

private:
	QScopedPointer<AudioPlayerDataPrivate> d;
	
};