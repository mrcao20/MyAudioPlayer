#include "downloadmanagerwidget.h"

#include "downloadwidget2.h"
#include "downloaditem.h"
#include <qapplication.h>

#include <QFileDialog>
#include <QWebEngineDownloadItem>

struct DownloadManagerWidgetDataPrivate {
	QString m_appPath;
};

DownloadManagerWidget::DownloadManagerWidget(QWidget *parent)
    : QWidget(parent)
    , m_numDownloads(0)
	, d(new DownloadManagerWidgetDataPrivate)
{
    setupUi(this);

#ifdef _DEBUG
	d->m_appPath = qApp->applicationDirPath() + "/../Release";
#else
	d->m_appPath = qApp->applicationDirPath();
#endif // _DEBUG

	QDir dir(d->m_appPath + "/data/serialization");
	if (!dir.exists()) {
		dir.mkpath(d->m_appPath + "/data/serialization");
	}
	QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files);
	for (QFileInfo fileInfo : fileInfoList) {
		DownloadItem *downloadItem = new DownloadItem;
		QString filePath = fileInfo.absoluteFilePath();
		QFile file(filePath);
		file.open(QIODevice::ReadOnly);
		QDataStream in(&file);
		in >> downloadItem;
		file.close();
		QFile::remove(filePath);
		if (!QFile::exists(downloadItem->completePath())) {
			delete downloadItem;
			continue;
		}
		add(new DownloadWidget2(downloadItem));
	}
}

void DownloadManagerWidget::downloadRequested(QWebEngineDownloadItem *download)
{
    Q_ASSERT(download && download->state() == QWebEngineDownloadItem::DownloadRequested);
	
	QString path = "D:/MyDownloads/" + QFileInfo(download->path()).fileName();
	if (QFile::exists(path)) {
		QString tempPath;
		QFileInfo fileInfo(path);
		QString suffix = "." + fileInfo.completeSuffix();
		QString path_left = path.left(path.lastIndexOf(suffix));
		for (int i = 1; ; i++) {
			tempPath = path_left + QString("(%1)").arg(i) + suffix;
			if (!QFile::exists(tempPath))
				break;
		}
		path = tempPath;
	}
    path = QFileDialog::getSaveFileName(this, tr("Save as"), path);
    if (path.isEmpty())
        return;

    //download->setPath(path);
    //download->accept();
	DownloadItem *downloadItem = new DownloadItem(path, download->url());
	download->cancel();
	//delete download;
	downloadItem->accept();
	add(new DownloadWidget2(downloadItem));

    show();
	//delete download;
}

void DownloadManagerWidget::downloadRequested(const QString &fileName, const QString &url) {

	QString dirPath = d->m_appPath + "/data/song/local/";
	QDir dir;
	if (!dir.exists(dirPath)) {
		dir.mkpath(dirPath);
	}
	QString path = dirPath + fileName;
	if (QFile::exists(path)) {
		QString tempPath;
		QFileInfo fileInfo(path);
		QString suffix = "." + fileInfo.completeSuffix();
		QString path_left = path.left(path.lastIndexOf(suffix));
		for (int i = 1; ; i++) {
			tempPath = path_left + QString("(%1)").arg(i) + suffix;
			if (!QFile::exists(tempPath))
				break;
		}
		path = tempPath;
	}
	path = QFileDialog::getSaveFileName(this, tr("Save as"), path);
	if (path.isEmpty())
		return;

	DownloadItem *downloadItem = new DownloadItem(path, url);
	downloadItem->accept();
	add(new DownloadWidget2(downloadItem));

	show();
}

DownloadManagerWidget::~DownloadManagerWidget() {
	QLayoutItem *child;
	while ((child = m_itemsLayout->takeAt(0)) != 0) {
		delete child;
	}
}

void DownloadManagerWidget::add(DownloadWidget2 *downloadWidget)
{
    connect(downloadWidget, &DownloadWidget2::removeClicked, this, &DownloadManagerWidget::remove);
    m_itemsLayout->insertWidget(0, downloadWidget, 0, Qt::AlignTop);
    if (m_numDownloads++ == 0)
        m_zeroItemsLabel->hide();
}

void DownloadManagerWidget::remove(DownloadWidget2 *downloadWidget)
{
    m_itemsLayout->removeWidget(downloadWidget);
    downloadWidget->deleteLater();
    if (--m_numDownloads == 0)
        m_zeroItemsLabel->show();
}
