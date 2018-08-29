#pragma once

#include "ApiBase.h"

class LocalApi : public ApiBase{
	Q_OBJECT

public:
	LocalApi(QObject *parent = 0);
	virtual ~LocalApi(){}

	int addSong(const QString &song_name, const QString &song_filePath, const QString &songlistName);
	QString getSongLink(const int index);

private:

};
