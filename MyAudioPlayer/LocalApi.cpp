#include "LocalApi.h"
#include <qvariant.h>

LocalApi::LocalApi(QObject *parent)
	: ApiBase(parent) {

}

int LocalApi::addSong(const QString &song_name, const QString &song_filePath, const QString &songlistName) {
	initDatabase();
	QString table = "t_local";
	m_query.exec(QString("SELECT song_index FROM %1 WHERE song_filePath = '%2'").arg(table).arg(song_filePath));
	int index;
	if (m_query.next()) {
		index = m_query.value(0).toInt();
		ApiBase::addSong(index, songlistName);
		closeDatabase();
		return index;
	}
	index = getMaxSongIndex();
	index++;
	ApiBase::addSong(index, song_name, table, songlistName);
	m_query.exec(QString("INSERT INTO %1 VALUES(%2, '%3')").arg(table).arg(index).arg(song_filePath));
	closeDatabase();
	return index;
}

QString LocalApi::getSongLink(const int index) {
	initDatabase();
	m_query.exec(QString("SELECT song_filePath FROM t_local WHERE song_index = %1").arg(index));
	m_query.next();
	QString songLink = m_query.value(0).toString();
	closeDatabase();
	return songLink;
}