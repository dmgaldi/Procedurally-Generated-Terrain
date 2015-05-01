/****************************************************************************
**
Main for OpenGL in QT.
Start code for CMSC 427, Spring 2015
Reference: cube & texture example in Qt Creator
author: Zheng Xu, xuzhustc@gmail.com
**
****************************************************************************/

#ifndef QT_NO_OPENGL
#include "terrainwindow.h"
#endif

#include <QApplication>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    app.setApplicationName("Car 101");
#ifndef QT_NO_OPENGL
    TerrainWindow myW;
    myW.show();
#else
    QLabel note("OpenGL Support required");
    note.show();
#endif
    return app.exec();
}
