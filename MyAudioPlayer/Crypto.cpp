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
	Py_Initialize();//使用python之前，要调用Py_Initialize();这个函数进行初始化
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('./')");//这一步很重要，修改Python路径
	PyObject * pModule = NULL;//声明变量
	pModule = PyImport_ImportModule("RSAEncrypt");//这里是要调用的文件名RSAEncrypt.py
	PyObject* pyRet = PyObject_CallMethod(pModule, "rsaEncrypt", "sss", text.toStdString().c_str(), pubKey.toStdString().c_str(), modulus.toStdString().c_str());//给python函数参数赋值
	PyArg_Parse(pyRet, "s", &cipher);//转换返回类型
	Py_Finalize();
#endif // !_DEBUG
	return QString(cipher);
}

QString Crypto::getSecretKey(int size) {
	char *key = "";
#ifndef _DEBUG
	Py_Initialize();//使用python之前，要调用Py_Initialize();这个函数进行初始化
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('./')");//这一步很重要，修改Python路径
	PyObject * pModule = NULL;//声明变量
	pModule = PyImport_ImportModule("RSAEncrypt");//这里是要调用的文件名RSAEncrypt.py
	PyObject* pyRet = PyObject_CallMethod(pModule, "createSecretKey", "i", 16);//给python函数参数赋值
	PyArg_Parse(pyRet, "s", &key);//转换返回类型
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
