#include "basicopenglview.h"

BasicOpenGLView::BasicOpenGLView(QWidget *parent)
      : QGLWidget(parent)
{
    drawon=false;
    clearon = false;
    mousedown = false;
    currentPoint = -1;
}

void BasicOpenGLView::initializeGL()
{
    resize(300,300);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glShadeModel( GL_FLAT );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glPointSize(5);
}

void BasicOpenGLView::resizeGL(int width, int height)
{
    glViewport(0, 0, 300, 300);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0,GLdouble(300),0,GLdouble(300),-10.0,10.0);
    //glOrtho(-(GLdouble)width/2.0, (GLdouble)width/2.0, -(GLdouble)height/2.0, (GLdouble)height/2.0, -10.0,10.0);

    glMatrixMode(GL_MODELVIEW);
}

void BasicOpenGLView::paintGL()
{
    // This paint function iterates through matrixShapeList and applies the specified matrix
    // to the appropriate polygons.

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    drawFigure();

    // TODO-DG: Send the information over to glwidget somehow...
    //(Window)parentRef->updateCustomCylinder(shapes);
    //void updateCustomCylinder(QVector<QVector3D> points);
    mainWidget->updateCustomCylinder(shapes);
}


void BasicOpenGLView::mousePressEvent(QMouseEvent *e)
{
    if (mousedown) return;
    mousedown = true;
    currentPoint = -1;

    if (e->button() == Qt::RightButton) addPoint( e->x(), height()-e->y() );
    if (e->button() == Qt::MiddleButton) addPoint( e->x(), height()-e->y(), true );
    if (e->button() == Qt::LeftButton) movePoint(e->x(), height()-e->y() );
    update();
}

void BasicOpenGLView::mouseMoveEvent(QMouseEvent *e)
{
    if (mousedown) movePoint(e->x(), height()-e->y());
    update();
}

void BasicOpenGLView::mouseReleaseEvent(QMouseEvent *e)
{
    //Finished moving the point
    movePoint(e->x(), height()-e->y());
    mousedown = false;
    update();
}

void BasicOpenGLView::movePoint(int x, int y)
{
    if (mousedown) { // Only move if mouse is down.
        if (currentPoint < 0) {
            select(x,y); // select current point
        } else {
            // Only changes a point from a single shape.
            shapes[currentPoint].setX(x);
            shapes[currentPoint].setY(y);
        }
    }
}

// Adds a point to a shape if it is the only shape selected.
void BasicOpenGLView::addPoint(int x, int y, bool distinct)
{
        QVector3D tempPoint = QVector3D(x, y, 1);
        if (distinct)
            tempPoint.setZ(0);
        shapes.append(tempPoint);
}

// Returns true if a point is found, otherwise false
bool BasicOpenGLView::select(int x, int y)
{
    // Allows selection of any point from any shape.
    for (int i = 0; i < shapes.count(); i++) {
            if ( ( ( shapes[i].x()-RADIUS) <x && (shapes[i].x() + RADIUS) >x) && ( ( shapes[i].y()-RADIUS) <y && (shapes[i].y()+RADIUS) >y)) {
                currentPoint = i;
                return true;
            }
    }
    return false;
}


void BasicOpenGLView::setDraw(bool on)
{
    drawon=on;
}

void BasicOpenGLView::drawCircle(double radius, double xcen, double ycen, bool line)
{
   GLint i;
   static GLfloat circcoords[100][2];

   for(i=0;i<100;i++) {
     circcoords[i][0]=radius*cos(i*2*M_PI/100.0)+xcen;
     circcoords[i][1]=radius*sin(i*2*M_PI/100.0)+ycen;
   }
   if (line)   glBegin(GL_LINES); else glBegin(GL_POLYGON);
      for(i=0;i<100;i++)
         glVertex2fv(&circcoords[i][0]);
   glEnd();
}

void BasicOpenGLView::clearme()
{
    clearon=true;
    update();
}

void BasicOpenGLView::clearPoints()
{
    clearon=true;
    shapes.clear();
    update();
}

void BasicOpenGLView::drawLine(double x0, double y0, double x1, double y1 )
{
    glBegin(GL_LINES);
    glVertex2f(x0, y0);
    glVertex2f(x1, y1);
    glEnd();
}

// Draws every shape's points, altered by the matrix applied to them.
void BasicOpenGLView::drawFigure()
{
    int x0,x1,y0,y1;
    for (int i = 0; i < shapes.count(); i++) {
            x1 = shapes[i].x();
            y1 = shapes[i].y();
            glColor3f(0.0f, 1.0f, 0.0f);
            drawCircle( (double)RADIUS, x1, y1, false);

            //Draw a line between this point and the previous point (if it exists)
            if (i > 0 && shapes[i].z() != 0) {
                    glColor3f(1.0f, 0.0f, 1.0f);
                    drawLine(x0, y0, x1, y1);
            }
            x0 = x1;
            y0 = y1;
        }
}

