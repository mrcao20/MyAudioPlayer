#include "MyAudioPlayer.h"
#include "SingleApplication.h"

QString commandLineFilePathArgument() {
	const QStringList args = QCoreApplication::arguments();
	for (const QString &arg : args.mid(1)) {
		if (!arg.startsWith(QLatin1Char('-')))
			return arg;
	}
	return "";
}

int main(int argc, char *argv[]){
	SingleApplication a(argc, argv);
	if (a.isRunning())
		return 0;
	MyAudioPlayer w;
	a.w = &w;
	QString filePath = commandLineFilePathArgument();
	if (!filePath.isEmpty())
		w.playFile(filePath);
	w.show();
	return a.exec();
	//QSharedMemory mem("AudioPlayer");//��ϵͳexe����Ϊ���������干���ڴ�mem  
	//if (!mem.create(1))//���������ڴ�mem������ù����ڴ��Ѵ��ڣ��򵯳���ʾ�Ի��򣬲��˳�  
	//{
	//	qDebug() << "������";
	//	return 0;
	//}
}
