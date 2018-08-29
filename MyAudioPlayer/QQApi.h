#pragma once

#include "NetworkApiBase.h"
#include <qlist.h>

class QQApi : public NetworkApiBase
{
	Q_OBJECT
public:
	QQApi(QObject * = 0);
	virtual ~QQApi(){}

	QString getSongLink(const int index);
	QString getSongLink(const QString &songMid);
	QString getLyricLink(const int index);
	QString getLyricLink(const QString &songMid);
	QString getLyric(const QString &url);
	QString getTranslatedLyric(const QString &url);
	int addSong(const QString &song_name, const QString &song_mid, const QString &songlistName);
	QList<SongSearchDetailedInfo> searchSong(const QString &songName, int offset = 0);

private:
	void getSongMid(const int index);
	QByteArray getLyricJson(const QString &url);

private:
	QString m_songMid;
	int m_searchSongNum;

};
