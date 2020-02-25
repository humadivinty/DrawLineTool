#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"cameraModule/Camera6467_plate.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_Connect_clicked();

    void on_pushButton_DownLoadConfig_clicked();

    void on_pushButton_CloseConnect_clicked();

    void on_pushButton_upLoad_clicked();

    void on_pushButton_loadImg_clicked();

private:
    Ui::MainWindow *ui;

    Camera6467_plate* m_pCamera;

    char m_chLogBuf[10240];
    void CustWriteFormatLog(const char* szfmt, ...);
};

#endif // MAINWINDOW_H
