#pragma once
#pragma execution_character_set("UTF-8")
#include <qframe.h>

QT_BEGIN_NAMESPACE
class QTableWidget;
class QTableWidgetItem;
QT_END_NAMESPACE

class NetworkApiBase;
struct SongSearchDetailedInfo;

struct NetworkSearchDataPrivate;

class NetworkSearchFrame : public QFrame{
	Q_OBJECT

public:
	NetworkSearchFrame(QSize size, QStringList playlistList, QWidget *parent = 0);
	~NetworkSearchFrame();

	void searchSong(const QString &songName);
	void previousSearch();
	void nextSearch();
	void setNetworkApi(NetworkApiBase *networkApi);

signals:
	void play(const QString &songName, const QString &songLink, const QString &lyricLink);
	void addSong(const QString &songId, const QString &songName, const QString &table, const QString &playlist);

private slots:
	void itemDoubleClicked(QTableWidgetItem *);

private:
	void addItem();
	void playSong(int index);
	QString getSongType(const QString &songLink);
	QList<SongSearchDetailedInfo> search();
	const SongSearchDetailedInfo &getSongInfo(int index);

private:
	NetworkSearchDataPrivate *d;

};