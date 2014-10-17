//-------------------------------------------------------------------------------------------
//   Qtgl for graphics class
//-------------------------------------------------------------------------------------------

#include "glwidget.h"



const double lim=0.5;
const double RadPerPixel = - 0.01;
const double MovePerPixel = - 0.1;

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent)
{
    startup();
}

GLWidget::~GLWidget()
{    

}

void GLWidget::startup()
{
    winw=width();  // width returns width of window
    winh=height();
    button = 0;
    cerr << "Glwidget\n";
    version=MYVERSION;
    CameraPos.x = CameraPos.y = CameraPos.z = 5;
    Rotating = false;
    Scaling = false;
}

void GLWidget::clear()
{
     updateGL();
}

void GLWidget::initializeGL()
{

//    GLfloat whiteDir[4] = {2.0, 2.0, 2.0, 1.0};
//    GLfloat whiteAmb[4] = {1.0, 1.0, 1.0, 1.0};
//    GLfloat lightPos[4] = {30.0, 30.0, 30.0, 1.0};

//    glEnable(GL_LIGHTING);
//    glEnable(GL_LIGHT0);
//    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
//    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, whiteAmb);

//    glMaterialfv(GL_FRONT, GL_DIFFUSE, whiteDir);
//    glMaterialfv(GL_FRONT, GL_SPECULAR, whiteDir);
//    glMaterialf(GL_FRONT, GL_SHININESS, 20.0);

//    glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteDir);		// enable diffuse
//    glLightfv(GL_LIGHT0, GL_SPECULAR, whiteDir);	// enable specular
//    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glShadeModel( GL_SMOOTH );

    // Set up various other stuff
    glClearColor( 0.5, 1.0, 0.75, 0.0 ); // Let OpenGL clear to black
    glEnable( GL_CULL_FACE );  	// don't need Z testing for convex objects
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
}

void GLWidget::redraw()
{
        updateGL();
}

void GLWidget::paintGL()
{
    glClear( GL_COLOR_BUFFER_BIT );

    glLoadIdentity();
    gluLookAt(CameraPos.x,
              CameraPos.y,
              CameraPos.z, 0, 0, 0, 0.0, 1.0, 0.0);

    glLineWidth(3);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(100, 0, 0);
    glEnd();
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0f, 1000.0f, 0.0f);
    glEnd();
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0, 0, 1000.0f);
    glEnd();
}

/* 2D */
void GLWidget::resizeGL( int w, int h )
{
    glViewport( 0, 0, (GLint)w, (GLint)h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -1.0, 1.0, -1.0, 1.0, 5.0, 1500.0 );
    glMatrixMode( GL_MODELVIEW );
}

void GLWidget::help()
{
    QString helpString;
    QString title="Application Help";

    helpString="Control Information goes here: ";
    QMessageBox::information( this, title, helpString, QMessageBox::Ok );
}




void GLWidget::initLight()
{
   GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat mat_shininess[] = { 128.0 };
   GLfloat light_position[] = { 0.2, 0.2, 0.9, 0.0 };
   GLfloat light_ambient[] = { 0.05, 0.05, 0.05, 1.0 };
   GLfloat light_specular[] = { 0.99, 0.99, 0.99, 1.0 };
   GLfloat light_diffuse[] = { 0.7, 0.7, 0.7, 1.0 };

   glShadeModel (GL_SMOOTH);

// MATERIAL
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

// LIGHT0
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
   glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

   glColorMaterial(GL_FRONT, GL_DIFFUSE);
   glEnable(GL_COLOR_MATERIAL);

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);

}

void GLWidget::createAxes( )
{

}

void GLWidget::mousePressEvent( QMouseEvent *e )
{
    if (e->button() == Qt::LeftButton)
    {
        lastMousePoint = e->pos();
        Rotating = true;
    } else if (e->button() == Qt::RightButton) {
        lastMousePoint = e->pos();
        Scaling = true;
    } else if (e->button() == Qt::MiddleButton) {
        //TODO-DG: Create a point when middle mouse button is clicked
    }
}

void GLWidget::mouseReleaseEvent( QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && Rotating)
    {
        DoRotate(e->pos(), lastMousePoint);
        Rotating = false;
    }

    if (e->button() == Qt::RightButton && Scaling)
    {
        DoScale(e->pos(), lastMousePoint);
        Scaling = false;
    }

    updateGL();
}

void GLWidget::mouseMoveEvent( QMouseEvent *e )
{
    if ((e->buttons() & Qt::LeftButton) && Rotating)
    {
        DoRotate(e->pos(), lastMousePoint);
        lastMousePoint = e->pos();
    }

    if ((e->buttons() & Qt::RightButton) && Scaling)
    {
        DoScale(e->pos(), lastMousePoint);
        lastMousePoint = e->pos();
    }

    updateGL();
}



void GLWidget::RotateY(Vector3d * pVec, double rad)
{
    double cosPhi = (double)cos(rad);
    double sinPhi = (double)sin(rad);

    Matrix33d m;
    m.M11 = cosPhi;
    m.M13 = sinPhi;
    m.M22 = 1;
    m.M31 = - sinPhi;
    m.M33 = cosPhi;
    *pVec = MultiplyMatrix33Vec3(*pVec, m);
}

void GLWidget::RotateZ(Vector3d * pVec, double rad)
{
    double cosPhi = (double)cos(rad);
    double sinPhi = (double)sin(rad);

    Matrix33d m;
    m.M11 = cosPhi;
    m.M12 = - sinPhi;
    m.M21 = sinPhi;
    m.M22 = cosPhi;
    m.M33 = 1;
    *pVec = MultiplyMatrix33Vec3(*pVec, m);
}

void GLWidget::DoRotate(QPoint desc, QPoint orig)
{
    double YRot = (desc.x() - orig.x()) * RadPerPixel;
    double ZRot = (desc.y() - orig.y()) * RadPerPixel;

    RotateY(&CameraPos, YRot);
    RotateZ(&CameraPos, ZRot);
}

void GLWidget::DoScale(QPoint desc, QPoint orig)
{
    double length = sqrt(CameraPos.x * CameraPos.x + CameraPos.y * CameraPos.y);
    double newLength = length + (desc.y() - orig.y()) * MovePerPixel;
    if (newLength > lim)
    {
        double ratio = newLength / length;
        CameraPos.x = CameraPos.x * ratio;
        CameraPos.y = CameraPos.y * ratio;
        CameraPos.z = CameraPos.z * ratio;
    }
}
