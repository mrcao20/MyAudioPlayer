#pragma once

#include <QObject>

class Crypto : public QObject{
	Q_OBJECT

public:
	explicit Crypto(QObject *parent = 0);
	~Crypto();

	QString aesEncrypt(const QString &text, const QString &key, const QString &iv);
	QString rsaEncrypt(const QString &text, const QString &pubKey, const QString &modulus);
	QString getSecretKey(int size);

private:
	QString PKCS5Padding(const QString &text);

};
