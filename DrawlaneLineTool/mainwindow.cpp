#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDebug>
#include<QMessageBox>
#include<QFileDialog>
#include<QImage>


#include"utilityTool/tool_function.h"



#define QWRITE_LOG(fmt,...) CustWriteFormatLog("%s:: " fmt, __FUNCTION__,##__VA_ARGS__);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_pCamera(NULL)
{
    ui->setupUi(this);
    memset(m_chLogBuf, '\0', sizeof(m_chLogBuf));
}

MainWindow::~MainWindow()
{
    if(m_pCamera)
    {
       delete m_pCamera;
        m_pCamera = NULL;
    }

    delete ui;
}

void MainWindow::on_pushButton_Connect_clicked()
{
    QString strIP = ui->lineEdit->text();

    if(m_pCamera)
    {
        m_pCamera->DisConnectCamera();
    }
    else
    {
        m_pCamera = new Camera6467_plate();
    }
    QMessageBox msgBox;

    m_pCamera->SetCameraIP(strIP.toStdString().c_str());
    int iRet = m_pCamera->ConnectToCameraWithoutResult();
    if(0 == iRet)
    {
        QWRITE_LOG("ConnectToCameraWithoutResult success.");
        msgBox.setText(QString("ConnectToCameraWithoutResult %1 success.").arg(strIP));
    }
    else
    {
        QWRITE_LOG("ConnectToCameraWithoutResult failed, code = %d.", iRet);
        msgBox.setText(QString("ConnectToCameraWithoutResult %1 failed.").arg(strIP));
    }

    msgBox.exec();
}

void MainWindow::CustWriteFormatLog(const char *szfmt, ...)
{
    memset(m_chLogBuf, '\0', sizeof(m_chLogBuf));
    va_list arg_ptr;
    va_start(arg_ptr, szfmt);
    _vsnprintf(m_chLogBuf,  sizeof(m_chLogBuf),  szfmt, arg_ptr);
    va_end(arg_ptr);

    qDebug()<<m_chLogBuf;
}

void MainWindow::on_pushButton_DownLoadConfig_clicked()
{
     QMessageBox msgBox;
     QString strFileName = ui->lineEdit->text();
     strFileName.append(".xml");

    if(m_pCamera)
    {
        std::string strXML = m_pCamera->GetHarWareConfig();
        if(!strXML.empty())
        {
            Tool_SaveFile((void*)strXML.c_str(), strXML.length(), strFileName.toStdString().c_str());

            msgBox.setText(QString("download xml config file %1 success.").arg(strFileName));
        }
        else
        {
            QWRITE_LOG("ge config from camera failed.");
            msgBox.setText(QString("download xml config file %1 failed.").arg(strFileName));
        }
    }
    else
    {
        QWRITE_LOG("camera is not init.");
        msgBox.setText(QString("camera is not init..").arg(strFileName));
    }
    msgBox.exec();
}

void MainWindow::on_pushButton_CloseConnect_clicked()
{
    if(m_pCamera)
    {
        m_pCamera->DisConnectCamera();
    }
    QMessageBox msgBox;
    msgBox.setText("camer close finish.");
    msgBox.exec();
}

void MainWindow::on_pushButton_upLoad_clicked()
{
    QMessageBox msgBox;
    if(NULL == m_pCamera)
    {
        msgBox.setText("please connect camera at first.");
        msgBox.exec();
        return;
    }
    QString strLog;

    QString strFileName = QFileDialog::getOpenFileName(this, "choose file", "*.xml");
    qDebug()<<strFileName;

    size_t ibufLength = 1024*1024;
    char* chFileBuf = new char[ibufLength];
    memset(chFileBuf, '\0', ibufLength);
    if(Tool_LoadFile(strFileName.toStdString().c_str(), chFileBuf, ibufLength))
    {
        strLog = QString("load file %1 success.").arg(strFileName);
    }
    else
    {
        strLog = QString("load file %1 failed.").arg(strFileName);
        SAFE_DELETE_ARRAY(chFileBuf);
        return;
    }

    if( m_pCamera->UploadHarWareConfig(chFileBuf))
    {
        strLog = QString("upload %1 to camera %2 success").arg(strFileName).arg( ui->lineEdit->text());
        msgBox.setText(strLog);
    }
    else
    {
        strLog = QString("upload %1 to camera %2 failed").arg(strFileName).arg( ui->lineEdit->text());
        msgBox.setText(strLog);
    }
    msgBox.exec();
    SAFE_DELETE_ARRAY(chFileBuf);
    qDebug()<<strLog;
}

void MainWindow::on_pushButton_loadImg_clicked()
{
//    QPixmap image("123.jpg");
//    ui->label->setScaledContents(true);
//    ui->label->setPixmap(image);

    std::vector<LineSegment> lines = ui->label->GetCurrentLines();
    for(auto it = lines.begin();it != lines.end(); it++)
    {
        qDebug()<<"line "<<"("<<(*it).startPoint.x<<","<<(*it).startPoint.y<<")"
               <<"("<<(*it).endPoint.x<<","<<(*it).endPoint.y<<")";;
    }

    //bool GetOneJpegImg(CameraIMG &destImg);
//    QMessageBox msgBox;
//    if(NULL == m_pCamera)
//    {
//        msgBox.setText("please connect camera at first.");
//        msgBox.exec();
//        return;
//    }
//    QString strLog;
//    if(m_pCamera->SetJpegCallbackOnly())
//    {
//        strLog = QString("SetJpegCallback success");
//        CameraIMG tempIMG;
//        int iTryTime = 10;
//        bool bFind = false;
//        while(iTryTime--)
//        {
//            if(m_pCamera-> GetOneJpegImg(tempIMG))
//            {
//                bFind = true;
//                break;
//            }
//            usleep(50 * 1000);
//        }

//        if(bFind)
//        {
//            m_pCamera->InterruptionConnection();
//            QString strFileName = ui->lineEdit->text();
//            strFileName.append(".jpgl");
//            Tool_SaveFile(tempIMG.pbImgData, tempIMG.dwImgSize, strFileName.toStdString().c_str());

//            strLog = QString("get jpeg success.");
//        }
//        else
//        {
//            strLog = QString("get jpeg failed.");
//        }
//    }
//    else
//    {
//        strLog = QString("SetJpegCallback failed.");
//    }
//    qDebug()<<strLog;


}
