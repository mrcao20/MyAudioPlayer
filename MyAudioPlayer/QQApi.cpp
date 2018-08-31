#include "QQApi.h"
#include <qtextstream.h>
#include <qtextstream.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <QJsonArray>
#include <qfile.h>
#include <QRegularExpression>
#include <qapplication.h>
#include <qdebug.h>

#define PLAY_LIST "https://c.y.qq.com/qzone/fcg-bin/fcg_ucc_getcdinfo_byids_cp.fcg?type=1&json=1&utf8=1&onlysong=0&disstid=%1&format=json&g_tk=2122464954&loginUin=839566521&hostUin=0&format=json&inCharset=utf8&outCharset=utf-8&notice=0&platform=yqq&needNewCode=0"
#define VKEY "https://c.y.qq.com/base/fcgi-bin/fcg_music_express_mobile3.fcg?g_tk=1109981464&loginUin=839566521&hostUin=0&format=json&inCharset=utf8&outCharset=utf-8&notice=0&platform=yqq&needNewCode=0&cid=205361747&uin=839566521&songmid=%1&filename=%2.m4a&guid=2054016189"
#define SONG "http://dl.stream.qqmusic.qq.com/%1.m4a?vkey=%2&guid=2054016189&uin=839566521&fromtag=66"
#define LYRIC "https://c.y.qq.com/lyric/fcgi-bin/fcg_query_lyric_new.fcg?callback=MusicJsonCallback_lrc&pcachetime=1526475335158&songmid=%1&g_tk=202117865&jsonpCallback=MusicJsonCallback_lrc&loginUin=839566521&hostUin=0&format=jsonp&inCharset=utf8&outCharset=utf-8&notice=0&platform=yqq&needNewCode=0"
#define SEARCH "http://s.music.qq.com/fcgi-bin/music_search_new_platform?t=0&n=%1&aggr=1&cr=1&loginUin=0&format=json&inCharset=GB2312&outCharset=utf-8&notice=0&platform=jqminiframe.json&needNewCode=0&p=%2&catZhida=0&remoteplace=sizer.newclient.next_song&w=%3"

QQApi::QQApi(QObject *parent)
	: NetworkApiBase(parent)
	, m_searchSongNum(20)
	, m_songMid(""){

}

int QQApi::addSong(const QString &song_name, const QString &song_mid, const QString &songlistName) {
	initDatabase();
	QString table = "t_qqmusic";
	m_query.exec(QString("SELECT song_index FROM %1 WHERE song_mid = '%2'").arg(table).arg(song_mid));
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
	m_query.exec(QString("INSERT INTO %1 VALUES(%2, '%3')").arg(table).arg(index).arg(song_mid));
	closeDatabase();
	return index;
}

QList<SongSearchDetailedInfo> QQApi::searchSong(const QString &songName, int offset) {
	QList<SongSearchDetailedInfo> songSearchDetailedInfos;
	int page = offset / m_searchSongNum + 1;
	QByteArray json = getNetworkData(QString(SEARCH).arg(m_searchSongNum).arg(page).arg(songName));
	m_songCount = getJsonValue(json, "data.song.curnum").toInt() - 1;
	QJsonArray jsonArray = getJsonValue(json, "data.song.list").toArray();
	for (auto itr = jsonArray.begin(); itr != jsonArray.end(); itr++) {
		QJsonObject jsonObject = itr->toObject();
		SongSearchDetailedInfo songSearchDetailedInfo;
		QString f = getJsonValue(jsonObject, "f").toString();
		QStringList fList = f.split(QRegularExpression("\\|"));
		if (fList.size() < 25)
			continue;
		songSearchDetailedInfo.song_id = fList.at(20);
		songSearchDetailedInfo.song_name = getJsonValue(jsonObject, "fsong").toString();
		songSearchDetailedInfo.song_artistName = getJsonValue(jsonObject, "fsinger").toString();
		songSearchDetailedInfo.song_albumName = getJsonValue(jsonObject, "albumName_hilight").toString();
		songSearchDetailedInfos.append(songSearchDetailedInfo);
		if (songSearchDetailedInfos.size() >= m_searchSongNum)
			break;
	}
	return songSearchDetailedInfos;
}

inline void QQApi::getSongMid(const int index) {
	initDatabase();
	m_query.exec(QString("SELECT song_mid FROM t_qqmusic WHERE song_index = %1").arg(index));
	m_query.next();
	m_songMid = m_query.value(0).toString();
	closeDatabase();
}

QString QQApi::getDownloadLink(const QString &songMid) {
	return getSongLink(songMid);
}

QString QQApi::getSongLink(const int index) {
	getSongMid(index);
	return getSongLink(m_songMid);
}

QString QQApi::getSongLink(const QString &songMid) {
	QString fileName = "C400" + songMid;
	QString vkey = QString(VKEY).arg(songMid).arg(fileName);
	QByteArray json = getNetworkData(vkey);
	QString key = getJsonValue(json, "data.items.vkey").toString();
	return QString(SONG).arg(fileName).arg(key);
}

QString QQApi::getLyricLink(const int index) {
	if (m_songMid.isEmpty())
		getSongMid(index);
	return getLyricLink(m_songMid);
}

QString QQApi::getLyricLink(const QString &songMid) {
	return QString(LYRIC).arg(songMid);
}

QByteArray QQApi::getLyricJson(const QString &url) {
	QMap<QByteArray, QByteArray> headers;
	headers["user-agent"] = "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.87 Safari/537.36";
	headers["referer"] = "https://y.qq.com/portal/player.html";
	headers["cookie"] = "eas_sid=61p5Z0l2y8i4W8h8D4r7y803u0; pgv_pvi=4378796032; RK=cM1a+vMvSK; LW_sid=p1s5e0G983J7w4W3W7L588h8r1; LW_uid=O1k5h0T9B3h7i423e7G5z878f3; pac_uid=1_839566521; tvfe_boss_uuid=58f06ad94fdb153e; __guid=140997088.274668621649698000.1526195014088.8257; ptui_loginuin=839566521; ptcz=e775b76914a71cc7890fa080accbe5f058790ef61c74d41cbe8c317a35344b4a; luin=o0839566521; lskey=000100004738b1f0caf3492e5d09d239d8a8865af9aac1fac4cf85be6db271309424bd8f985272aeb6bbb92c; pt2gguin=o0839566521; p_luin=o0839566521; p_lskey=0004000099c9563a7a30ea414785a82f0821601f69523745768bf0c4050d64035c7c830e1e3e0e747dbebc99; ts_refer=www.baidu.com/link; o_cookie=839566521; monitor_count=1; pgv_si=s588147712; yq_playschange=0; yq_playdata=; qqmusic_fromtag=66; yqq_stat=0; yplayer_open=1; yq_index=1; pgv_info=ssid=s8283094928; ts_last=y.qq.com/portal/player.html; pgv_pvid=2054016189; ts_uid=4636148028; player_exist=1";
	QByteArray json = getNetworkData(url, headers);
	json.remove(0, QString("MusicJsonCallback_lrc(").size());
	json.remove(json.size() - 1, 1);
	return json;
}

QString QQApi::getLyric(const QString &url) {
	QByteArray json = getLyricJson(url);
	return QByteArray::fromBase64(getJsonValue(json, "lyric").toString().toUtf8());
}

QString QQApi::getTranslatedLyric(const QString &url) {
	QByteArray json = getLyricJson(url);
	return QByteArray::fromBase64(getJsonValue(json, "trans").toString().toUtf8());
}

//void QQApi::getSongMid() {
//	QMap<QByteArray, QByteArray> headers;
//	headers["user-agent"] = "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.87 Safari/537.36";
//	headers["referer"] = "https://y.qq.com/portal/profile.html";
//	QByteArray json = getNetworkData(QString(PLAY_LIST).arg(3640119869), headers);
//	QJsonArray array = getJsonValue(json, "cdlist.songlist").toArray();
//	int i = 0;
//	for (auto itr = array.begin(); itr != array.end(); itr++, i++) {
//		SongInfo songInfo;
//		songInfo.m_index = i;           // 获取index
//		QJsonObject node3 = (*itr).toObject();
//		QString songname = node3.value("songname").toString();
//		songInfo.m_song_name = songname;      // 获取song name
//		auto itr1 = node3.find("songurl");
//		QString url = itr1.value().toString();
//		if (!url.isEmpty()) {
//			if (url.endsWith("\\"))
//				url.remove(url.size() - 1, 1);
//			songInfo.m_song_src = url;          // 获取url
//			songInfo.m_song_mid = "";			// 当存在url时，mid置空
//			m_songInfo.insert(i, songInfo);
//			continue;
//		}
//		QString songmid = node3.value("songmid").toString();
//		songInfo.m_song_mid = "songmid:" + songmid;     // 获取song mid
//		songInfo.m_song_src = "";			// 当不存在url时，置空
//		m_songInfo.insert(i, songInfo);
//	}
//}
