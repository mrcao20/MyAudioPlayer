#pragma once

#include <qobject.h>
#include <qmap.h>
#include <qlist.h>
#include <Windows.h>
#include <QSqlQuery>
#include <qsqldatabase.h>

struct SongBaseInfo{
	int m_songIndex;
	QString m_songName;
	QString m_songSrcTabel;
};

class ApiBase : public QObject{
	Q_OBJECT

public:
	ApiBase(QObject *parent = 0);
	virtual ~ApiBase(){}

	virtual int addSong(const QString &song_name, const QString &song_src, const QString &songlistName) { return -1; }
	void addSong(const QString &songlistName, int index);
	QStringList loadSonglist();
	QList<SongBaseInfo> loadSong(const QString &songlistName);
	virtual QString getSongLink(const int index) { return ""; }
	virtual QString getLyricLink(const int index) { return ""; }
	virtual QString getLyric(const QString &url){ return ""; }
	virtual QString getTranslatedLyric(const QString &url) { return ""; }
	QString getNewSonglist();
	void deleteSong(int index, const QString &songlistName);
	void deleteSonglist(const QString &songlistName);
	bool addSonglist(const QString &songlistName);
	bool renameSonglist(const QString &newSonglistName, const QString &songlistName);

protected:
	void initDatabase();
	void closeDatabase();
	void addSong(int index, const QString &song_name, const QString &src_tabel, const QString &songlistName);
	void addSong(int index, const QString &songlistName);
	int getMaxSongIndex();

protected:
	QSqlQuery m_query;

private:
	void startSvc(LPSTR szServiceName);
	int getSonglistId(const QString &songlistName);
	void deleteSong(int index, int songlistId);

private:
	QSqlDatabase m_db;

};
