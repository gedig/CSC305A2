//-------------------------------------------------------------------------------------------
//  This is a demo. program for open gl vertex arrays
//-------------------------------------------------------------------------------------------
#include <QtGui>
#include <QtOpenGL>
#include "window.h"
//#include "dfuncs.h"

#define BIT0 0x00000001   // alpha
#define BIT1 0x00000002   // no buttons
#define BIT2 0x00000004   // set don't use native dialog

//extern dfuncs *df;

//------------------------------------------------------------------------------------
// Creates and initializes the main window for application
//------------------------------------------------------------------------------------
Window::Window(QWidget *parent):QDialog(parent)
{
    //We create an instance of GLWidget component we built in glwidget.h
    m_glWidget = new GLWidget;

    //Setup application interface. Creates all the required components and sliders.
    setupUi(this);

    //We need to attach our m_glWidget to glWidgetArea
    //All our drawings will be on glWidgetArea
    glWidgetArea->setWidget(m_glWidget);
}

void Window::resizeEvent( QResizeEvent * )
{
  cerr << "new size "<< width() SEP height() NL;
}

void Window::clear()
{
    m_glWidget->clear();
}

void Window::helpBut()
{
    m_glWidget->help();
}

void Window::on_pushButton_2_clicked()
{
    m_glWidget->clearPoints();
}

void Window::on_pushButton_3_clicked()
{
    m_glWidget->togglePoints();
}

void Window::on_viewSelect_activated(int index)
{
    m_glWidget->toggleOrtho(index);
}

void Window::on_pushButton_clicked()
{
    m_glWidget->toggleAnim();
}

void Window::on_playButton_clicked()
{
    m_glWidget->playPauseAnim();
}

void Window::on_toggleCylinderButton_clicked()
{
    m_glWidget->toggleCylinder();
}

void Window::on_cylinderShapeSelect_currentIndexChanged(int index)
{
    m_glWidget->changeCylinderShape(index);
}
