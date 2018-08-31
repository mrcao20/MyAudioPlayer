#include "Crypto.h"
#include <filters.h>
#include <modes.h>
#include <Hex.h>      // StreamTransformationFilter
#include <aes.h>
#include <string>

#ifndef _DEBUG
#include <Python.h>
#endif // !_DEBUG

using namespace CryptoPP;

enum AESKeyLength{
	AES_KEY_LENGTH_16 = 16, AES_KEY_LENGTH_24 = 24, AES_KEY_LENGTH_32 = 32
};

Crypto::Crypto(QObject *parent) : 
	QObject(parent){

}

QString Crypto::aesEncrypt(const QString &text, const QString &key, const QString &iv) {
	std::string outData;
	QString inData = PKCS5Padding(text);
	CBC_Mode<AES>::Encryption e;
	e.SetKeyWithIV((byte*)key.toStdString().c_str(), key.size(), (byte *)iv.toStdString().c_str());
	StringSource ss(inData.toStdString(), true,
		new StreamTransformationFilter(e, new StringSink(outData)));

	QByteArray b(outData.c_str(), outData.size());
	QString outStr = b.toBase64(QByteArray::Base64Encoding);
	return outStr;
}

QString Crypto::rsaEncrypt(const QString &text, const QString &pubKey, const QString &modulus) {
	char *cipher = "";
#ifndef _DEBUG
	Py_Initialize();//ʹ��python֮ǰ��Ҫ����Py_Initialize();����������г�ʼ��
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('./')");//��һ������Ҫ���޸�Python·��
	PyObject * pModule = NULL;//��������
	pModule = PyImport_ImportModule("RSAEncrypt");//������Ҫ���õ��ļ���RSAEncrypt.py
	PyObject* pyRet = PyObject_CallMethod(pModule, "rsaEncrypt", "sss", text.toStdString().c_str(), pubKey.toStdString().c_str(), modulus.toStdString().c_str());//��python����������ֵ
	PyArg_Parse(pyRet, "s", &cipher);//ת����������
	Py_Finalize();
#endif // !_DEBUG
	return QString(cipher);
}

QString Crypto::getSecretKey(int size) {
	char *key = "";
#ifndef _DEBUG
	Py_Initialize();//ʹ��python֮ǰ��Ҫ����Py_Initialize();����������г�ʼ��
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('./')");//��һ������Ҫ���޸�Python·��
	PyObject * pModule = NULL;//��������
	pModule = PyImport_ImportModule("RSAEncrypt");//������Ҫ���õ��ļ���RSAEncrypt.py
	PyObject* pyRet = PyObject_CallMethod(pModule, "createSecretKey", "i", 16);//��python����������ֵ
	PyArg_Parse(pyRet, "s", &key);//ת����������
	Py_Finalize();
#endif // !_DEBUG
	return QString(key);
}

QString Crypto::PKCS5Padding(const QString &text) {
	QString result = text;
	int pad = 16 - (text.size() % 16);
	for (int i = 0; i < pad; i++) {
		result += QChar(pad);
	}
	return result;
}

Crypto::~Crypto(){
}
