//-------------------------------------------------------------------------------------------
//   Qtgl for graphics class
//-------------------------------------------------------------------------------------------

#include "glwidget.h"

#define PLANE_SIZE 100
#define GRID_DISTANCE 1


#define MAX_ZOOM 100
#define MIN_ZOOM 7
#define POINT_RADIUS .1
#define MIN_MOUSE_MOVE 16
#define POINT_HANDLE_LENGTH 0.2
#define POINT_HANDLE_WIDTH 5
#define CATMULL_FIDELITY 20

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
    displayPoints = true;
    dragAxis = NONE;
    Scaling = false;
    selectedPoint = -1;
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
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, -100.0);
    glVertex3f(0, 0, 100.0f);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0f, 100.0f, 0.0f);
    glEnd();


    for (int i = 0; i < pointList.size(); i++) {
        GLUquadric* quad = gluNewQuadric();
        if (displayPoints) {
            // Draws spheres to represent points, with catmull splines where they can be computed.
            glColor3f(0.0, 0.6, 0.0);
            glPushMatrix();
            glTranslatef(pointList[i].x(), pointList[i].y(), pointList[i].z());
            gluSphere(quad, GLdouble(POINT_RADIUS), GLint(30), GLint(30));
            glPopMatrix(); // Applies the transform to the sphere without affecting the lines.
        }

        // This section draws the "handles" for selected points
        if (i == selectedPoint) {
            // Lines to denote axis of movement.
            glLineWidth(POINT_HANDLE_WIDTH);
            glColor3f(0.0, 0.0, 1.0);
            glBegin(GL_LINES);
            glVertex3f(pointList[i].x() + POINT_RADIUS, pointList[i].y(), pointList[i].z());
            glVertex3f(pointList[i].x() + POINT_HANDLE_LENGTH, pointList[i].y(), pointList[i].z());
            glVertex3f(pointList[i].x(), pointList[i].y() + POINT_RADIUS, pointList[i].z());
            glVertex3f(pointList[i].x(), pointList[i].y() + POINT_HANDLE_LENGTH, pointList[i].z());
            glVertex3f(pointList[i].x(), pointList[i].y(), pointList[i].z() + POINT_RADIUS);
            glVertex3f(pointList[i].x(), pointList[i].y(), pointList[i].z() + POINT_HANDLE_LENGTH);
            glEnd();

            // Spheres at the end of the handles
            glPushMatrix();
            glTranslatef(pointList[i].x() + POINT_RADIUS*2, pointList[i].y(), pointList[i].z());
            gluSphere(quad, GLdouble(POINT_RADIUS/2), GLint(30), GLint(30));
            glPopMatrix();
            glPushMatrix();
            glTranslatef(pointList[i].x(), pointList[i].y() + POINT_RADIUS*2, pointList[i].z());
            gluSphere(quad, GLdouble(POINT_RADIUS/2), GLint(30), GLint(30));
            glPopMatrix();
            glPushMatrix();
            glTranslatef(pointList[i].x(), pointList[i].y(), pointList[i].z() + POINT_RADIUS*2);
            gluSphere(quad, GLdouble(POINT_RADIUS/2), GLint(30), GLint(30));
            glPopMatrix();
        }

        // Set colour for the spline line
        glColor3f(0.0, 1.0, 0.0);
        glLineWidth(3);
        // draw the catmull spline between the points
        if (i > 2) {
            QVector3D prevPoint = pointList[i-2];
            for (int j = 0; j <= CATMULL_FIDELITY; j++) {
                float t = (float)j/CATMULL_FIDELITY;
                QVector3D p0 = pointList[i-3];
                QVector3D p1 = pointList[i-2];
                QVector3D p2 = pointList[i-1];
                QVector3D p3 = pointList[i];

                // Algorithm to calculate the point on the catmull rom spline
                QVector3D nextPoint = 0.5 * ((2*p1) + (p0*(-1) + p2)*t + (2*p0 - 5*p1 + 4*p2 - p3)*t*t + (p0*(-1) + 3*p1 - 3*p2 + p3)*t*t*t);
                glBegin(GL_LINES);
                glVertex3f(prevPoint.x(), prevPoint.y(), prevPoint.z());
                glVertex3f(nextPoint.x(), nextPoint.y(), nextPoint.z());
                glEnd();
                prevPoint = nextPoint;
            }
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

    helpString = "Controls:\n\nLeft mouse button rotates the camera, selects points, and moves points.";
    helpString += "\n\nRight mouse button controls zoom and deletes points.";
    helpString += "\n\nMiddle mouse button adds points.";
    QMessageBox::information( this, title, helpString, QMessageBox::Ok );
}

void GLWidget::clearPoints()
{
    selectedPoint = -1;
    pointList.clear();
    clear();
}

void GLWidget::togglePoints()
{
    selectedPoint = -1;
    displayPoints = !displayPoints;
    updateGL();
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

QVector3D GLWidget::convertWindowToWorld(float x, float y, float z)
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

    return QVector3D(worldX, worldY, worldZ);
}

int GLWidget::nearestPointToRay(int mouseX, int mouseY) {
    int nearestPoint = -1;
    float smallestT = -1;
    for (int i = 0; i < pointList.size(); i++) {
        QVector3D cameraRayStart = convertWindowToWorld(mouseX, mouseY, 0.0);
        QVector3D cameraRayDirection = convertWindowToWorld(mouseX, mouseY, 1.0);

        QVector3D rayOriginMinusSphereCenter = cameraRayStart - pointList[i];

        float partA = cameraRayDirection.lengthSquared();
        float partB = QVector3D::dotProduct(cameraRayDirection, rayOriginMinusSphereCenter);
        float partC = rayOriginMinusSphereCenter.lengthSquared() - (POINT_RADIUS * POINT_RADIUS);

        float discriminant = (partB * partB) - (partA* partC);

        if (discriminant >= 0) {
            float t = (-partB - discriminant) / partA;
            // Calculate the smaller T value and test it against smallestT
            if (t < smallestT || smallestT == -1) {
                smallestT = t;
                nearestPoint = i;
            }
        }
    }
    return nearestPoint;
}

void GLWidget::mousePressEvent( QMouseEvent *e )
{
    if (e->button() == Qt::LeftButton)
    {
        if (selectedPoint >= 0) { // If there is a selected point, we have to see if the user is moving it.
            QVector3D cameraRayStart = convertWindowToWorld(e->pos().x(), e->pos().y(), 0.0);
            QVector3D cameraRayDirection = convertWindowToWorld(e->pos().x(), e->pos().y(), 1.0);
            for (int i = 0; i < 3; i++) {
                QVector3D sphereCenter = QVector3D(pointList[selectedPoint]);
                switch (i) {
                    case 0:
                        sphereCenter += QVector3D(POINT_RADIUS*2, 0, 0);
                        break;
                    case 1:
                        sphereCenter += QVector3D(0, POINT_RADIUS*2, 0);
                        break;
                    case 2:
                        sphereCenter += QVector3D(0, 0, POINT_RADIUS*2);
                        break;
                    default:
                        break;
                }

                QVector3D rayOriginMinusSphereCenter = cameraRayStart - sphereCenter;

                float partA = cameraRayDirection.lengthSquared();
                float partB = QVector3D::dotProduct(cameraRayDirection, rayOriginMinusSphereCenter);
                float partC = rayOriginMinusSphereCenter.lengthSquared() - (POINT_RADIUS * POINT_RADIUS)/2;

                float discriminant = (partB * partB) - (partA* partC);

                if (discriminant >= 0) {
                    switch (i) {
                    case 0:
                        dragAxis = X;
                        break;
                    case 1:
                        dragAxis = Y;
                        break;
                    case 2:
                        dragAxis = Z;
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        if (dragAxis == NONE) {
            Rotating = true;
        }
        lastMousePoint = e->pos();
        initialMousePoint = e->pos();
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
    if (e->button() == Qt::LeftButton)
    {
        float x = initialMousePoint.x() - e->pos().x();
        float y = initialMousePoint.y() - e->pos().y();
        float pointDistanceSquared = x*x + y*y;
        if (pointDistanceSquared > MIN_MOUSE_MOVE && Rotating) {
            DoRotate(e->pos(), lastMousePoint);
            Rotating = false;
        } else if (pointDistanceSquared < MIN_MOUSE_MOVE) {
            // If movement is insignificant, ray trace for point to select.
            if (displayPoints)
                selectedPoint = nearestPointToRay(e->pos().x(), e->pos().y());
        }
        dragAxis = NONE;
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
            //  If movement is insignificant, test ray against each object.
            int nearestPoint = nearestPointToRay(e->pos().x(), e->pos().y());
            if (nearestPoint >= 0) {
                pointList.remove(nearestPoint);
            }
        }
    }

    updateGL();
}

void GLWidget::mouseMoveEvent( QMouseEvent *e )
{
    if ((e->buttons() & Qt::LeftButton) )
    {
        if (Rotating) {
            DoRotate(e->pos(), lastMousePoint);
        } else if (dragAxis != NONE && selectedPoint != -1) {
            DoDrag(e->pos(), lastMousePoint);
        }
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

// Moves pointList[selectedPoint] by desc.axis - orig.axis in whichever axis is selected.
void GLWidget::DoDrag(QPoint desc, QPoint orig)
{
    // Need to get distance to the point
    QVector3D newPosition = convertWindowToWorld(desc.x(), desc.y(), 0.1);
    QVector3D prevPosition = convertWindowToWorld(orig.x(), orig.y(), 0.1);
    QVector3D distance = newPosition - prevPosition;
    distance *= 1.5;

    switch (dragAxis) {
        case X:
            pointList[selectedPoint] += QVector3D(distance.x(), 0, 0);
            break;
        case Y:
            pointList[selectedPoint] += QVector3D(0, distance.y(), 0);
            if (pointList[selectedPoint].y() < POINT_RADIUS)
                pointList[selectedPoint].setY(POINT_RADIUS);
            break;
        case Z:
            pointList[selectedPoint] += QVector3D(0, 0, distance.z());
            break;
        default:
            break;
    }
}
