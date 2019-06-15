/********************************************************************************
** Form generated from reading UI file 'Lyric.ui'
**
** Created by: Qt User Interface Compiler version 5.12.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LYRIC_H
#define UI_LYRIC_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LyricWidget
{
public:

    void setupUi(QWidget *LyricWidget)
    {
        if (LyricWidget->objectName().isEmpty())
            LyricWidget->setObjectName(QString::fromUtf8("LyricWidget"));
        LyricWidget->resize(800, 120);

        retranslateUi(LyricWidget);

        QMetaObject::connectSlotsByName(LyricWidget);
    } // setupUi

    void retranslateUi(QWidget *LyricWidget)
    {
        LyricWidget->setWindowTitle(QApplication::translate("LyricWidget", "LyricWidget", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LyricWidget: public Ui_LyricWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LYRIC_H
