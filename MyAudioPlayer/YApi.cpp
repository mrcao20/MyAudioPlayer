#include "YApi.h"
#include <qjsonvalue.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <QJsonArray>
#include <qdebug.h>

#define SEARCH_SONG "http://s.music.163.com/search/get/?version=1&type=1&s=%1&limit=20&offset=%2"
#define SEARCH_SONG_POST "http://music.163.com/api/search/get"
#define SONG_LINK "http://music.163.com/song/media/outer/url?id=%1"
#define LYRIC_LINK "http://music.163.com/api/song/lyric?os=pc&id=%1&lv=-1&kv=-1&tv=-1"

YApi::YApi(QObject *parent)
	: NetworkApiBase(parent)
	, m_searchSongNum(20)
	, m_songId("") {

}

int YApi::addSong(const QString &song_name, const QString &song_id, const QString &songlistName) {
	initDatabase();
	QString table = "t_ymusic";
	m_query.exec(QString("SELECT song_index FROM %1 WHERE song_id = '%2'").arg(table).arg(song_id));
	int index;
	if (m_query.next()) {
		index = m_query.value(0).toInt();
		ApiBase::addSong(index, songlistName);
		closeDatabase();
		return index;
	}
	index = getMaxSongIndex();
	index++;
	ApiBase::addSong(index, song_name, table, songlistName);
	m_query.exec(QString("INSERT INTO %1 VALUES(%2, '%3')").arg(table).arg(index).arg(song_id));
	closeDatabase();
	return index;
}

QList<SongSearchDetailedInfo> YApi::searchSong(const QString &songName, int offset) {
	return searchSong_new(songName, offset, "1", m_searchSongNum);
}

QList<SongSearchDetailedInfo> YApi::searchSong_old(const QString &songName, int offset) {
	QList<SongSearchDetailedInfo> songSearchDetailedInfos;
	QMap<QByteArray, QByteArray> header;
	header["Cookie"] = "appver=1.5.0.75771";
	header["Referer"] = "http://music.163.com/";
	QByteArray json = getNetworkData(QString(SEARCH_SONG).arg(songName).arg(offset), header);
	m_songCount = getJsonValue(json, "result.songCount").toInt();
	QJsonArray jsonArray = getJsonValue(json, "result.songs").toArray();
	for (auto itr = jsonArray.begin(); itr != jsonArray.end(); itr++) {
		QJsonObject jsonObject = itr->toObject();
		SongSearchDetailedInfo songSearchDetailedInfo;
		songSearchDetailedInfo.song_id = QString::number(getJsonValue(jsonObject, "id").toInt());
		songSearchDetailedInfo.song_name = getJsonValue(jsonObject, "name").toString();
		songSearchDetailedInfo.song_artistName = getJsonValue(jsonObject, "artists.name").toString();
		songSearchDetailedInfo.song_albumName = getJsonValue(jsonObject, "album.name").toString();
		songSearchDetailedInfos.append(songSearchDetailedInfo);
		if (songSearchDetailedInfos.size() >= m_searchSongNum)
			break;
	}
	return songSearchDetailedInfos;
}

QList<SongSearchDetailedInfo> YApi::searchSong_new(const QString &songName, int offset, const QString &type, int limit, bool total) {
	QList<SongSearchDetailedInfo> songSearchDetailedInfos;
	QByteArray postData;
	postData.append(QString("s=%1&").arg(songName));
	postData.append(QString("offset=%1&").arg(offset));
	postData.append(QString("type=%1&").arg(type));
	postData.append(QString("limit=%1&").arg(limit));
	postData.append(QString("total=%1").arg(total));
	QByteArray json = getNetworkData(SEARCH_SONG_POST, postData);
	m_songCount = getJsonValue(json, "result.songCount").toInt();
	QJsonArray jsonArray = getJsonValue(json, "result.songs").toArray();
	for (auto itr = jsonArray.begin(); itr != jsonArray.end(); itr++) {
		QJsonObject jsonObject = itr->toObject();
		SongSearchDetailedInfo songSearchDetailedInfo;
		songSearchDetailedInfo.song_id = QString::number(getJsonValue(jsonObject, "id").toInt());
		songSearchDetailedInfo.song_name = getJsonValue(jsonObject, "name").toString();
		songSearchDetailedInfo.song_artistName = getJsonValue(jsonObject, "artists.name").toString();
		songSearchDetailedInfo.song_albumName = getJsonValue(jsonObject, "album.name").toString();
		QJsonArray aliasArray = getJsonValue(jsonObject, "alias").toArray();
		if (!aliasArray.isEmpty())
			songSearchDetailedInfo.song_alias = aliasArray.first().toString();
		songSearchDetailedInfos.append(songSearchDetailedInfo);
		if (songSearchDetailedInfos.size() >= m_searchSongNum)
			break;
	}
	return songSearchDetailedInfos;
}

void YApi::getSongId(const int index) {
	initDatabase();
	m_query.exec(QString("SELECT song_id FROM t_ymusic WHERE song_index = %1").arg(index));
	m_query.next();
	m_songId = m_query.value(0).toString();
	closeDatabase();
}

QString YApi::getSongLink(const int index) {
	getSongId(index);
	return getSongLink(m_songId);
}

QString YApi::getSongLink(const QString &songId) {
	return QString(SONG_LINK).arg(songId);
}

QString YApi::getLyricLink(const int index) {
	if (m_songId.isEmpty())
		getSongId(index);
	return getLyricLink(m_songId);
}

QString YApi::getLyricLink(const QString &songId) {
	return QString(LYRIC_LINK).arg(songId);
}

QString YApi::getLyric(const QString &url) {
	QByteArray json = getNetworkData(url);
	return getJsonValue(json, "lrc.lyric").toString();
}

QString YApi::getTranslatedLyric(const QString &url) {
	QByteArray json = getNetworkData(url);
	return getJsonValue(json, "tlyric.lyric").toString();
}