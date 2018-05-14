#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "worker.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void pricessFrameAndUpdateGUI(QPixmap imageOriginal);

private slots:
    void on_pushButtonCamera_clicked();
    void on_pushButtonVideo_clicked();
    void on_pushButtonStop_clicked();
    void on_pushButtonCUDA_clicked();

private:
    Ui::MainWindow *ui;
    QPixmap         m_Image;

    //-----------------------------------------------------------------------
    //  @brief Thread object which will let us manipulate the running thread
    //-----------------------------------------------------------------------
    QThread*        thread;

    //-----------------------------------------------------------------------
    //  @brief Object which contains methods that should be runned in another thread
    //-----------------------------------------------------------------------
    Worker*         worker;

};

#endif // MAINWINDOW_H
