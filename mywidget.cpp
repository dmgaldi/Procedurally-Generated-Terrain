/****************************************************************************
**
Window for OpenGL in QT.
Start code for CMSC 427, Spring 2015
Reference: cube & texture example in Qt Creator
author: Zheng Xu, xuzhustc@gmail.com
**
****************************************************************************/

#include "mywidget.h"

MyWidget::MyWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    memset(textures, 0, sizeof(textures));
    txtPath = TXT_IMG_PATH;
    cubNum = CUBE_NUM;
    horizontalAngle = 90.0f;
    verticalAngle = 0.0f;
    turningSpeed = 1.0f;
    movementSpeed = 0.01f;
    position = new QVector3D();
    automoveTimer = new QTimer(this);
    automoveInterval = 300;
    somersaultTimer = new QTimer(this);
    lightDirection = QVector3D(0.0f, 1.0f, 0.0f);
    lightDirection.normalize();
    lightIntensity = QVector3D(.8f, .8f, .8f);
    connect(automoveTimer, SIGNAL(timeout()), this, SLOT(automove()));
    connect(somersaultTimer, SIGNAL(timeout()), this, SLOT(somersault()));
}

MyWidget::~MyWidget()
{
    makeCurrent();
    vbo.destroy();
    delete program;
    for (int j = 0; j < 6; ++j)
        delete textures[j];
    doneCurrent();
}

void MyWidget::initializeGL()
{
    int i, j;
    initializeOpenGLFunctions();

    setFocusPolicy(Qt::TabFocus);

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);
    // Enable back face culling
    glEnable(GL_CULL_FACE);

    clearColor.setRgbF(0.1, 0.1, 0.1, 1.0);

    vao.create(); vao.bind();

    meshXSize = 240;
    meshYSize = 240;

    float **hmap = new float*[meshXSize];
    for (i = 0; i < meshXSize; i++) {
        hmap[i] = new float[meshYSize];
        for (j = 0; j < meshYSize; j++) {
            hmap[i][j] = (1.0f/((float)(20 + (i-120)*(i-120) + (j-120)*(j-120)))) - .1f;
        }
    }


    //loadCubes();
    addHeightMap(hmap, 0.0f, 2.0f, 0.0f, 2.0f);
    initMat();
    initShaders();
}

void MyWidget::initMat()
{
    QMatrix4x4 proj;
    QMatrix4x4 view;
    proj.perspective(45.0f, 1.0f, 0.000001f, 100.0f);
    view.lookAt(QVector3D(0.0f, 0.0, 0.0f), QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.0f, 1.0f, 0.0f));
    mvpMat0 = proj * view;
    mvpMat = mvpMat0;
    //mvpMat.rotate(20.0f, 0.0f, 1.0f, 0.0f);

}

void MyWidget::initShaders()
{
#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_COLOR_ATTRIBUTE 1
#define PROGRAM_NORMAL_ATTRIBUTE 2

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc =
            "#version 330\n"
            "layout (location = 0) in vec4 vertex;\n"
            "layout (location = 1) in vec4 color;\n"
            "layout (location = 2) in vec3 normal;\n"
            "uniform mat4 matrix;\n"
            "uniform vec3 lightIntensity;\n"
            "uniform vec3 lightDirection;\n"
            "out vec4 clr;\n"
            "out vec3 norm;\n"
            "void main(void)\n"
            "{\n"
            "   vec4 n = normalize(matrix * vec4(normal, 0.0));\n"
            "   clr = vec4(lightIntensity, 1.0) * color * max( dot( vec4(lightDirection, 0.0), n ), 0.0);\n"
            "   norm = normal;\n"
            "   gl_Position = matrix * vertex;\n"
            "}\n";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc =
            "#version 330\n"
            "in vec4 clr;\n"
            "in vec3 norm;\n"
            "out vec4 ffColor;\n"
            "void main(void)\n"
            "{\n"
            "       ffColor = clr;\n"
            "}\n";
    fshader->compileSourceCode(fsrc);

    program = new QOpenGLShaderProgram;
    program->addShader(vshader);
    program->addShader(fshader);
    program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    program->bindAttributeLocation("color", PROGRAM_COLOR_ATTRIBUTE);
    program->bindAttributeLocation("normal", PROGRAM_NORMAL_ATTRIBUTE);

    program->link();

    program->bind();
    program->setUniformValue("fTexture", 0);

}

void MyWidget::addHeightMapVertex(QVector<GLfloat> &vertData, QVector3D &position, QVector3D &normal, QVector4D &color) {
    /* Vertex Info */
    vertData.append(position.x());
    vertData.append(position.y());
    vertData.append(position.z());
    /* Color Info */
    vertData.append(color.x());
    vertData.append(color.y());
    vertData.append(color.z());
    vertData.append(color.w());
    /* Normal Info */
    vertData.append(normal.x());
    vertData.append(normal.y());
    vertData.append(normal.z());
}

void MyWidget::addHeightMap(float **hmap, float minXCoord, float maxXCoord, float minZCoord, float maxZCoord)
{
    int i, j;
    QVector<GLfloat> vertData;
    int xSize = meshXSize;
    int ySize = meshYSize;
    QVector4D color = QVector4D(0.0f,0.8f,0.0f,1.0f);
    QVector3D v1, v2, v3, v4, normal1, normal2;
    float scaleFactorX = (maxXCoord - minXCoord) / (float) meshXSize;
    float scaleFactorZ = (maxZCoord - minZCoord) / (float) meshYSize;
    for (i = 0; i < xSize-1; i++) {
        for (j = 0; j < ySize-1; j++) {
            // Create the four vertices in the mesh
            v1 = QVector3D(((float) i) * scaleFactorX, hmap[i][j], ((float) j) * scaleFactorZ );
            v2 = QVector3D(((float) i) * scaleFactorX, hmap[i][j+1], ((float) j+1) * scaleFactorZ );
            v3 = QVector3D(((float) i+1) * scaleFactorX, hmap[i+1][j], ((float) j) * scaleFactorZ );
            v4 = QVector3D(((float) i+1) * scaleFactorX, hmap[i+1][j+1], ((float) j+1) * scaleFactorZ );
            normal1 = QVector3D::crossProduct(v1 - v3, v2 - v1);
            normal2 = QVector3D::crossProduct(v3 - v4, v2 - v4);
            normal1.normalize();
            normal2.normalize();
            //Triangle 1
            addHeightMapVertex(vertData, v1, normal1, color);
            addHeightMapVertex(vertData, v2, normal1, color);
            addHeightMapVertex(vertData, v3, normal1, color);
            //Triangle 2
            addHeightMapVertex(vertData, v3, normal2, color);
            addHeightMapVertex(vertData, v2, normal2, color);
            addHeightMapVertex(vertData, v4, normal2, color);
        }
    }
    vbo.create();
    vbo.bind();
    vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));

}

void MyWidget::loadCubes()
{
    // Min at iCds[0][1]
    // Max at iCds[5][2]
    static const int iCds[6][4][3] = {
        { { +1, -1, -1 }, { -1, -1, -1 }, { -1, +1, -1 }, { +1, +1, -1 } },
        { { +1, +1, -1 }, { -1, +1, -1 }, { -1, +1, +1 }, { +1, +1, +1 } },
        { { +1, -1, +1 }, { +1, -1, -1 }, { +1, +1, -1 }, { +1, +1, +1 } },
        { { -1, -1, -1 }, { -1, -1, +1 }, { -1, +1, +1 }, { -1, +1, -1 } },
        { { +1, -1, +1 }, { -1, -1, +1 }, { -1, -1, -1 }, { +1, -1, -1 } },
        { { -1, -1, +1 }, { +1, -1, +1 }, { +1, +1, +1 }, { -1, +1, +1 } }
    };
    float coords[6][4][3];
    QVector<GLfloat> vertData;

    moveCube(iCds, coords, 1.5f, 0.5f, 10.0f, 0.05f);
    addCube(vertData, coords, 1.0f, 0.0f, 0.0f, 0.5f);

    moveCube(iCds, coords, -1.5f, 0.5f, 10.0f, 0.05f);
    addCube(vertData, coords, 0.0f, 1.0f, 0.0f, 0.5f);

    moveCube(iCds, coords, -1.5f, 0.5f, 20.0f, 0.05f);
    addCube(vertData, coords, 0.0f, 0.0f, 1.0f, 0.5f);

    moveCube(iCds, coords, 1.5f, 0.5f, 20.0f, 0.05f);
    addCube(vertData, coords, 1.0f, 1.0f, 0.0f, 0.5f);

    moveCube(iCds, coords, -1.5f, 0.5f, 15.0f, 0.05f);
    addCube(vertData, coords, 0.0f, 1.0f, 1.0f, 0.5f);

    moveCube(iCds, coords, 1.5f, 0.5f, 15.0f, 0.05f);
    addCube(vertData, coords, 1.0f, 0.0f, 1.0f, 0.5f);

    for (int j = 0; j < 6; ++j)
    {
        if(RESOURCE_FLAG)
        {
            textures[j] = new QOpenGLTexture(QImage(QString(":/images/side%1.png").arg(j + 1)).mirrored(true, false));
        }
        else
        {
            textures[j] = new QOpenGLTexture(QImage(txtPath+QString("/side%1.png").arg(j + 1)).mirrored(true, false));
        }
    }

    vbo.create();
    vbo.bind();
    vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));
}

void MyWidget::addCube(QVector<GLfloat> &vertData,float coords[6][4][3], float red, float green, float blue, float alpha)
{
    cubeMinPoints.push_back(new QVector3D(coords[0][1][0], coords[0][1][1], coords[0][1][2]));
    cubeMaxPoints.push_back(new QVector3D(coords[5][2][0], coords[5][2][1], coords[5][2][2]));

    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 4; ++j) {
            // vertex position
            vertData.append(coords[i][j][0]);
            vertData.append(coords[i][j][1]);
            vertData.append(coords[i][j][2]);
            // texture coordinate
            vertData.append(j == 0 || j == 3);
            vertData.append(j == 0 || j == 1);
            // color value
            vertData.append(red);
            vertData.append(green);
            vertData.append(blue);
            vertData.append(alpha);
        }
    }
}

void MyWidget::moveCube(const int cords[6][4][3], float (&nCds)[6][4][3], float x, float y, float z, float scale)
{
    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            nCds[i][j][0] = (cords[i][j][0]+x)*scale;
            nCds[i][j][1] = (cords[i][j][1]+y)*scale;
            nCds[i][j][2] = (cords[i][j][2]+z)*scale;
        }
    }
}

void MyWidget::paintGL()
{
    //background
    glClearColor(clearColor.redF(), clearColor.greenF(), clearColor.blueF(), clearColor.alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    program->setUniformValue("matrix", mvpMat);
    program->setUniformValue("lightDirection", lightDirection);
    program->setUniformValue("lightIntensity", lightIntensity);
    program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    program->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3, 10 * sizeof(GLfloat));
    program->enableAttributeArray(PROGRAM_COLOR_ATTRIBUTE);
    program->setAttributeBuffer(PROGRAM_COLOR_ATTRIBUTE, GL_FLOAT, 3 * sizeof(GLfloat), 4, 10 * sizeof(GLfloat));
    program->enableAttributeArray(PROGRAM_NORMAL_ATTRIBUTE);
    program->setAttributeBuffer(PROGRAM_NORMAL_ATTRIBUTE, GL_FLOAT, 7 * sizeof(GLfloat), 3, 10 * sizeof(GLfloat));

    glDrawArrays(GL_TRIANGLES, 0, 6 * ((meshXSize-1) * (meshYSize-1)));

}

void MyWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
}

/* Move the camera forward by the specified amount. Forward is relative to the direction the camera is facing */
void MyWidget::moveCameraForward(float amount) {
    int i;
    boolean collision = false;
    float xMovement = -cos(PI * horizontalAngle/180.0f)*amount;
    float zMovement = -sin(PI * horizontalAngle/180.0f)*amount;
    for (i = 0; i < cubeMinPoints.size(); i++) {
        if (collides(cubeMinPoints.at(i), cubeMaxPoints.at(i), new QVector3D(position->x() - xMovement, position->y(), position->z() - zMovement))) {
            collision = true;
        }
    }
    if (!collision) {
        mvpMat.translate(xMovement, 0.0f, zMovement);
        position->setX(position -> x() - xMovement);
        position->setZ(position -> z() - zMovement);
    }
}

/* Rotate view around the y-axis anchored at camera by specified number of degrees */
void MyWidget::rotateCamera(float degrees, float x, float y, float z) {
    mvpMat.translate(position->x(), position->y(), position->z()); // Translate to origin to rotate around camera
    mvpMat.rotate(degrees, x, y, z);
    mvpMat.translate(-position->x(), -position->y() , -position->z());
}

/* Private slot used for automove QTimer */
void MyWidget::automove() {
    moveCameraForward(.01f);
    update();
}

void MyWidget::somersault() {
    verticalAngle += turningSpeed;
    if (verticalAngle > 360.0f) {
        somersaultTimer->stop(); // Stop after one full rotation
        verticalAngle = 0.0f;
    }
    rotateCamera(turningSpeed, -sin(PI * horizontalAngle/180.0f), 0.0f, cos(PI * horizontalAngle/180.0f)); //Rotate perpendicular to the camera's forward and up directions
    update();
}

/* Detects whether a cube represented by 2 points intersects a point */
boolean MyWidget::collides(QVector3D *min, QVector3D *max, QVector3D *point) {
    return (point->x() > min->x() && point->x() < max->x()
            && point->y() > min->y() && point->y() < max->y()
            && point->z() > min->z() && point->z() < max->z());
}

void MyWidget::mousePressEvent(QMouseEvent *ev) {
    if (ev->button() == Qt::LeftButton) {
        rotateCamera(-turningSpeed, 0.0f, 1.0f, 0.0f);
        horizontalAngle -= turningSpeed;
    } else if (ev->button() == Qt::RightButton) {
        rotateCamera(turningSpeed, 0.0f, 1.0f, 0.0f);
        horizontalAngle += turningSpeed;
    } else {
        QWidget::mousePressEvent(ev);
    }
    update();
}

void MyWidget::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Left) {
        rotateCamera(-turningSpeed, 0.0f, 1.0f, 0.0f);
        horizontalAngle -= turningSpeed;
    } else if (ev->key() == Qt::Key_Right) {
        rotateCamera(turningSpeed, 0.0f, 1.0f, 0.0f);
        horizontalAngle += turningSpeed;
    } else if (ev->key() == Qt::Key_Up) {
        moveCameraForward(movementSpeed);
    } else if (ev->key() == Qt::Key_Down) {
        moveCameraForward(-movementSpeed);
    } else if (ev->key() == Qt::Key_Space) {
        // Automove every 300 ms
        if (!automoveTimer->isActive())
            automoveTimer->start(automoveInterval);
        else
            automoveTimer->stop();
    }
    else if (ev->key() == Qt::Key_F) {
        if (automoveInterval > 30)
            automoveInterval -= 20;
        automoveTimer->setInterval(automoveInterval);
    } else if (ev->key() == Qt::Key_S) {
        if (automoveInterval > 30)
            automoveInterval += 20;
        automoveTimer->setInterval(automoveInterval);
    }else if (ev->key() == Qt::Key_M) {
        somersaultTimer->start(50);
    } else {
        QWidget::keyPressEvent(ev);
    }
    /* Angle should never exceed 360 degrees */
    if (horizontalAngle > 360.0f) {
        horizontalAngle -= 360.0f;
    }
    update();
}

