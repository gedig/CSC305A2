//-------------------------------------------------------------------------------------------
//  University of Bath Computer Science Department
//	FrameWork for OpenGL application under QT
//  Course title: Introduction Computer Graphics
//-------------------------------------------------------------------------------------------
#ifndef WINDOW_H
#define WINDOW_H

#include <QDialog>
#include <QColorDialog>
#include "ui_MainWindow.h"
#include "glwidget.h"

class GLWidget;

//Create a window and load the components from generated code ui_MainWindow.h
class Window : public QDialog, public Ui::frmMain
{
	Q_OBJECT;

public:
	//Constructor 
	Window(QWidget *parent = 0);
	
private:
        GLWidget *m_glWidget;


    private slots:

        void endit(){cerr << "goodbye\n"; exit(0);};
        void clear();
        void helpBut();

        void on_pushButton_2_clicked();

        void on_pushButton_3_clicked();

        void on_viewSelect_activated(int index);

        void on_pushButton_clicked();

        void on_playButton_clicked();

        void on_toggleCylinderButton_clicked();

protected:
         void  resizeEvent(QResizeEvent * );

};


#endif
