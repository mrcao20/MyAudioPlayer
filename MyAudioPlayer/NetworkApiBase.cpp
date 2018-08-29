#include "NetworkApiBase.h"
#include <qnetworkaccessmanager.h>
#include <qnetworkreply.h>
#include <qnetworkrequest.h>
#include <qeventloop.h>
#include <qfile.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <QJsonArray>
#include <QRegularExpression>
#include <qapplication.h>
#include "downloadmanagerwidget.h"

struct NetworkApiBaseDataPrivate{
	QScopedPointer<DownloadManagerWidget> m_downloadManagerWidget;
};

NetworkApiBase::NetworkApiBase(QObject *parent)
	: ApiBase(parent)
	, d(new NetworkApiBaseDataPrivate)
	, m_songCount(0){

	d->m_downloadManagerWidget.reset(new DownloadManagerWidget);
}

/*
����ָ����nodeName��QByteArray���ҳ���һ������������QJsonValue
*/
QJsonValue NetworkApiBase::getJsonValue(const QByteArray &json, const QString &nodeName) {
	QJsonValue nodeValue;
	QJsonDocument jsonDocument = QJsonDocument::fromJson(json);
	if (jsonDocument.isEmpty()) {
		printf("audio link error, json value not found\n");
		fflush(stdout);
		return nodeValue;
	}
	QJsonObject root = jsonDocument.object();
	nodeValue = getJsonValue(root, nodeName);
	return nodeValue;
}

/*
����nodeName��QJsonObject���ҳ���һ������������QJsonValue
*/
QJsonValue NetworkApiBase::getJsonValue(const QJsonObject &root, const QString &nodeName) {
	QJsonValue nodeValue;
	QStringList nodeList = nodeName.split(QRegularExpression("\\."), QString::SkipEmptyParts);
	QJsonObject jsonObject = root;
	getValue(jsonObject, nodeValue, nodeList);
	return nodeValue;
}

/*
������ָ����nodelist���ص�һ������������nodeValue
*/
void NetworkApiBase::getValue(QJsonObject &root, QJsonValue &nodeValue, QStringList &nodeList) {
	for (int i = 0; i < nodeList.size(); i++) {
		QStringList list = nodeList;
		nodeValue = root.value(list.takeAt(i));   // ��������ڸ�node����nodeValue��typeΪUndefined
		if (nodeValue.type() != QJsonValue::Object && nodeValue.type() != QJsonValue::Array)
			break;
		if (i == nodeList.size() - 1)
			break;
		if (nodeValue.type() == QJsonValue::Object) {
			root = nodeValue.toObject();
			continue;
		}
		QJsonArray array = nodeValue.toArray();
		for (auto itr = array.begin(); itr != array.end(); itr++) {
			root = (*itr).toObject();
			getValue(root, nodeValue, list);
			if (!nodeValue.isUndefined() && !nodeValue.isNull())
				return;
		}
	}
}

void NetworkApiBase::downloadSong(const QString &fileName, const QString &songLink) {
	d->m_downloadManagerWidget->downloadRequested(fileName, songLink);
}

QByteArray NetworkApiBase::getNetworkData(const QString &url, const QMap<QByteArray, QByteArray> &headers) {
	QNetworkAccessManager networkAccessManager;
	QNetworkRequest request;
	request.setUrl(url);
	for (auto itr = headers.cbegin(); itr != headers.cend(); itr++) {
		request.setRawHeader(itr.key(), itr.value());
	}
	QNetworkReply *reply = networkAccessManager.get(request);
	QByteArray data = getNetworkData(reply);
	return data;
}

QByteArray NetworkApiBase::getNetworkData(const QString &url, const QByteArray &postData, const QMap<QByteArray, QByteArray> &headers) {
	QNetworkAccessManager networkAccessManager;
	QNetworkRequest request;
	request.setUrl(url);
	for (auto itr = headers.cbegin(); itr != headers.cend(); itr++) {
		request.setRawHeader(itr.key(), itr.value());
	}
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	request.setHeader(QNetworkRequest::ContentLengthHeader, postData.length());
	QNetworkReply *reply = networkAccessManager.post(request, postData);
	QByteArray data = getNetworkData(reply);
	return data;
}

QByteArray NetworkApiBase::getNetworkData(QNetworkReply *reply) {
	QEventLoop loop;
	connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();
	QByteArray data = reply->readAll();
	reply->close();
	delete reply;
	return data;
}

NetworkApiBase::~NetworkApiBase(){}