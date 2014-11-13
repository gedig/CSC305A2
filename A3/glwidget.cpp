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
#define CATMULL_FIDELITY 25
#define CIRCLE_QUALITY 20

const double RadPerPixel = - 0.01;
const double MovePerPixel = - 0.1;

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent),
      displayPoints(true),
      displayCylinder(true),
      cubeOn(false),
      animPlay(true),
      currentCubeFrame(0)
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
    CameraPos.x = CameraPos.y = CameraPos.z = 5;
    Rotating = false;
    dragAxis = NONE;
    currentPerspective = P;
    currentCylinderShape = SQUARE;
    changeCylinderShape(SQUARE);
    Scaling = false;
    selectedPoint = -1;

    animTimer = new QTimer(this);
    connect(animTimer, SIGNAL(timeout()), this, SLOT(incrementCubePos()));
    // animTimer is started when the animation is started.
}

QVector3D GLWidget::crossSectionToWorld(QVector3D vec, float mat[3][3])
{   
    QVector3D temp;
    temp.setX(vec.x() * mat[0][0] + vec.y() * mat[1][0] + mat[2][0]);
    temp.setY(vec.x() * mat[0][1] + vec.y() * mat[1][1] + mat[2][1]);
    temp.setZ(vec.x() * mat[0][2] + vec.y() * mat[1][2] + mat[2][2]);
    return temp;
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
    glEnable( GL_DEPTH_TEST );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
}

void GLWidget::redraw()
{
        updateGL();
}

void GLWidget::paintGL()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    switch (currentPerspective) {
        case P:
        gluLookAt(CameraPos.x,CameraPos.y, CameraPos.z, // Eye
                  0, 0, 0, // Center
                  0.0, 1.0, 0.0); // Up
        break;
    case XPOS:
        gluLookAt(3, 1, 0, // Eye
                  0, 1, 0, // Center
                  0.0, 1.0, 0.0); // Up
        break;
    case XNEG:
        gluLookAt(-3, 1, 0, // Eye
                  0, 1, 0, // Center
                  0.0, 1.0, 0.0); // Up
        break;
    case ZPOS:
        gluLookAt(0, 1, 3, // Eye
                  0, 1, 0, // Center
                  0.0, 1.0, 0.0); // Up
        break;
    case ZNEG:
        gluLookAt(0, 1, -3, // Eye
                  0, 1, 0, // Center
                  0.0, 1.0, 0.0); // Up
        break;
    default:
        qDebug() << "We should never get here, using this in lieu of assert";
        exit(1);
    }

    // Draws the xz plane
    glBegin(GL_QUADS);
    glColor3f(0.6, 0.6, 1.0);
    glVertex3f( -PLANE_SIZE, -0.1, -PLANE_SIZE);
    glVertex3f( -PLANE_SIZE, -0.1, PLANE_SIZE);
    glVertex3f( PLANE_SIZE, -0.1, PLANE_SIZE);
    glVertex3f( PLANE_SIZE, -0.1, -PLANE_SIZE);
    glEnd();

    // Draws the grid on the xz plane.
    glColor3f(.2, .2, .2);
    glBegin(GL_LINES);
    for(int i = -PLANE_SIZE; i <= PLANE_SIZE; i += GRID_DISTANCE) {
        if (i == 0) {
            glColor3f(1.0, 0, 0);
            glVertex3f(i,0,-PLANE_SIZE);
            glVertex3f(i,0,PLANE_SIZE);
            glColor3f(0, 0, 1.0);
            glVertex3f(-PLANE_SIZE,0,i);
            glVertex3f(PLANE_SIZE,0,i);
            glColor3f(.2, .2, .2);
        } else {
            glVertex3f(i,0,-PLANE_SIZE);
            glVertex3f(i,0,PLANE_SIZE);
            glVertex3f(-PLANE_SIZE,0,i);
            glVertex3f(PLANE_SIZE,0,i);
        }
    };
    glEnd();

    // Draws the axis lines
    glLineWidth(3);
    glBegin(GL_LINES);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0f, 100.0f, 0.0f);
    glEnd();


    QVector3D prevN, prevB, prevP;
    int currentFrame = 0;
    totalFrames = 0;
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
            QVector3D p0 = pointList[i-3];
            QVector3D p1 = pointList[i-2];
            QVector3D p2 = pointList[i-1];
            QVector3D p3 = pointList[i];

            QVector3D P, V, Q;
            QVector3D T, N, B;
            if (i == 3) { // Very first frame on the catmull rom spline, this is where we define the base frenet frame
                P = p1;
                V = 0.5 * (-1*p0 + p2);
                Q = 2*p0 - 5*p1 + 4*p2 - p3;

                T = V.normalized();
                N = V.crossProduct(V, V.crossProduct(Q,V)).normalized();
                B = B.crossProduct(T,N).normalized();
                if (displayCylinder) {
                    glColor3f(0.0, 0.8, 0.2);
                    drawEndFrames(true, N, B, P);
                }
                prevN = N;
                prevB = B;
                prevP = P;
            }
            for (int j = 0; j <= CATMULL_FIDELITY; j++) {
                currentFrame++;
                float t = (float)j/CATMULL_FIDELITY;

                // Algorithm to calculate the point on the catmull rom spline
                QVector3D nextPoint = 0.5 * ((2*p1) + (p0*(-1) + p2)*t + (2*p0 - 5*p1 + 4*p2 - p3)*t*t + (p0*(-1) + 3*p1 - 3*p2 + p3)*t*t*t);

                P = nextPoint;
                V = 0.5 * (3*t*t*((-1)*p0 + 3*p1 - 3*p2 + p3) +2*t*(2*p0-5*p1 + 4*p2 - p3) + ((-1)*p0 + p2));
                Q = 0.5 * (6*t*((-1)*p0 + 3*p1 - 3*p2 + p3) + 2*(2*p0-5*p1 + 4*p2 - p3));

                T = V.normalized();
                N = N.crossProduct(T,prevB).normalized();
                B = B.crossProduct(T,N).normalized();

                //if (displayCylinder && j == CATMULL_FIDELITY) {
                if (displayCylinder) {
                    glColor3f(0.0, 0.8, 0.2);
                    drawFrenetFrame(prevN, prevB, prevP, N, B, P, currentFrame);
                }

                if (cubeOn && currentFrame == currentCubeFrame) {
                    // Draw the cube
                    glPushMatrix();
                    // Get rotation of Cube via frenet calculations
                    float cubeRotationDegree = T.dotProduct(QVector3D(1, 0, 0), T); //Angle of rotation is arccos(objectXAxis.T)
                    cubeRotationDegree = qAcos(cubeRotationDegree);
                    cubeRotationDegree *= 57.29578f; // Convert from radians to degrees
                    QVector3D cubeRotationAxis = T.crossProduct(QVector3D(1, 0, 0), T); // Rotation axis is objectXAxis crossproduct T
                    glTranslatef(P.x(), P.y(), P.z());
                    glScalef(0.3f, 0.3f, 0.3f);
                    glRotatef(cubeRotationDegree, cubeRotationAxis.x(), cubeRotationAxis.y(), cubeRotationAxis.z());
                    drawCube(true);
                    glPopMatrix(); // Applies the transform to the sphere without affecting the lines.
                    glColor3f(0.0, 1.0, 0.0);
                }

                glColor3f(0.0, 1.0, 0.0);
                glBegin(GL_LINES);
                glVertex3f(prevPoint.x(), prevPoint.y(), prevPoint.z());
                glVertex3f(nextPoint.x(), nextPoint.y(), nextPoint.z());
                glEnd();

                prevN = N;
                prevB = B;
                prevP = P;
                prevPoint = nextPoint;
            }
        }
    }
    totalFrames = currentFrame;
}

/* Draws a cube with sides of size 1 with center at 0,0,0.
    if changeColours is true, then the cube will have different colours on all sides.*/
void GLWidget::drawCube(bool changeColours)
{
    glBegin(GL_QUADS);
    if (changeColours)
        glColor3f(1.0, 0.6, 1.0);
    glVertex3f( 0.5f, 0.5f,-0.5f);    // Top Right Of The Quad (Top)
    glVertex3f(-0.5f, 0.5f,-0.5f);    // Top Left Of The Quad (Top)
    glVertex3f(-0.5f, 0.5f, 0.5f);    // Bottom Left Of The Quad (Top)
    glVertex3f( 0.5f, 0.5f, 0.5f);    // Bottom Right Of The Quad (Top)
    if (changeColours)
        glColor3f(1.0, 1.0, 0.0);

    glVertex3f( 0.5f,-0.5f, 0.5f);    // Top Right Of The Quad (Bottom)
    glVertex3f(-0.5f,-0.5f, 0.5f);    // Top Left Of The Quad (Bottom)
    glVertex3f(-0.5f,-0.5f,-0.5f);    // Bottom Left Of The Quad (Bottom)
    glVertex3f( 0.5f,-0.5f,-0.5f);    // Bottom Right Of The Quad (Bottom)
    if (changeColours)
        glColor3f(1.0, 0.5, 0.0);

    glVertex3f( 0.5f, 0.5f, 0.5f);    // Top Right Of The Quad (Front)
    glVertex3f(-0.5f, 0.5f, 0.5f);    // Top Left Of The Quad (Front)
    glVertex3f(-0.5f,-0.5f, 0.5f);    // Bottom Left Of The Quad (Front)
    glVertex3f( 0.5f,-0.5f, 0.5f);    // Bottom Right Of The Quad (Front)
    if (changeColours)
        glColor3f(1.0, 0.0, 0.0);

    glVertex3f( 0.5f,-0.5f,-0.5f);    // Top Right Of The Quad (Back)
    glVertex3f(-0.5f,-0.5f,-0.5f);    // Top Left Of The Quad (Back)
    glVertex3f(-0.5f, 0.5f,-0.5f);    // Bottom Left Of The Quad (Back)
    glVertex3f( 0.5f, 0.5f,-0.5f);    // Bottom Right Of The Quad (Back)
    if (changeColours)
        glColor3f(0.0, 0.0, 1.0);

    glVertex3f(-0.5f, 0.5f, 0.5f);    // Top Right Of The Quad (Left)
    glVertex3f(-0.5f, 0.5f,-0.5f);    // Top Left Of The Quad (Left)
    glVertex3f(-0.5f,-0.5f,-0.5f);    // Bottom Left Of The Quad (Left)
    glVertex3f(-0.5f,-0.5f, 0.5f);    // Bottom Right Of The Quad (Left)

    glVertex3f( 0.5f, 0.5f,-0.5f);    // Top Right Of The Quad (Right)
    glVertex3f( 0.5f, 0.5f, 0.5f);    // Top Left Of The Quad (Right)
    glVertex3f( 0.5f,-0.5f, 0.5f);    // Bottom Left Of The Quad (Right)
    glVertex3f( 0.5f,-0.5f,-0.5f);    // Bottom Right Of The Quad (Right) - See more at: http://www.codemiles.com/c-opengl-examples/draw-3d-cube-using-opengl-t9018.html#sthash.DpmpkUoa.dpuf
    glEnd();
}

void GLWidget::drawEndFrames(bool first, QVector3D N, QVector3D B, QVector3D P)
{
    // If not first frame, draw quads between this frame and the previous frame
    float matrix[3][3];
    matrix[0][0] = N.x();
    matrix[0][1] = N.y();
    matrix[0][2] = N.z();
    matrix[1][0] = B.x();
    matrix[1][1] = B.y();
    matrix[1][2] = B.z();
    matrix[2][0] = P.x();
    matrix[2][1] = P.y();
    matrix[2][2] = P.z();

    // TODO-DG: Draw the shape in 2D here including fill.
//    QVector3D c0 = crossSectionToWorld(crossSectionCoords[0], matrix);
//    QVector3D c1 = crossSectionToWorld(crossSectionCoords[1], matrix);
//    QVector3D c2 = crossSectionToWorld(crossSectionCoords[2], matrix);
//    QVector3D c3 = crossSectionToWorld(crossSectionCoords[3], matrix);

//    glBegin(GL_QUADS);
//    glVertex3f(c0.x(), c0.y(), c0.z());
//    glVertex3f(c1.x(), c1.y(), c1.z());
//    glVertex3f(c2.x(), c2.y(), c2.z());
//    glVertex3f(c3.x(), c3.y(), c3.z());
//    glEnd();
}

void GLWidget::drawFrenetFrame(QVector3D prevN, QVector3D prevB, QVector3D prevP, QVector3D N, QVector3D B, QVector3D P, int frame)
{
    // draw quads between this frame and the previous frame
    float prevMatrix[3][3];
    prevMatrix[0][0] = prevN.x();
    prevMatrix[0][1] = prevN.y();
    prevMatrix[0][2] = prevN.z();
    prevMatrix[1][0] = prevB.x();
    prevMatrix[1][1] = prevB.y();
    prevMatrix[1][2] = prevB.z();
    prevMatrix[2][0] = prevP.x();
    prevMatrix[2][1] = prevP.y();
    prevMatrix[2][2] = prevP.z();
    float matrix[3][3];
    matrix[0][0] = N.x();
    matrix[0][1] = N.y();
    matrix[0][2] = N.z();
    matrix[1][0] = B.x();
    matrix[1][1] = B.y();
    matrix[1][2] = B.z();
    matrix[2][0] = P.x();
    matrix[2][1] = P.y();
    matrix[2][2] = P.z();

    // DRAW QUADS BETWEEN PREVIOUS 2D POINTS AND THESE 2D POINTS FOR EACH LINE IN THE 2D DIAGRAM
    for (int i = 0; i < crossSectionCoords.size(); i++) {
        QVector3D currentPoint = crossSectionCoords[i];
        QVector3D p0, p1, p2, p3;
        if (currentPoint.z() != 0) {
            QVector3D prevPoint = crossSectionCoords[(i+crossSectionCoords.size()-1)%crossSectionCoords.size()];
            if (frame % 2 != 0) {
                p0 = crossSectionToWorld(prevPoint, prevMatrix);
                 p1 = crossSectionToWorld(currentPoint, prevMatrix);
                 p2 = crossSectionToWorld(-1*currentPoint, matrix);
                 p3 = crossSectionToWorld(-1*prevPoint, matrix);
            } else {
                p0 = crossSectionToWorld(-1*prevPoint, prevMatrix);
                p1 = crossSectionToWorld(-1*currentPoint, prevMatrix);
                p2 = crossSectionToWorld(currentPoint, matrix);
                p3 = crossSectionToWorld(prevPoint, matrix);
            }

            glBegin(GL_QUADS);
            glVertex3f(p0.x(), p0.y(), p0.z());
            glVertex3f(p1.x(), p1.y(), p1.z());
            glVertex3f(p2.x(), p2.y(), p2.z());
            glVertex3f(p3.x(), p3.y(), p3.z());
            glVertex3f(p3.x(), p3.y(), p3.z());
            glVertex3f(p2.x(), p2.y(), p2.z());
            glVertex3f(p1.x(), p1.y(), p1.z());
            glVertex3f(p0.x(), p0.y(), p0.z());
            glEnd();
        }
    }
}

/* 2D */
void GLWidget::resizeGL( int w, int h )
{
    currentPerspective = P;
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

void GLWidget::toggleOrtho(int index)
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    switch (index) {
    case 0:
        currentPerspective = P;
        glFrustum( -1.0, 1.0, -1.0, 1.0, 5.0, 1500.0 );
        break;
    case 1:
        currentPerspective = XPOS;
        break;
    case 2:
        currentPerspective = XNEG;
        break;
    case 3:
        currentPerspective = ZPOS;
        break;
    case 4:
        currentPerspective = ZNEG;
        break;
    }

    if (index > 0) {
        glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -5.0f, 100.0f);
    }
    glMatrixMode( GL_MODELVIEW );
    updateGL();
}
void GLWidget::toggleAnim()
{
    if (cubeOn) {
        animTimer->stop();
        cubeOn = false;
    } else {
        cubeOn = true;
        if (animPlay)
            animTimer->start(100);
    }
    updateGL();
}

void GLWidget::playPauseAnim()
{
    if (animPlay) {
        animPlay = false;
        animTimer->stop();
    } else {
        animPlay = true;
        animTimer->start(100);
    }
}

void GLWidget::toggleCylinder()
{
    displayCylinder = !displayCylinder;
    updateGL();
}

void GLWidget::changeCylinderShape(int shape)
{
    crossSectionCoords.clear();
    float PI2 = 6.283185;
    switch(shape) {
    case SQUARE:
        crossSectionCoords.append(QVector3D(-0.3f,-0.3f, 1));
        crossSectionCoords.append(QVector3D(-0.3f, 0.3f, 1));
        crossSectionCoords.append(QVector3D(0.3f, 0.3f, 1));
        crossSectionCoords.append(QVector3D(0.3f, -0.3f, 1));
        break;
    case CIRCLE:
        for ( int i = CIRCLE_QUALITY; i > 0; i--) {
            float radiusStep = (float)i / (float)CIRCLE_QUALITY;
            float x = 0.42 * cos(PI2 * radiusStep);
            float y = 0.42 * sin(PI2 * radiusStep);
            crossSectionCoords.append(QVector3D(x, y, 1));
        }
        break;
    case STAR:
        crossSectionCoords.append(QVector3D(0, 0.42f, 1));
        crossSectionCoords.append(QVector3D(0.1f, 0.1f, 1));
        crossSectionCoords.append(QVector3D(0.4f, 0.1f, 1));
        crossSectionCoords.append(QVector3D(0.2f, -0.1f, 1));
        crossSectionCoords.append(QVector3D(0.3f, -0.4f, 1));
        crossSectionCoords.append(QVector3D(0, -0.2f, 1));
        crossSectionCoords.append(QVector3D(-0.3f, -0.4f, 1));
        crossSectionCoords.append(QVector3D(-0.2f, -0.1f, 1));
        crossSectionCoords.append(QVector3D(-0.4f, 0.1f, 1));
        crossSectionCoords.append(QVector3D(-0.1f, 0.1f, 1));
        break;
    case LINE:
        crossSectionCoords.append(QVector3D(0, 0.42f, 1));
        crossSectionCoords.append(QVector3D(0.0f, 0.1f, 1));
        break;
    case CUSTOM:
        // TODO-DG: Pop up a window and allow the user to draw shapes to it.
        break;
    }
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

// This function tests a ray against the objects in pointList and returns the index
// of the closest intersecting point.
int GLWidget::nearestPointToRay(int mouseX, int mouseY) {
    int nearestPoint = -1;
    float smallestT = -1;
    for (int i = 0; i < pointList.size(); i++) {
        QVector3D cameraRayStart = convertWindowToWorld(mouseX, mouseY, -0.1);
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
        if (dragAxis == NONE && currentPerspective == P) {
            Rotating = true;
        }
        lastMousePoint = e->pos();
        initialMousePoint = e->pos();
    } else if (e->button() == Qt::RightButton) {
        lastMousePoint = e->pos();
        initialMousePoint = e->pos();
        if (currentPerspective == P)
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
        if (pointDistanceSquared > MIN_MOUSE_MOVE && Rotating && currentPerspective == P) {
            DoRotate(e->pos(), lastMousePoint);
            Rotating = false;
        } else if (pointDistanceSquared < MIN_MOUSE_MOVE) {
            // If movement is insignificant, ray trace for point to select.
            if (displayPoints) {
                selectedPoint = nearestPointToRay(e->pos().x(), e->pos().y());
                Rotating = false;
            }
        }
        dragAxis = NONE;
    }

    if (e->button() == Qt::RightButton && Scaling && currentPerspective == P)
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

void GLWidget::incrementCubePos()
{
    currentCubeFrame++;
    if (currentCubeFrame > totalFrames)
        currentCubeFrame = 0;

    updateGL();
}
