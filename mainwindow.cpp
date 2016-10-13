#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "vectorAdd.h"

//---------------------------------------------------------------------------
//  MainWindow
//---------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

//---------------------------------------------------------------------------
//  ~MainWindow
//---------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}

//---------------------------------------------------------------------------
//  on_pushButton_clicked
//---------------------------------------------------------------------------
void MainWindow::on_pushButton_clicked()
{
    int ret = vectorAdd();
}
