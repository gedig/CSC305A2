//-------------------------------------------------------------------------------------------
//   Qtgl for graphics class
//-------------------------------------------------------------------------------------------

#include "glwidget.h"

#define PLANE_SIZE 100
#define GRID_DISTANCE 1


#define MAX_ZOOM 100
#define MIN_ZOOM 7
#define POINT_RADIUS .1
#define MIN_MOUSE_MOVE 9
const double RadPerPixel = - 0.01;
const double MovePerPixel = - 0.1;
const float DEG2RAD = 3.14159/180;

QVector3D convertWindowToWorld(float x, float y, float z)
{
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble worldX, worldY, worldZ;

    glGetDoublev( GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    winX = x;
    winY = viewport[3] - y;
    winZ = z;

    gluUnProject(winX, winY, winZ, modelview, projection, viewport, &worldX, &worldY, &worldZ);

    QVector3D worldPoint(worldX, worldY, worldZ);
    return worldPoint;
}

void calculateMouseRay(QVector3D &startingPoint, QVector3D &direction, int mouseX, int mouseY)
{
    startingPoint = convertWindowToWorld(mouseX, mouseY, 0.0);
    direction = convertWindowToWorld(mouseX, mouseY, 1.0);
}

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
    glShadeModel( GL_SMOOTH );

    // Set up various other stuff
    glClearColor( 0.1, 0.1, 0.1, 0.0 ); // Let OpenGL clear to black
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

    // Draws the xz plane
    glBegin(GL_QUADS);
    glColor3f(0.6, 0.6, 1.0);
    glVertex3f( -PLANE_SIZE, 0, -PLANE_SIZE);
    glVertex3f( -PLANE_SIZE, 0, PLANE_SIZE);
    glVertex3f( PLANE_SIZE, 0, PLANE_SIZE);
    glVertex3f( PLANE_SIZE, 0, -PLANE_SIZE);
    glEnd();

    // Draws the grid on the xz plane.
    glBegin(GL_LINES);
    for(int i = -PLANE_SIZE; i <= PLANE_SIZE; i += GRID_DISTANCE) {
        glColor3f(.2, .2, .2);
        glVertex3f(i,0,-PLANE_SIZE);
        glVertex3f(i,0,PLANE_SIZE);
        glVertex3f(-PLANE_SIZE,0,i);
        glVertex3f(PLANE_SIZE,0,i);
    };
    glEnd();

    // Draws the axis lines
    glLineWidth(3);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(-100.0, 0.0, 0.0);
    glVertex3f(100, 0, 0);
    glEnd();
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, -100.0);
    glVertex3f(0, 0, 100.0f);
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0f, 100.0f, 0.0f);
    glEnd();

    // Draws circles at every point, with a line between them.
    // TODO-DG: Turn this line into a catmull spline
    for (int i = 0; i < pointList.size(); i++) {
        glPushMatrix();
        glTranslatef(pointList[i].x(), pointList[i].y(), pointList[i].z());
        GLUquadric* quad = gluNewQuadric();
        gluSphere(quad, GLdouble(POINT_RADIUS), GLint(30), GLint(30));
        glPopMatrix(); // Applies the transform to the sphere without affecting the lines.

        // TODO-DG: Don't draw a line between the points, draw the catmull spline.
        if (i > 0) {
            glBegin(GL_LINES);
            glVertex3f(pointList[i-1].x(), pointList[i-1].y(), pointList[i-1].z());
            glVertex3f(pointList[i].x(), pointList[i].y(), pointList[i].z());
            glEnd();
        }
    }
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

    helpString = "Controls: \nLeft mouse button rotates the camera and selects points.";
    helpString += "\nRight mouse button controls zoom and deletes points.";
    helpString += "\nMiddle mouse button adds points.";
    QMessageBox::information( this, title, helpString, QMessageBox::Ok );
}

void GLWidget::clearPoints()
{
    pointList.clear();
    clear();
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

void GLWidget::mousePressEvent( QMouseEvent *e )
{
    if (e->button() == Qt::LeftButton)
    {
        lastMousePoint = e->pos();
        initialMousePoint = e->pos();
        Rotating = true;
    } else if (e->button() == Qt::RightButton) {
        lastMousePoint = e->pos();
        initialMousePoint = e->pos();
        Scaling = true;
    } else if (e->button() == Qt::MiddleButton) {
        //Create a point when middle mouse button is clicked
        QVector3D mVector = convertWindowToWorld( e->pos().x(), e->pos().y(), 0.1);
        if (mVector.y() >= 0) {
            pointList.push_back(mVector);
        }
    }
}



void GLWidget::mouseReleaseEvent( QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && Rotating)
    {
        float x = initialMousePoint.x() - e->pos().x();
        float y = initialMousePoint.y() - e->pos().y();
        float pointDistanceSquared = x*x + y*y;
        if (pointDistanceSquared > MIN_MOUSE_MOVE) {
            DoRotate(e->pos(), lastMousePoint);
            Rotating = false;
        } else {
            // TODO-DG: If movement is insignificant, ray trace for point to select.
        }

    }

    if (e->button() == Qt::RightButton && Scaling)
    {
        float x = initialMousePoint.x() - e->pos().x();
        float y = initialMousePoint.y() - e->pos().y();
        float pointDistanceSquared = x*x + y*y;
        if (pointDistanceSquared > MIN_MOUSE_MOVE) {
            DoScale(e->pos(), lastMousePoint);
            Scaling = false;
        } else {
            // TODO-DG: If movement is insignificant, ray trace for point to delete.
            QVector3D cameraRayStart;
            QVector3D cameraRayDirection;
            calculateMouseRay(cameraRayStart, cameraRayDirection, e->pos().x(), e->pos().y());
        }
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
    if (newLength <= MAX_ZOOM && newLength >= MIN_ZOOM)
    {
        double ratio = newLength / length;
        CameraPos.x = CameraPos.x * ratio;
        CameraPos.y = CameraPos.y * ratio;
        CameraPos.z = CameraPos.z * ratio;
    }
}
