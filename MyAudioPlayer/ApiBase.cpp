#include "ApiBase.h"
#include <qvariant.h>
#include <qmessagebox.h>

struct ApiBaseDataPrivate{
	bool m_isRunningDBService;
};

ApiBase::ApiBase(QObject *parent)
	: QObject(parent) ,
	d(new ApiBaseDataPrivate) {

}

void ApiBase::init() {
	d->m_isRunningDBService = startSvc("MYSQL57");
	if (!d->m_isRunningDBService) {
		QMessageBox::warning(NULL, "warning", "failed start service!");
	}
}

bool ApiBase::initDatabase() {
	if (!QSqlDatabase::contains("audio_database_connect")) {
		QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "audio_database_connect");
		db.setHostName("localhost");
		db.setPort(3306);
		db.setDatabaseName("d_music");
		db.setUserName("root");
		db.setPassword("839566521");
		while (!db.open()) {
			if (!d->m_isRunningDBService) {
				QSqlDatabase::removeDatabase("audio_database_connect");
				return false;
			}
			Sleep(300);
		}
	}
	m_db = QSqlDatabase::database("audio_database_connect");
	m_query = QSqlQuery(m_db);
	return true;
}

void ApiBase::closeDatabase() {
	m_query.clear();
	m_db.close();
}

QStringList ApiBase::loadSonglist() {
	if (!initDatabase()) {
		return QStringList();
	}
	QStringList songlistList;
	m_query.exec(QString("SELECT songlist_name FROM t_songlistinfo"));
	while (m_query.next()) {
		songlistList.append(m_query.value(0).toString());
	}
	closeDatabase();
	return songlistList;
}

QList<SongBaseInfo> ApiBase::loadSong(const QString &songlistName) {
	if (!initDatabase()) {
		return QList<SongBaseInfo>();
	}
	QList<SongBaseInfo> songBaseInfoList;
	int songlist_id = getSonglistId(songlistName);
	m_query.exec(QString("SELECT t_m.song_index, song_name, src_tabel FROM t_music t_m, t_songlist t_p "\
		"WHERE t_p.songlist_id = %1 and t_m.song_index = t_p.song_index").arg(songlist_id));
	while (m_query.next())
	{
		SongBaseInfo songBaseInfo;
		songBaseInfo.m_songIndex = m_query.value(0).toInt();
		songBaseInfo.m_songName = m_query.value(1).toString();
		songBaseInfo.m_songSrcTabel = m_query.value(2).toString();
		// 读入的顺序为数据库中的顺序，显示的顺序为index大的显示在最上方，index小的显示在最下方
		songBaseInfoList.prepend(songBaseInfo);
	}
	closeDatabase();
	return songBaseInfoList;
}

void ApiBase::addSong(const QString &songlistName, int index) {
	if (!initDatabase()) {
		return;
	}
	addSong(index, songlistName);
	closeDatabase();
}

void ApiBase::addSong(int index, const QString &song_name, const QString &src_tabel, const QString &songlistName) {
	// 为防止歌曲名出现单引号造成语句执行失败，故歌曲名用双引号表示
	m_query.exec(QString(R"(INSERT INTO t_music VALUES(%1, "%2", '%3'))").arg(index).arg(song_name).arg(src_tabel));
	addSong(index, songlistName);
}

void ApiBase::addSong(int index, const QString &songlistName) {
	int songlist_id = getSonglistId(songlistName);
	m_query.exec(QString("INSERT INTO t_songlist VALUES(%1, %2)").arg(songlist_id).arg(index));
}

inline void ApiBase::deleteSong(int index, int songlistId) {
	m_query.exec(QString("DELETE FROM t_songlist WHERE songlist_id = %1 and song_index = %2").arg(songlistId).arg(index));
	m_query.exec(QString("DELETE FROM t_music WHERE song_index = %1").arg(index));
}

QString ApiBase::getNewSonglist() {
	QString songlist = "new songlist";
	if (!initDatabase()) {
		return songlist;
	}
	int i = 0;
	do {
		if (i > 0)
			songlist = "new songlist" + QString::number(i);
		m_query.clear();
		m_query.exec(QString("SELECT * FROM t_songlistinfo WHERE songlist_name = '%1'").arg(songlist));
		i++;
	} while (m_query.next());
	closeDatabase();
	return songlist;
}

void ApiBase::deleteSong(int index, const QString &songlistName) {
	if (!initDatabase()) {
		return;
	}
	int songlist_id = getSonglistId(songlistName);
	deleteSong(index, songlist_id);
	closeDatabase();
}

void ApiBase::deleteSonglist(const QString &songlistName) {
	if (!initDatabase()) {
		return;
	}
	int songlist_id = getSonglistId(songlistName);
	m_query.exec(QString("SELECT song_index FROM t_songlist WHERE songlist_id = %1").arg(songlist_id));
	while (m_query.next()) {
		deleteSong(m_query.value(0).toInt(), songlist_id);
	}
	m_query.exec(QString("DELETE FROM t_songlistinfo WHERE songlist_id = %1").arg(songlist_id));
	m_query.exec("SELECT MAX(songlist_id) FROM t_songlistinfo");
	m_query.next();
	int maxId = m_query.value(0).toInt();
	if (songlist_id > maxId) {
		m_query.exec(QString("ALTER TABLE t_songlistinfo AUTO_INCREMENT = %1").arg(maxId));
	}
	closeDatabase();
}

/*
添加一个歌单，如果数据库不存在此歌单，则插入，并返回true
如果存在，则取消操作，并返回false
*/
bool ApiBase::addSonglist(const QString &songlistName) {
	if (!initDatabase()) {
		return false;
	}
	bool isExists = false;
	m_query.exec(QString("SELECT * FROM t_songlistinfo WHERE songlist_name = '%1'").arg(songlistName));
	isExists = m_query.next();
	if(!isExists)
		m_query.exec(QString("INSERT INTO t_songlistinfo (songlist_name) VALUES('%1')").arg(songlistName));
	closeDatabase();
	return !isExists;
}

bool ApiBase::renameSonglist(const QString &newSonglistName, const QString &songlistName) {
	if (!initDatabase()) {
		return false;
	}
	bool isRename = true;
	m_query.exec(QString("SELECT songlist_id FROM t_songlistinfo WHERE songlist_name='%1'").arg(newSonglistName));
	isRename = !m_query.next();
	if(isRename)
		m_query.exec(QString("update t_songlistinfo set songlist_name='%1' where songlist_name='%2'").arg(newSonglistName).arg(songlistName));
	closeDatabase();
	return isRename;
}

inline int ApiBase::getSonglistId(const QString &songlistName) {
	m_query.exec(QString("SELECT songlist_id FROM t_songlistinfo WHERE songlist_name = '%1'").arg(songlistName));
	m_query.next();
	return m_query.value(0).toInt();
}

int ApiBase::getMaxSongIndex() {
	// 由于存在歌曲的删除，数据库中歌曲的song_index可能不连续，故最大值与歌曲数量可能不等，这里使用最大值获取最后一首歌的song_index
	m_query.exec("SELECT MAX(song_index) FROM t_music");
	m_query.next();
	return m_query.value(0).toInt();
}

bool ApiBase::startSvc(LPSTR szServiceName) {
	SC_HANDLE schService;
	schService = OpenServiceA(OpenSCManagerA(NULL, NULL, GENERIC_READ), szServiceName, SERVICE_ALL_ACCESS);
	if (!schService) {
		printf("mysql service not found '%d'\n", ::GetLastError());
		fflush(stdout);
		return false;
	}
	SERVICE_STATUS svcStatus = { 0 };
	QueryServiceStatus(schService, &svcStatus);
	if (svcStatus.dwCurrentState == SERVICE_RUNNING) {
		CloseServiceHandle(schService);
		return true;
	}
	bool ret = StartServiceA(schService, 0, NULL);
	//WaitServiceState(schService, NULL, 1000, NULL);
	CloseServiceHandle(schService);
	if (!ret) {
		printf("Service start failed(%d)\n", GetLastError());
		fflush(stdout);
		return ret;
	}
	return ret;
}

ApiBase::~ApiBase(){}