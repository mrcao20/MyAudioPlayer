#include "TcpClient.h"
#include <qtcpsocket.h>
#include <qmessagebox.h>
#include "Global.h"

struct TcpClientData {
	QTcpSocket *m_tcpClient;
	QString m_ip;
	quint16 m_port;
};

TcpClient::TcpClient(QObject *parent)
	: QObject(parent)
	, d(new TcpClientData)
{

	d->m_ip = "127.0.0.1";
	d->m_port = 8000;
}

void TcpClient::init() {
	//��ʼ��TCP�ͻ���
	d->m_tcpClient = new QTcpSocket(this);   //ʵ����tcpClient
	d->m_tcpClient->abort();                 //ȡ��ԭ������
	connect(d->m_tcpClient, SIGNAL(readyRead()), this, SLOT(readData()));
	connect(d->m_tcpClient, SIGNAL(error(QAbstractSocket::SocketError)), \
		this, SLOT(readError(QAbstractSocket::SocketError)));

	d->m_tcpClient->connectToHost(d->m_ip, d->m_port);
	if (d->m_tcpClient->waitForConnected(1000)) {  // ���ӳɹ������if{}
		d->m_tcpClient->write(QString("MyAudioPlayer Connected").toUtf8().toBase64()); //qt5��ȥ��.toAscii()
	}
	connect(Output::instance(), &Output::message, [this](const QString &msg) {
		d->m_tcpClient->write(msg.toUtf8().toBase64()); //qt5��ȥ��.toAscii()
	});
}

void TcpClient::readData(){
	QByteArray buffer = d->m_tcpClient->readAll();
	
}

void TcpClient::readError(QAbstractSocket::SocketError){
	d->m_tcpClient->disconnectFromHost();
	d->m_tcpClient->deleteLater();
	d->m_tcpClient = NULL;
	QMessageBox msgBox;
	msgBox.setText(tr("failed to connect server because %1").arg(d->m_tcpClient->errorString()));
	msgBox.exec();
}

TcpClient::~TcpClient(){
	if (d->m_tcpClient) {
		d->m_tcpClient->write(QString("MyAudioPlayer Disconnected").toUtf8().toBase64()); //qt5��ȥ��.toAscii()
		d->m_tcpClient->disconnectFromHost();
	}
}
