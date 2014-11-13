//-------------------------------------------------------------------------------------------
//  University of Bath Computer Science Department
//	FrameWork for OpenGL application under QT
//  Course title: Computer Graphics CM20219
//-------------------------------------------------------------------------------------------
//These two lines are header guardians against multiple includes
#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
//#include <QProgressBar>
#include "foundation.h"
#include <QtGui>
#include <QtOpenGL>
#include <glu.h>


const int maxw=1024;
const int maxh=1024;
//This is our OpenGL Component we built it on top of QGLWidget
class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    //Constructor for GLWidget
    GLWidget(QWidget *parent = 0);

    //Destructor for GLWidget
    ~GLWidget();

    void clear();
    void help();
    void clearPoints();
    void togglePoints();
    void toggleOrtho(int index);
    void toggleAnim();
    void playPauseAnim();
    void toggleCylinder();
    void changeCylinderShape(int shape);

protected:
    //Initialize the OpenGL Graphics Engine
    void initializeGL();

    //All our painting stuff are here
    void paintGL();

    //When user resizes main window, the scrollArea will be resized and it will call this function from
    //its attached GLWidget
    void resizeGL(int width, int height);
    //Handle mouse
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    void startup();
    QVector3D crossSectionToWorld(QVector3D vec, float mat[3][3]);
    int winw, winh, button,imx,imy;
    void dopan(int x, int y, bool click);
    void zoomWheel(int z);
    void displayImage();
    void drawCube(bool changeColours);
    void drawEndFrames(bool first, QVector3D N, QVector3D B, QVector3D P);
    void drawFrenetFrame(QVector3D prevN, QVector3D prevB, QVector3D prevP, QVector3D N, QVector3D B, QVector3D P, int frame);

    // Functions
    void redraw();
    void initLight();
    QVector3D convertWindowToWorld(float x, float y, float z);
    int nearestPointToRay(int mouseX, int mouseY);

    Vector3d CameraPos;
    QPoint lastMousePoint;
    QPoint initialMousePoint;
    int selectedPoint;


    QVector<QVector3D> pointList;
    QVector<QVector3D> crossSectionCoords;

    enum Perspective { P, XPOS, XNEG, ZPOS, ZNEG };
    Perspective currentPerspective;
    enum CylinderShape { SQUARE, CIRCLE, STAR, LINE, CUSTOM };
    CylinderShape currentCylinderShape;
    bool Rotating;
    bool displayPoints;
    enum Dragging { NONE, X, Y, Z };
    Dragging dragAxis;
    bool Scaling;
    void RotateY(Vector3d * vec_in, double rad);
    void RotateZ(Vector3d * vec_in, double rad);
    void DoRotate(QPoint desc, QPoint orig);
    void DoScale(QPoint desc, QPoint orig);
    void DoDrag(QPoint desc, QPoint orig);

    bool displayCylinder;

    QTimer *animTimer;
    bool cubeOn;
    bool animPlay;
    int currentCubeFrame;
    int totalFrames;

private slots:
    void incrementCubePos();

};


#endif
