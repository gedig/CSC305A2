#include "sideview.h"
#include "ui_sideview.h"

sideview::sideview(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::sideview)
{
    ui->setupUi(this);
}

sideview::~sideview()
{
    delete ui;
}
