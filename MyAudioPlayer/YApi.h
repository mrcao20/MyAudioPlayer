#pragma once

#include "NetworkApiBase.h"
#include <qlist.h>

class YApi : public NetworkApiBase{
	Q_OBJECT

public:
	YApi(QObject *parent = 0);
	virtual ~YApi(){}

	int addSong(const QString &song_name, const QString &song_id, const QString &songlistName);
	QString getSongLink(const int index);
	QString getSongLink(const QString &songId);
	QString getLyricLink(const int index);
	QString getLyricLink(const QString &songId);
	QString getLyric(const QString &url);
	QString getTranslatedLyric(const QString &url);
	QList<SongSearchDetailedInfo> searchSong(const QString &songName, int offset = 0);
	QList<SongSearchDetailedInfo> searchSong_old(const QString &songName, int offset = 0);
	QList<SongSearchDetailedInfo> searchSong_new(const QString &songName, int offset = 0, const QString &type = "1", int limit = 20, bool total = true);

private:
	void getSongId(const int index);

private:
	QString m_songId;
	int m_searchSongNum;

};
