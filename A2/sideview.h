#ifndef SIDEVIEW_H
#define SIDEVIEW_H

#include <QDialog>

namespace Ui {
class sideview;
}

class sideview : public QDialog
{
    Q_OBJECT
    
public:
    explicit sideview(QWidget *parent = 0);
    ~sideview();
    
private:
    Ui::sideview *ui;
};

#endif // SIDEVIEW_H
