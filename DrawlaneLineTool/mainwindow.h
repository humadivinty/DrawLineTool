#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"cameraModule/Camera6467_plate.h"
#include<QDomNode>

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

    void on_pushButton_LoadConfig_clicked();

    void on_pushButton_loadxmlToGUI_clicked();

    void on_pushButton_modifXML_clicked();

    bool GetNodeValue(QDomNode& node, QString attributeName, QString& value);

    void on_pushButton_loadLocalImg_clicked();

private:
    Ui::MainWindow *ui;

    Camera6467_plate* m_pCamera;

    char m_chLogBuf[10240];
    void CustWriteFormatLog(const char* szfmt, ...);

    QString m_strLoadedXML;

public:
    static void JPEG_Arrive(void* pUser, int Statuse);

signals:
    void JpegArrive();
public slots:
    void GetJpegImg();
};

#endif // MAINWINDOW_H
