/****************************************************************************
**
Window for OpenGL in QT.
Start code for CMSC 427, Spring 2015
Reference: cube & texture example in Qt Creator
author: Zheng Xu, xuzhustc@gmail.com
**
****************************************************************************/

#include "terrainwindow.h"

TerrainWindow::TerrainWindow(QWidget *parent)
    : QOpenGLWidget(parent)
{
    memset(textures, 0, sizeof(textures));
    txtPath = TXT_IMG_PATH;
    cubNum = CUBE_NUM;
    horizontalAngle = 90.0f;
    verticalAngle = 0.0f;
    turningSpeed = 1.0f;
    movementSpeed = 0.005f;
    position = new QVector3D();
    automoveTimer = new QTimer(this);
    automoveInterval = 300;
    somersaultTimer = new QTimer(this);
    lightDirection = QVector3D(1.0f, 1.0f, 1.0f);
    lightDirection.normalize();
    lightIntensity = QVector3D(1.0f, 1.0f, 1.0f);
    meshSize = 1 + pow(2, 8);
    minCoord = -1.0f;
    maxCoord = 1.0f;
    minHeight = 0.2f;
    maxHeight = -0.2;
    connect(automoveTimer, SIGNAL(timeout()), this, SLOT(automove()));
    connect(somersaultTimer, SIGNAL(timeout()), this, SLOT(somersault()));
}

TerrainWindow::~TerrainWindow()
{
    makeCurrent();
    vbo.destroy();
    delete program;
    for (int j = 0; j < 6; ++j)
        delete textures[j];
    doneCurrent();
}

void TerrainWindow::initializeGL()
{
    int i, j;
    initializeOpenGLFunctions();

    setFocusPolicy(Qt::TabFocus);

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);
    // Enable back face culling
    glEnable(GL_CULL_FACE);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);

    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    clearColor.setRgbF(0.1, 0.1, 0.1, 1.0);

    vao.create(); vao.bind();

    hmap = new float*[meshSize];
    for (i = 0; i < meshSize; i++) {
        hmap[i] = new float[meshSize];
    }


   /* float mean = meshSize/2.0f;
    float stdDev = 45.0f;
    hmap = new float*[meshSize];
    for (i = 0; i < meshSize; i++) {
        hmap[i] = new float[meshSize];
        for (j = 0; j < meshSize; j++) {
            hmap[i][j] = (1.0f / 24.0f) * expf(-((i-mean)*(i-mean) + (j-mean)*(j-mean))/(2.0f * stdDev * stdDev));
        }
    }

    double max = 0.0;
    for (i = 0; i < meshSize; i++) {
        for (j = 0; j < meshSize; j++) {
            if (hmap[i][j] > max) {
                max = hmap[i][j];
            }
        }
    }
    for (i = 0; i < meshSize; i++) {
        for (j = 0; j < meshSize; j++) {
            hmap[i][j] /= max;
            hmap[i][j] = (hmap[i][j] / (maxHeight - minHeight)) - minHeight;
        }
    }*/

    dsFractal(.2, .2, .3, .2, .01);
    for (i = 0; i < meshSize; i++) {
        for (j = 0; j < meshSize; j++) {
            printf("%f ", hmap[i][j]);
        }
        printf("\n");
    }
    addHeightMap(hmap);
    initMat();
    initShaders();
}

void TerrainWindow::initMat()
{
    QMatrix4x4 proj;
    QMatrix4x4 view;
    proj.perspective(45.0f, 1.0f, 0.000001f, 100.0f);
    view.lookAt(QVector3D(0.0f, 0.0, 0.0f), QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.0f, 1.0f, 0.0f));
    mvpMat0 = proj * view;
    mvpMat = mvpMat0;
    int xHeightMapCoord = round(((position -> x() - minCoord) * (float) meshSize)/(maxCoord-minCoord));
    int yHeightMapCoord = round(((position -> z() - minCoord) * (float) meshSize)/(maxCoord-minCoord));
    //  position->setY()
    position->setY(hmap[xHeightMapCoord][yHeightMapCoord]);
    mvpMat.translate(0.0f, -position->y() - .01f, 0.0f);
    //mvpMat.rotate(20.0f, 0.0f, 1.0f, 0.0f);


}

void TerrainWindow::initShaders()
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

void TerrainWindow::addHeightMapVertex(QVector<GLfloat> &vertData, QVector3D &position, QVector3D &normal, QVector4D &color) {
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

QVector4D TerrainWindow::getColor(float height, QVector3D low, QVector3D mid, QVector3D high)  {
    float coefficient = (height + minHeight)/(maxHeight-minHeight);
    QVector3D color = low * (1.0f - coefficient) + high * (coefficient);
    return QVector4D(color.x(), color.y(), color.z(), 1.0f);
}

void TerrainWindow::dsFractal(float a, float b, float c, float d, float rough) {

 // unsigned char meshCount;

  unsigned int meshCount;
  unsigned int i,j;
  float r;
  // seed corners of array
  hmap[0][0] = a;
  hmap[meshSize-1][0] = b;
  hmap[0][meshSize-1] = c;
  hmap[meshSize-1][meshSize-1] = d;
  // seed the RNG with time
  srand(time(NULL));

  // iterate through meshScales until reaching floor of 1
  for (meshCount = meshSize; meshCount > 2; meshCount = 1 + meshCount/2) {

    // diamond step
    for (i = meshCount/2; i < meshSize; i += meshCount-1) {
        for (j = meshCount/2; j < meshSize; j += meshCount-1) {
            r = rand() / (float)RAND_MAX;
            hmap[i][j] = rough*r + 0.25f*(
                hmap[i-meshCount/2][j-meshCount/2]
                + hmap[i+meshCount/2][j-meshCount/2]
                + hmap[i-meshCount/2][j+meshCount/2]
                + hmap[i+meshCount/2][j+meshCount/2]
            );
        }
    }

    // square step

    // even rows
    // top row
    for (j = meshCount/2; j < meshSize; j += meshCount-1) {
        r = rand() / (float)RAND_MAX;
        hmap[0][j] = rough*r + (
            hmap[0][j-meshCount/2]
            + hmap[0][j+meshCount/2]
            + hmap[meshCount/2][j]
        )/3.0f;
    }
    // middle evens
    for (i = meshCount-1; i < meshSize-(meshCount-1); i += meshCount-1) {
        for (j = meshCount/2; j < meshSize; j += meshCount-1) {
            r = rand() / (float)RAND_MAX;
            hmap[i][j] = rough*r + 0.25f*(
                hmap[i][j-meshCount/2]
                + hmap[i][j+meshCount/2]
                + hmap[i-meshCount/2][j]
                + hmap[i+meshCount/2][j]
            );
        }
    }
    // bottom row
    for (j = meshCount/2; j < meshSize; j += meshCount-1) {
        r = rand() / (float)RAND_MAX;
        hmap[meshSize-1][j] = rough*r + (
            hmap[meshSize-1][j-meshCount/2]
            + hmap[meshSize-1][j+meshCount/2]
            + hmap[i-meshCount/2][j]
        )/3.0f;
    }

    // odd rows
    for (i = meshCount/2; i < meshSize; i += meshCount-1) {
        // left column
        r = rand() / (float)RAND_MAX;
        hmap[i][0] = rough*r + (
            hmap[i][meshCount/2]
            + hmap[i-meshCount/2][0]
            + hmap[i+meshCount/2][0]
        )/3.0f;
        // middle columns
        for (j = meshCount-1; j < meshSize; j += meshCount-1) {
            r = rand() / (float)RAND_MAX;
            hmap[i][j] = rough*r + 0.25f*(
                hmap[i][j-meshCount/2]
                + hmap[i][j+meshCount/2]
                + hmap[i-meshCount/2][j]
                + hmap[i+meshCount/2][j]
            );
        }
        // right column
        r = rand() / (float)RAND_MAX;
        hmap[i][meshSize-1] = rough*r + (
            hmap[i][(meshSize-1)-meshCount/2]
            + hmap[i-meshCount/2][meshSize-1]
            + hmap[i+meshCount/2][meshSize-1]
        )/3.0f;
    }

  }

  return;
}




void TerrainWindow::addHeightMap(float **hmap)
{
    int i, j;
    QVector<GLfloat> vertData;
    int xSize = meshSize;
    int ySize = meshSize;
    QVector4D color = QVector4D(0.0f,0.8f,0.0f,1.0f);
    QVector3D v1, v2, v3, v4, normal1, normal2;
    QVector4D colorv1, colorv2, colorv3, colorv4;
    QVector3D colorLow = QVector3D(0.0f, 0.8f, 0.0f);
    QVector3D colorMid = QVector3D(0.3f, 0.3f, 0.3f);
    float scaleFactor = ( maxCoord - minCoord ) / (float) meshSize;
    for (i = 0; i < xSize-1; i++) {
        for (j = 0; j < ySize-1; j++) {
            // Create the four vertices in the mesh
            v1 = QVector3D(minCoord + ((float) i) * scaleFactor, hmap[i][j], minCoord + ((float) j) * scaleFactor );
            v2 = QVector3D(minCoord + ((float) i) * scaleFactor, hmap[i][j+1], minCoord + ((float) j+1) * scaleFactor );
            v3 = QVector3D(minCoord + ((float) i+1) * scaleFactor, hmap[i+1][j], minCoord + ((float) j) * scaleFactor );
            v4 = QVector3D(minCoord + ((float) i+1) * scaleFactor, hmap[i+1][j+1], minCoord + ((float) j+1) * scaleFactor );
            normal1 = QVector3D::crossProduct(v1 - v3, v2 - v1);
            normal1.normalize();
            normal2 = QVector3D::crossProduct(v3 - v4, v2 - v4);
            normal2.normalize();
            colorv1 = getColor(hmap[i][j], colorLow, colorMid, colorMid);
            colorv2 = getColor(hmap[i][j+1], colorLow, colorMid, colorMid);
            colorv3 = getColor(hmap[i+1][j], colorLow, colorMid, colorMid);
            colorv4 = getColor(hmap[i+1][j+1], colorLow, colorMid, colorMid);
            color = QVector4D(0.0f, 1.0f, 0.0f, 1.0f);
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

void TerrainWindow::paintGL()
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

    glDrawArrays(GL_TRIANGLES, 0, 6 * ((meshSize-1) * (meshSize-1)));

}

void TerrainWindow::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
}

/* Move the camera forward by the specified amount. Forward is relative to the direction the camera is facing */
void TerrainWindow::moveCameraForward(float amount) {
    int i;
    boolean collision = false;
    float xMovement = -cos(PI * horizontalAngle/180.0f)*amount;
    float zMovement = -sin(PI * horizontalAngle/180.0f)*amount;
    position->setX(position -> x() - xMovement);
    position->setZ(position -> z() - zMovement);
    int xHeightMapCoord = round(((position -> x() - minCoord) * (float) meshSize)/(maxCoord-minCoord));
    int yHeightMapCoord = round(((position -> z() - minCoord) * (float) meshSize)/(maxCoord-minCoord));
    float test = hmap[xHeightMapCoord][yHeightMapCoord] - position->y();
    position->setY(hmap[xHeightMapCoord][yHeightMapCoord]);
    mvpMat.translate(xMovement, -test, zMovement);
}

/* Rotate view around the y-axis anchored at camera by specified number of degrees */
void TerrainWindow::rotateCamera(float degrees, float x, float y, float z) {
    mvpMat.translate(position->x(), position->y(), position->z()); // Translate to origin to rotate around camera
    mvpMat.rotate(degrees, x, y, z);
    mvpMat.translate(-position->x(), -position->y() , -position->z());
}

/* Private slot used for automove QTimer */
void TerrainWindow::automove() {
    moveCameraForward(.01f);
    update();
}

void TerrainWindow::somersault() {
    verticalAngle += turningSpeed;
    if (verticalAngle > 360.0f) {
        somersaultTimer->stop(); // Stop after one full rotation
        verticalAngle = 0.0f;
    }
    rotateCamera(turningSpeed, -sin(PI * horizontalAngle/180.0f), 0.0f, cos(PI * horizontalAngle/180.0f)); //Rotate perpendicular to the camera's forward and up directions
    update();
}

/* Detects whether a cube represented by 2 points intersects a point */

void TerrainWindow::mousePressEvent(QMouseEvent *ev) {
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

/*float TerrainWindow::bilerp(float x, float z) {

}*/

void TerrainWindow::keyPressEvent(QKeyEvent *ev)
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

