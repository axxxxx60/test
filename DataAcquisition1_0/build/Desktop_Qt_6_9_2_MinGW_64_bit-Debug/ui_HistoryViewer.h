/********************************************************************************
** Form generated from reading UI file 'HistoryViewer.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HISTORYVIEWER_H
#define UI_HISTORYVIEWER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE

class Ui_HistoryViewer
{
public:

    void setupUi(QDialog *HistoryViewer)
    {
        if (HistoryViewer->objectName().isEmpty())
            HistoryViewer->setObjectName("HistoryViewer");
        HistoryViewer->resize(400, 300);

        retranslateUi(HistoryViewer);

        QMetaObject::connectSlotsByName(HistoryViewer);
    } // setupUi

    void retranslateUi(QDialog *HistoryViewer)
    {
        HistoryViewer->setWindowTitle(QCoreApplication::translate("HistoryViewer", "Dialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class HistoryViewer: public Ui_HistoryViewer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HISTORYVIEWER_H
