/****************************************************************************
**
Window for OpenGL in QT.
Start code for CMSC 427, Spring 2015
Reference: cube & texture example in Qt Creator
author: Zheng Xu, xuzhustc@gmail.com
**
****************************************************************************/

#ifndef TERRAINWINDOW_H
#define TERRAINWINDOW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QKeyEvent>
#include <QtGui>
#include <QTimer>
#include <time.h>

#include <QMatrix4x4>
#include <QVector4D>

class TerrainWindow : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit TerrainWindow(QWidget *parent = 0);
    ~TerrainWindow();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

    void initShaders();
    void initMat();
    void keyPressEvent(QKeyEvent *) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;

private:
    void loadCubes();
    void dsFractal(float a, float b, float c, float d, float rough);
    void moveCube(const int cords[6][4][3], float (&nCds)[6][4][3], float x, float y, float z, float scale);
    void addCube(QVector<GLfloat> &vertData, float coords[6][4][3], float red, float green, float blue, float alpha);

    void smoothTerrain();
    void calculateNormals();
    QVector3D *getVertexNormal(int i, int j);

    void addHeightMap(float **hmap);
    void addHeightMapVertex(QVector<GLfloat> &vertData, QVector3D &position, QVector3D &normal, QVector4D &color);
    void rotateCamera(float degrees, float x, float y, float z);
    void moveCameraForward(float amount);
    QVector4D getColor(float height, QVector3D color1, QVector3D color2, QVector3D color3);


    /* Private Member variables */
    float **hmap;
    float minCoord, maxCoord;
    float minHeight, maxHeight;
    QColor clearColor;
    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;
    QOpenGLTexture *textures[6];
    QString txtPath;
    unsigned int meshSize;
    QVector3D **normals;

    /* Collision Detection variables:
     * cube x is represented by corners cubeMinPoints.at(x), cubeMaxPoints.at(x)*/
    std::vector<QVector3D*> cubeMinPoints;
    std::vector<QVector3D*> cubeMaxPoints;
    QVector3D *position;
    QVector3D lightDirection;
    QVector3D lightIntensity;


    /* Camera control variables */
    QMatrix4x4 mvpMat;
    QMatrix4x4 mvpMat0;
    QTimer *automoveTimer;
    QTimer *somersaultTimer;
    int automoveInterval;
    float turningSpeed;
    float movementSpeed;
    float horizontalAngle;
    float verticalAngle;


private slots:
    void automove();
    void somersault();

#define RESOURCE_FLAG true
#define TXT_IMG_PATH "C:/Users/Zheng/Documents/openglTest/images"
#define CUBE_NUM 6
#define PI 3.1415926535897932384626433832795
};

#endif  //MYWIDGET_h
