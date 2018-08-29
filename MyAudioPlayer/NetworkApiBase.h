#pragma once

#include "ApiBase.h"

QT_BEGIN_NAMESPACE
class QNetworkReply;
QT_END_NAMESPACE

struct SongSearchDetailedInfo {
	QString song_id;
	QString song_name;
	QString song_artistName;
	QString song_albumName;
	QString song_alias = "";
};

struct NetworkApiBaseDataPrivate;

class NetworkApiBase : public ApiBase {
	Q_OBJECT

public:
	NetworkApiBase(QObject *parent = 0);
	virtual ~NetworkApiBase();

	int songCount() { return m_songCount; }
	void downloadSong(const QString &fileName, const QString &songLink);
	virtual QList<SongSearchDetailedInfo> searchSong(const QString &songName, int offset = 0) { return QList<SongSearchDetailedInfo>(); }
	virtual QString getSongLink(const QString &songId) { return ""; }
	virtual QString getLyricLink(const QString &songId) { return ""; }

protected:
	int m_songCount;

protected:
	QByteArray getNetworkData(const QString &, const QMap<QByteArray, QByteArray> & = QMap<QByteArray, QByteArray>());
	QByteArray getNetworkData(const QString &, const QByteArray &postData, const QMap<QByteArray, QByteArray> & = QMap<QByteArray, QByteArray>());
	QJsonValue getJsonValue(const QByteArray &json, const QString &nodeName);
	QJsonValue getJsonValue(const QJsonObject &root, const QString &nodeName);
	void getValue(QJsonObject &root, QJsonValue &nodeValue, QStringList &nodeList);

private:
	QByteArray getNetworkData(QNetworkReply *reply);

private:
	QScopedPointer<NetworkApiBaseDataPrivate> d;

};
