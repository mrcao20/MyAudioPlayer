#include "YApi.h"
#include <qjsonvalue.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <QJsonArray>
#include "Crypto.h"
#include <QUrl>
#include <qvariant.h>
#include <QTextStream>

#ifdef _DEBUG
#include <qdebug.h>
#endif // _DEBUG


#define SEARCH_SONG "http://s.music.163.com/search/get/?version=1&type=1&s=%1&limit=20&offset=%2"
#define SEARCH_SONG_POST "http://music.163.com/api/search/get"
#define SONG_LINK "http://music.163.com/song/media/outer/url?id=%1"
#define LYRIC_LINK "http://music.163.com/api/song/lyric?os=pc&id=%1&lv=-1&kv=-1&tv=-1"
#define SONG_LINK_GET "https://music.163.com/weapi/song/enhance/player/url"

struct YApiDataPrivate{
	Crypto m_crypto;
	QString m_pubKey = "010001";
	QString m_modulus = "00e0b509f6259df8642dbc35662901477df22677ec152b5ff68ace615bb7b725152b3ab17a876aea8a5aa76d2e417629ec4ee341f56135fccf695280104e0312ecbda92557c93870114af6c9d05c4f7f0c3685b7a46bee255932575cce10b424d813cfe4875d3e82047b97ddef52741d546b8e289dc6935b3ece0462db0a22b8e7";
};

YApi::YApi(QObject *parent)
	: NetworkApiBase(parent)
	, d(new YApiDataPrivate)
	, m_searchSongNum(20)
	, m_songId("") {

}

int YApi::addSong(const QString &song_name, const QString &song_id, const QString &songlistName) {
	if (!initDatabase()) {
		return -1;
	}
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
	QTextStream(stdout) << json;
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
#include <QMessageBox>
QString YApi::getEncryptParams(const QString &text, const QString &secKey) {
	QString iv = "0102030405060708";
	QString params = d->m_crypto.aesEncrypt(text, "0CoJUm6Qyw8W8jud", iv);
	params = d->m_crypto.aesEncrypt(params, secKey, iv);
	params = QUrl::toPercentEncoding(params);
	return params;
}

QString YApi::getEncryptEncSecKey(const QString &text) {
	QString params = d->m_crypto.rsaEncrypt(text, d->m_pubKey, d->m_modulus);
	return params;
}

void YApi::getSongId(const int index) {
	if (!initDatabase()) {
		m_songId = "";
		return;
	}
	m_query.exec(QString("SELECT song_id FROM t_ymusic WHERE song_index = %1").arg(index));
	m_query.next();
	m_songId = m_query.value(0).toString();
	closeDatabase();
}

QString YApi::getDownloadLink(const QString &songId) {
	QString text = QString("{\"ids\":\"[%1]\",\"br\":128000,\"csrf_token\":\"\"}").arg(songId);
	QString secKey;
	QString encSecKey;
#ifdef _DEBUG
	secKey = "FFFFFFFFFFFFFFFF";
	encSecKey = "257348aecb5e556c066de214e531faadd1c55d814f9be95fd06d6bff9f4c7a41f831f6394d5a3fd2e3881736d94a02ca919d952872e7d0a50ebfa1769a7a62d512f5f1ca21aec60bc3819a9c3ffca5eca9a0dba6d6f7249b06f5965ecfff3695b54e1c28f3f624750ed39e7de08fc8493242e26dbc4484a01c76f739e135637c";
#else
	secKey = d->m_crypto.getSecretKey(16);
	encSecKey = getEncryptEncSecKey(secKey);	// 将上述动态生成的第二个秘钥RSA加密作为参数
#endif // _DEBUG
	QString params = getEncryptParams(text, secKey);	// 通过已经固定的第一个秘钥和动态生成的第二个秘钥加密text
	QMap<QByteArray, QByteArray> header;
	header["Referer"] = "https://music.163.com/";
	header["Host"] = "music.163.com";
	header["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.87 Safari/537.36";
	header["cookie"] = "_ntes_nnid=f90ae9cb7692708641bbc58d6aa1719f,1505378239692; _ntes_nuid=f90ae9cb7692708641bbc58d6aa1719f; usertrack=c+xxC1nCJL0bNO4sAyhzAg==; vjuids=20e641b02.160df20dacd.0.00ee9d7e5eea3; vjlast=1515571961.1515571961.30; vinfo_n_f_l_n3=023c1ffb968dafb7.1.1.1514350738841.1514350745255.1515572109538; __guid=94650624.319761818276852900.1520926652843.5322; _ga=GA1.2.1299166061.1506414430; jsessionid-cpta=CNVtHPRGhOEWWbFnS3yb1DvnNnJdL%5C714f4ZiXZdn7hcDVk5P4AhVXNAlhL6LN6wTdpxaBxv1buU56ZlVci4bOnFM1iyS0r4fazhWhjN3JdZS%5Co78P%2FfGGpA%2FzkOWyGK2L%2FSf%5Cj7eay1kgUI8V8rQ1FDLfqxfk3d8HadHk1TkLXr4qJX%3A1526297054246; c98xpt_=30; NETEASE_WDA_UID=1457427762#|#1526296163428; mp_MA-9ADA-91BF1A6C9E06_hubble=%7B%22sessionReferrer%22%3A%20%22https%3A%2F%2Fcampus.163.com%2Fapp%2Fcampus%2Frecruitment4campus%22%2C%22updatedTime%22%3A%201532341816860%2C%22sessionStartTime%22%3A%201532341779709%2C%22sendNumClass%22%3A%20%7B%22allNum%22%3A%203%2C%22errSendNum%22%3A%200%7D%2C%22deviceUdid%22%3A%20%22b4b50ab2-cbf3-4ca5-9a6f-65dc20a7c01e%22%2C%22persistedTime%22%3A%201532341672744%2C%22LASTEVENT%22%3A%20%7B%22eventId%22%3A%20%22da_screen%22%2C%22time%22%3A%201532341816860%7D%2C%22sessionUuid%22%3A%20%2272bb7d24-669a-4cd5-b516-a0cad35a70ce%22%7D; __f_=1533041890126; playerid=87458816; JSESSIONID-WYYY=3RksMK%2FzEnIEGz7t1UMgbk%2FaJ8zfM0y8MU5smtCNhw1qQEQCK9u9gdJ4Q3Rwgm5y8UcqVfNzYX%5CIUuYzHwXkqfVQSGNjrZGym9Sj2doVn3M%2B%5Cu93NSn9KMclJ5MvHk%2BVVPr%5C3MgtIMxVmDR%2B%5CT7mktxTA8Fs7xj8yEbgh8csMoFdBSd0%3A1535509954005; _iuqxldmzr_=32; WM_NI=yi5ZKr631w%2FGz%2FTfWOYQxrWu5eZNPVPBUoGQBLewqBG5Cg%2FyHeh9XorlOzSb2TJG%2FXq%2BJQSLiJHFTfjyqFgZZoNv6hHEFYU%2BM3t6ozEQhyw8F8c6eeMGMEtHW7ckvrGaQ28%3D; WM_NIKE=9ca17ae2e6ffcda170e2e6ee95c65ab89c86abb44af187978bc44293bff7abcc6ab4efb890b36a94b681d4d52af0fea7c3b92abbb7fbbbca7af6edff98ae4dbaf19c87d6459b8fb893d67ff7bb96afd35e949e9b90c65b8294ab98f44a85eb8d88ae738cbba8dadb65b2b4819bd26af28c86a8ae54b3f586b2d248ab9688a7d76eaaa7bfa7e6529ab18297b5648c9b8abad8218fedbcd7ee72878fa89af63486e8add7e86f8f9983a6f74495e9ad8cf57a85bf9fd3bb37e2a3; WM_TID=hNFPe0KZYLEdcwbOXhJOFJm9drjhBQ7P; MUSIC_U=294c416fff82565ad632d4f8a67c6053b57ded44bba3973b5fd6c9a693e47173b90ac4896782651e16835d5f24772e273c9f63c2381ce2d9c3061cd18d77b7a0; __remember_me=true; __csrf=86c53a2474543be2df153b4c558e3bf2; __utma=94650624.1090234970.1505378241.1535502934.1535507106.42; __utmb=94650624.27.10.1535507106; __utmc=94650624; __utmz=94650624.1535502934.41.39.utmcsr=baidu|utmccn=(organic)|utmcmd=organic; monitor_count=12";
	QByteArray post_data;
	post_data.append(QString("params=%1&").arg(params));
	post_data.append(QString("encSecKey=%1").arg(encSecKey));
	QByteArray json = getNetworkData(SONG_LINK_GET, post_data, header);
	QString songLink = getJsonValue(json, "data.url").toString();
	return songLink;
}

QString YApi::getSongLink(const int index) {
	getSongId(index);
	return getSongLink(m_songId);
}

QString YApi::getSongLink(const QString &songId) {
	/*
		原音乐源地址仍然可用，但无法下载
		现适应网易云音乐的加密传输，通过加密算法重新获取音乐源，已可下载
		原地址被淘汰，但仍可用，继续保留
		2018-8-31: 重新启用原歌曲api，新加密api单独被用作下载链接
	*/
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

YApi::~YApi(){}