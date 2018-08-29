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
	//QSharedMemory mem("AudioPlayer");//以系统exe名称为参数，定义共享内存mem  
	//if (!mem.create(1))//创建共享内存mem，如果该共享内存已存在，则弹出提示对话框，并退出  
	//{
	//	qDebug() << "已运行";
	//	return 0;
	//}
}
