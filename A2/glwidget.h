//-------------------------------------------------------------------------------------------
//  University of Bath Computer Science Department
//	FrameWork for OpenGL application under QT
//  Course title: Computer Graphics CM20219
//-------------------------------------------------------------------------------------------
//These two lines are header guardians against multiple includes
#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <QProgressBar>
#include "foundation.h"
#include <QtGui>
#include <QtOpenGL>
#include <glu.h>
#include "version.h"


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
    int winw, winh, button,imx,imy;
    int version;
    void dopan(int x, int y, bool click);
    void zoomWheel(int z);
    void displayImage();

    // Functions
    void redraw();
    void initLight();

    Vector3d CameraPos;
    QPoint lastMousePoint;
    QPoint initialMousePoint;


    QVector<Vector3d> pointList;

    bool Rotating;
    bool Scaling;
    void RotateY(Vector3d * vec_in, double rad);
    void RotateZ(Vector3d * vec_in, double rad);
    void DoRotate(QPoint desc, QPoint orig);
    void DoScale(QPoint desc, QPoint orig);
};


#endif
