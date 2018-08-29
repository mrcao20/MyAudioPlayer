/********************************************************************************
** Form generated from reading UI file 'AudioPlayer.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AUDIOPLAYER_H
#define UI_AUDIOPLAYER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AudioPlayer
{
public:
    QLabel *m_positionLabel;
    QLabel *m_infoLabel;
    QSlider *m_positionSlider;
    QListWidget *m_songListWidget;
    QPushButton *m_previousButton;
    QPushButton *m_playButton;
    QPushButton *m_nextButton;
    QPushButton *m_lyricButton;
    QPushButton *m_playOrderButton;
    QPushButton *m_volumeButton;
    QLineEdit *m_searchLineEdit;
    QSlider *m_volumeSlider;
    QPushButton *m_networkSearchButton;
    QPushButton *m_localSongButton;
    QLineEdit *m_networkSearchEdit;
    QPushButton *m_networkSearchBackButton;
    QPushButton *m_searchBackButton;
    QPushButton *m_searchNextButton;
    QComboBox *m_searchSrcBox;
    QListWidget *m_songlistListWidget;
    QPushButton *m_addSonglistButton;
    QPushButton *m_songlistButton;
    QLabel *m_currentSonglistLabel;
    QPushButton *m_lyricTranslatedButton;

    void setupUi(QWidget *AudioPlayer)
    {
        if (AudioPlayer->objectName().isEmpty())
            AudioPlayer->setObjectName(QStringLiteral("AudioPlayer"));
        AudioPlayer->resize(723, 544);
        AudioPlayer->setContextMenuPolicy(Qt::CustomContextMenu);
        QIcon icon;
        icon.addFile(QStringLiteral(":/MyAudioPlayer/data/icon/format.ico"), QSize(), QIcon::Normal, QIcon::Off);
        AudioPlayer->setWindowIcon(icon);
        m_positionLabel = new QLabel(AudioPlayer);
        m_positionLabel->setObjectName(QStringLiteral("m_positionLabel"));
        m_positionLabel->setGeometry(QRect(478, 479, 101, 21));
        QFont font;
        font.setPointSize(10);
        m_positionLabel->setFont(font);
        m_positionLabel->setStyleSheet(QStringLiteral("color:rgb(97, 97, 97);"));
        m_positionLabel->setAlignment(Qt::AlignCenter);
        m_infoLabel = new QLabel(AudioPlayer);
        m_infoLabel->setObjectName(QStringLiteral("m_infoLabel"));
        m_infoLabel->setGeometry(QRect(165, 479, 321, 21));
        m_infoLabel->setStyleSheet(QStringLiteral("color:rgb(120, 120, 120);"));
        m_infoLabel->setAlignment(Qt::AlignCenter);
        m_positionSlider = new QSlider(AudioPlayer);
        m_positionSlider->setObjectName(QStringLiteral("m_positionSlider"));
        m_positionSlider->setGeometry(QRect(160, 500, 421, 22));
        m_positionSlider->setOrientation(Qt::Horizontal);
        m_songListWidget = new QListWidget(AudioPlayer);
        m_songListWidget->setObjectName(QStringLiteral("m_songListWidget"));
        m_songListWidget->setGeometry(QRect(100, 70, 601, 371));
        m_songListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
        m_songListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_songListWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_songListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_previousButton = new QPushButton(AudioPlayer);
        m_previousButton->setObjectName(QStringLiteral("m_previousButton"));
        m_previousButton->setGeometry(QRect(36, 481, 25, 25));
        m_previousButton->setStyleSheet(QStringLiteral("background-color:rgba(255, 255, 255, 0);"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/MyAudioPlayer/data/icon/previous.png"), QSize(), QIcon::Normal, QIcon::Off);
        m_previousButton->setIcon(icon1);
        m_previousButton->setIconSize(QSize(25, 25));
        m_playButton = new QPushButton(AudioPlayer);
        m_playButton->setObjectName(QStringLiteral("m_playButton"));
        m_playButton->setGeometry(QRect(69, 477, 32, 32));
        m_playButton->setStyleSheet(QStringLiteral("background-color:rgba(255, 255, 255, 0);"));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/MyAudioPlayer/data/icon/play.png"), QSize(), QIcon::Normal, QIcon::Off);
        m_playButton->setIcon(icon2);
        m_playButton->setIconSize(QSize(32, 32));
        m_nextButton = new QPushButton(AudioPlayer);
        m_nextButton->setObjectName(QStringLiteral("m_nextButton"));
        m_nextButton->setGeometry(QRect(109, 481, 25, 25));
        m_nextButton->setStyleSheet(QStringLiteral("background-color:rgba(255, 255, 255, 0);"));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/MyAudioPlayer/data/icon/next.png"), QSize(), QIcon::Normal, QIcon::Off);
        m_nextButton->setIcon(icon3);
        m_nextButton->setIconSize(QSize(25, 25));
        m_lyricButton = new QPushButton(AudioPlayer);
        m_lyricButton->setObjectName(QStringLiteral("m_lyricButton"));
        m_lyricButton->setGeometry(QRect(602, 481, 21, 21));
        QFont font1;
        font1.setPointSize(12);
        m_lyricButton->setFont(font1);
        m_lyricButton->setStyleSheet(QStringLiteral("QPushButton#m_lyricButton:!hover{ background-color:rgba(255, 255, 255, 0); color:rgb(97, 97, 97); }QPushButton#m_lyricButton:hover{ background-color:rgba(255, 255, 255, 0); color:rgb(150, 150, 150); }"));
        m_playOrderButton = new QPushButton(AudioPlayer);
        m_playOrderButton->setObjectName(QStringLiteral("m_playOrderButton"));
        m_playOrderButton->setGeometry(QRect(622, 486, 25, 25));
        m_playOrderButton->setStyleSheet(QStringLiteral("background-color:rgba(255, 255, 255, 0);"));
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/MyAudioPlayer/data/icon/shuffle.png"), QSize(), QIcon::Normal, QIcon::Off);
        m_playOrderButton->setIcon(icon4);
        m_playOrderButton->setIconSize(QSize(23, 23));
        m_volumeButton = new QPushButton(AudioPlayer);
        m_volumeButton->setObjectName(QStringLiteral("m_volumeButton"));
        m_volumeButton->setGeometry(QRect(656, 485, 24, 24));
        m_volumeButton->setStyleSheet(QStringLiteral("background-color:rgba(255, 255, 255, 0);"));
        QIcon icon5;
        icon5.addFile(QStringLiteral(":/MyAudioPlayer/data/icon/volume.png"), QSize(), QIcon::Normal, QIcon::Off);
        m_volumeButton->setIcon(icon5);
        m_volumeButton->setIconSize(QSize(24, 24));
        m_searchLineEdit = new QLineEdit(AudioPlayer);
        m_searchLineEdit->setObjectName(QStringLiteral("m_searchLineEdit"));
        m_searchLineEdit->setGeometry(QRect(547, 30, 151, 20));
        m_searchLineEdit->setClearButtonEnabled(true);
        m_volumeSlider = new QSlider(AudioPlayer);
        m_volumeSlider->setObjectName(QStringLiteral("m_volumeSlider"));
        m_volumeSlider->setGeometry(QRect(657, 310, 22, 160));
        m_volumeSlider->setValue(85);
        m_volumeSlider->setOrientation(Qt::Vertical);
        m_networkSearchButton = new QPushButton(AudioPlayer);
        m_networkSearchButton->setObjectName(QStringLiteral("m_networkSearchButton"));
        m_networkSearchButton->setGeometry(QRect(130, 15, 16, 16));
        m_networkSearchButton->setStyleSheet(QStringLiteral("background-color:rgba(255, 255, 255, 0);"));
        QIcon icon6;
        icon6.addFile(QStringLiteral(":/MyAudioPlayer/data/icon/search.png"), QSize(), QIcon::Normal, QIcon::Off);
        m_networkSearchButton->setIcon(icon6);
        m_localSongButton = new QPushButton(AudioPlayer);
        m_localSongButton->setObjectName(QStringLiteral("m_localSongButton"));
        m_localSongButton->setGeometry(QRect(7, 4, 25, 25));
        m_localSongButton->setStyleSheet(QStringLiteral("background-color:rgba(255, 255, 255, 0);"));
        QIcon icon7;
        icon7.addFile(QStringLiteral(":/MyAudioPlayer/data/icon/localSong.png"), QSize(), QIcon::Normal, QIcon::Off);
        m_localSongButton->setIcon(icon7);
        m_localSongButton->setIconSize(QSize(25, 25));
        m_networkSearchEdit = new QLineEdit(AudioPlayer);
        m_networkSearchEdit->setObjectName(QStringLiteral("m_networkSearchEdit"));
        m_networkSearchEdit->setGeometry(QRect(130, 13, 151, 20));
        m_networkSearchEdit->setClearButtonEnabled(true);
        m_networkSearchBackButton = new QPushButton(AudioPlayer);
        m_networkSearchBackButton->setObjectName(QStringLiteral("m_networkSearchBackButton"));
        m_networkSearchBackButton->setGeometry(QRect(96, 10, 31, 23));
        m_networkSearchBackButton->setStyleSheet(QStringLiteral("QPushButton#m_networkSearchBackButton:!hover{ background-color:rgba(255, 255, 255, 0); color:rgb(97, 97, 97); }QPushButton#m_networkSearchBackButton:hover{ background-color:rgba(255, 255, 255, 0); color:rgb(150, 150, 150); }"));
        m_searchBackButton = new QPushButton(AudioPlayer);
        m_searchBackButton->setObjectName(QStringLiteral("m_searchBackButton"));
        m_searchBackButton->setGeometry(QRect(98, 40, 31, 23));
        m_searchBackButton->setFont(font);
        m_searchBackButton->setStyleSheet(QStringLiteral("QPushButton#m_searchBackButton:!hover{ background-color:rgba(255, 255, 255, 0); color:rgb(97, 97, 97); }QPushButton#m_searchBackButton:hover{ background-color:rgba(255, 255, 255, 0); color:rgb(150, 150, 150); }"));
        m_searchNextButton = new QPushButton(AudioPlayer);
        m_searchNextButton->setObjectName(QStringLiteral("m_searchNextButton"));
        m_searchNextButton->setGeometry(QRect(134, 40, 31, 23));
        m_searchNextButton->setFont(font);
        m_searchNextButton->setStyleSheet(QStringLiteral("QPushButton#m_searchNextButton:!hover{ background-color:rgba(255, 255, 255, 0); color:rgb(97, 97, 97); }QPushButton#m_searchNextButton:hover{ background-color:rgba(255, 255, 255, 0); color:rgb(150, 150, 150); }"));
        m_searchSrcBox = new QComboBox(AudioPlayer);
        m_searchSrcBox->setObjectName(QStringLiteral("m_searchSrcBox"));
        m_searchSrcBox->setGeometry(QRect(290, 12, 61, 22));
        m_songlistListWidget = new QListWidget(AudioPlayer);
        m_songlistListWidget->setObjectName(QStringLiteral("m_songlistListWidget"));
        m_songlistListWidget->setGeometry(QRect(4, 95, 91, 192));
        m_songlistListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
        m_songlistListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_songlistListWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_addSonglistButton = new QPushButton(AudioPlayer);
        m_addSonglistButton->setObjectName(QStringLiteral("m_addSonglistButton"));
        m_addSonglistButton->setGeometry(QRect(60, 68, 21, 23));
        QFont font2;
        font2.setPointSize(17);
        m_addSonglistButton->setFont(font2);
        m_addSonglistButton->setStyleSheet(QStringLiteral("QPushButton#m_addSonglistButton:!hover{ background-color:rgba(255, 255, 255, 0); color:rgb(97, 97, 97); }QPushButton#m_addSonglistButton:hover{ background-color:rgba(255, 255, 255, 0); color:rgb(150, 150, 150); }"));
        m_songlistButton = new QPushButton(AudioPlayer);
        m_songlistButton->setObjectName(QStringLiteral("m_songlistButton"));
        m_songlistButton->setGeometry(QRect(4, 68, 61, 23));
        m_songlistButton->setStyleSheet(QLatin1String("QPushButton#m_songlistButton:!hover{ background-color:rgba(255, 255, 255, 0); color:rgb(97, 97, 97); }\n"
"QPushButton#m_songlistButton:hover{ background-color:rgba(255, 255, 255, 0); color:rgb(150, 150, 150); }"));
        m_currentSonglistLabel = new QLabel(AudioPlayer);
        m_currentSonglistLabel->setObjectName(QStringLiteral("m_currentSonglistLabel"));
        m_currentSonglistLabel->setGeometry(QRect(290, 50, 131, 16));
        m_currentSonglistLabel->setFont(font);
        m_currentSonglistLabel->setStyleSheet(QStringLiteral("color:rgb(200, 200, 200);"));
        m_currentSonglistLabel->setAlignment(Qt::AlignCenter);
        m_lyricTranslatedButton = new QPushButton(AudioPlayer);
        m_lyricTranslatedButton->setObjectName(QStringLiteral("m_lyricTranslatedButton"));
        m_lyricTranslatedButton->setGeometry(QRect(602, 499, 21, 21));
        m_lyricTranslatedButton->setFont(font1);
        m_lyricTranslatedButton->setStyleSheet(QStringLiteral("QPushButton#m_lyricTranslatedButton:!hover{ background-color:rgba(255, 255, 255, 0); color:rgb(97, 97, 97); }QPushButton#m_lyricTranslatedButton:hover{ background-color:rgba(255, 255, 255, 0); color:rgb(150, 150, 150); }"));

        retranslateUi(AudioPlayer);

        QMetaObject::connectSlotsByName(AudioPlayer);
    } // setupUi

    void retranslateUi(QWidget *AudioPlayer)
    {
        AudioPlayer->setWindowTitle(QApplication::translate("AudioPlayer", "AudioPlayer", nullptr));
        m_positionLabel->setText(QString());
        m_infoLabel->setText(QString());
        m_previousButton->setText(QString());
#ifndef QT_NO_ACCESSIBILITY
        m_playButton->setAccessibleName(QApplication::translate("AudioPlayer", "play", nullptr));
#endif // QT_NO_ACCESSIBILITY
        m_playButton->setText(QString());
        m_nextButton->setText(QString());
        m_lyricButton->setText(QApplication::translate("AudioPlayer", "\350\257\215", nullptr));
#ifndef QT_NO_ACCESSIBILITY
        m_playOrderButton->setAccessibleName(QApplication::translate("AudioPlayer", "shuffle", nullptr));
#endif // QT_NO_ACCESSIBILITY
        m_playOrderButton->setText(QString());
        m_volumeButton->setText(QString());
        m_networkSearchButton->setText(QString());
        m_localSongButton->setText(QString());
        m_networkSearchBackButton->setText(QApplication::translate("AudioPlayer", "\350\277\224\345\233\236", nullptr));
        m_searchBackButton->setText(QApplication::translate("AudioPlayer", "back", nullptr));
        m_searchNextButton->setText(QApplication::translate("AudioPlayer", "next", nullptr));
        m_addSonglistButton->setText(QApplication::translate("AudioPlayer", "+", nullptr));
        m_songlistButton->setText(QApplication::translate("AudioPlayer", "\346\210\221\347\232\204\346\255\214\345\215\225", nullptr));
        m_currentSonglistLabel->setText(QString());
        m_lyricTranslatedButton->setText(QApplication::translate("AudioPlayer", "\350\257\221", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AudioPlayer: public Ui_AudioPlayer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUDIOPLAYER_H
