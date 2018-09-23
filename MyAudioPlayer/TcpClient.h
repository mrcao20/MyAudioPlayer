#pragma once
#pragma execution_character_set("utf-8")
#include <QObject>
#include <qscopedpointer.h>
#include <qabstractsocket.h>

struct TcpClientData;

class TcpClient : public QObject{
	Q_OBJECT

public:
	explicit TcpClient(QObject *parent = 0);
	~TcpClient();

	void init();

private slots:
	//¿Í»§¶Ë²Ûº¯Êý
	void readData();
	void readError(QAbstractSocket::SocketError);

private:
	QScopedPointer<TcpClientData> d;
};
