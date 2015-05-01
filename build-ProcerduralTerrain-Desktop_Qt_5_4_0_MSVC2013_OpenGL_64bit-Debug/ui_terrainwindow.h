/********************************************************************************
** Form generated from reading UI file 'terrainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TERRAINWINDOW_H
#define UI_TERRAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TerrainWindow
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *TerrainWindow)
    {
        if (TerrainWindow->objectName().isEmpty())
            TerrainWindow->setObjectName(QStringLiteral("TerrainWindow"));
        TerrainWindow->resize(400, 300);
        menuBar = new QMenuBar(TerrainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        TerrainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(TerrainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        TerrainWindow->addToolBar(mainToolBar);
        centralWidget = new QWidget(TerrainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        TerrainWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(TerrainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        TerrainWindow->setStatusBar(statusBar);

        retranslateUi(TerrainWindow);

        QMetaObject::connectSlotsByName(TerrainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *TerrainWindow)
    {
        TerrainWindow->setWindowTitle(QApplication::translate("TerrainWindow", "TerrainWindow", 0));
    } // retranslateUi

};

namespace Ui {
    class TerrainWindow: public Ui_TerrainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TERRAINWINDOW_H
