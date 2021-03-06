#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDebug>
#include<QMessageBox>
#include<QFileDialog>
#include<QImage>
#include<QDomDocument>
#include<QFile>
#include<iostream>
#include<stack>
#include<map>
#include<QTextStream>


#include"utilityTool/tool_function.h"
#include"cameraModule/CameraResult.h"


#define QWRITE_LOG(fmt,...) CustWriteFormatLog("%s:: " fmt, __FUNCTION__,##__VA_ARGS__);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_pCamera(NULL)
{
    ui->setupUi(this);
    memset(m_chLogBuf, '\0', sizeof(m_chLogBuf));
    connect(this, SIGNAL(JpegArrive()), this, SLOT(GetJpegImg()));
    ui->label->setStyleSheet("color:red;");
    setWindowTitle("Lane Line Setting Tool");
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

void MainWindow::JPEG_Arrive(void *pUser, int Statuse)
{
    qDebug()<<"JPEG_Arrive";
    if(pUser == NULL)
    {
        return;
    }
    MainWindow* pWindow = (MainWindow*)pUser;
    emit pWindow->JpegArrive();
}

void MainWindow::GetJpegImg()
{
    qDebug()<<"GetJpegImg";
    CameraIMG tempIMG;
    int iTryTime = 10;
    bool bFind = false;
    while(iTryTime--)
    {
        if(m_pCamera-> GetOneJpegImg(tempIMG))
        {
            bFind = true;
            break;
        }
        usleep(50 * 1000);
    }
    QString strLog;
    if(bFind)
    {
        m_pCamera->SetJpegReceiveMsgCallback(NULL, NULL);
        m_pCamera->InterruptionConnection();
        //        QString strFileName = ui->lineEdit->text();
        //        strFileName.append(".jpgl");
        //        Tool_SaveFile(tempIMG.pbImgData, tempIMG.dwImgSize, strFileName.toStdString().c_str());

        strLog = QString("get jpeg success.");
        QPixmap image;
        image.loadFromData(tempIMG.pbImgData, tempIMG.dwImgSize);
        ui->label->setScaledContents(true);
        ui->label->setPixmap(image);
    }
    else
    {
        strLog = QString("get jpeg failed.");
    }
    qDebug()<<strLog;
    ui->pushButton_loadImg->setEnabled(true);
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
    QMessageBox msgBox;
    if(NULL == m_pCamera)
    {
        msgBox.setText("please connect camera at first.");
        msgBox.exec();
        return;
    }
    QString strLog;
    m_pCamera->SetJpegReceiveMsgCallback((void*)JPEG_Arrive, this);
    if(m_pCamera->SetJpegCallbackOnly())
    {
        strLog = QString("SetJpegCallback success");

    }
    else
    {
        strLog = QString("SetJpegCallback failed.");
    }
    qDebug()<<strLog;
    ui->pushButton_loadImg->setEnabled(false);
}

void MainWindow::on_pushButton_LoadConfig_clicked()
{
    QMessageBox msgBox;
    QString strFileName = QFileDialog::getSaveFileName(this, tr("Save xml File"),
                                                       QDir::currentPath(),
                                                       tr("XML Files (*.xml)"));

    if(strFileName.isEmpty() || !strFileName.contains(".xml"))
    {
        QMessageBox::about(0, "warning", "set the right out put xml name.");
        return ;
    }

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

void MainWindow::on_pushButton_loadxmlToGUI_clicked()
{
    QString strFileName = QFileDialog::getOpenFileName(this, "choose file", "*.xml");

    QFile file(strFileName);
    if (!file.open(QIODevice::ReadOnly))
        return ;

    QDomDocument doc("cameraXML");
    if (!doc.setContent(&file))
    {
        file.close();
        return ;
    }
    file.close();

    std::map<int, LineSegment> LinesMap;
    m_strLoadedXML = doc.toString();

    QDomNodeList nodeList = doc.elementsByTagName("KEY");
    int iLength = nodeList.length();
    bool bFindCount = false;
    QString strValue;
    QString strAttributeName;
    int iLineCount = -1;
    int iLineNum = 0;
    int iFindCount = 0;
    bool bTopX = false;
    bool bTopY = false;
    bool bBotX = false;
    bool bBotY = false;
    while(iLength > 0)
    {
        strValue = "";
        QDomNode node = nodeList.at(iLength--);
        if(!bFindCount)
        {
            if(GetNodeValue( node, "RoadCount", strValue))
            {
                std::cout<<node.nodeName().toStdString() <<":"<< strValue.toStdString()<<endl;
                iLineCount = strValue.toInt();
                bFindCount = true;
                iLength = nodeList.length();
            }
        }
        else
        {
            if(iLineNum < iLineCount)
            {
                strAttributeName = QString("Line%1TopX").arg(iLineNum);
                if(!bTopX && GetNodeValue( node, strAttributeName, strValue))
                {
                    std::cout<<strAttributeName.toStdString() <<":"<< strValue.toStdString()<<endl;
                    iFindCount++;
                    iLength = nodeList.length();
                    bTopX = true;

                    auto it = LinesMap.find(iLineNum);
                    if(LinesMap.end() != it)
                    {
                        it->second.startPoint.x = strValue.toInt();
                    }
                    else
                    {
                        LineSegment line;
                        line.startPoint.x = strValue.toInt();
                        LinesMap[iLineNum] = line;
                    }
                }

                strAttributeName = QString("Line%1TopY").arg(iLineNum);
                if(!bTopY && GetNodeValue( node, strAttributeName, strValue))
                {
                    std::cout<<strAttributeName.toStdString() <<":"<< strValue.toStdString()<<endl;
                    iFindCount ++;
                    iLength = nodeList.length();
                    bTopY = true;

                    auto it = LinesMap.find(iLineNum);
                    if(LinesMap.end() != it)
                    {
                        it->second.startPoint.y = strValue.toInt();
                    }
                    else
                    {
                        LineSegment line;
                        line.startPoint.y = strValue.toInt();
                        LinesMap[iLineNum] = line;
                    }
                }

                strAttributeName = QString("Line%1BottomX").arg(iLineNum);
                if(!bBotX && GetNodeValue( node, strAttributeName, strValue))
                {
                    std::cout<<strAttributeName.toStdString() <<":"<< strValue.toStdString()<<endl;
                    iFindCount ++;
                    iLength = nodeList.length();
                    bBotX = true;

                    auto it = LinesMap.find(iLineNum);
                    if(LinesMap.end() != it)
                    {
                        it->second.endPoint.x = strValue.toInt();
                    }
                    else
                    {
                        LineSegment line;
                        line.endPoint.x = strValue.toInt();
                        LinesMap[iLineNum] = line;
                    }
                }

                strAttributeName = QString("Line%1BottomY").arg(iLineNum);
                if(!bBotY && GetNodeValue( node, strAttributeName, strValue))
                {
                    std::cout<<strAttributeName.toStdString() <<":"<< strValue.toStdString()<<endl;
                    iFindCount ++;
                    iLength = nodeList.length();
                    bBotY = true;

                    auto it = LinesMap.find(iLineNum);
                    if(LinesMap.end() != it)
                    {
                        it->second.endPoint.y = strValue.toInt();
                    }
                    else
                    {
                        LineSegment line;
                        line.endPoint.y = strValue.toInt();
                        LinesMap[iLineNum] = line;
                    }
                }
            }

            if(iLineNum > 0 && iLineNum >= iLineCount)
            {
                break;
            }

            if(iFindCount == 4)
            {
                iFindCount = 0;
                bTopX = false;
                bTopY = false;
                bBotX = false;
                bBotY = false;
                iLineNum++;
            }
        }

    }

    std::vector<LineSegment> lineVec;
    for(auto pair=LinesMap.begin();pair!=LinesMap.end();++pair)
    {
        std::cout << pair->first << " ---  ";
        LineSegment line = pair->second;
        std::cout << " topX:" <<line.startPoint.x <<
                     " topY:" << line.startPoint.y <<
                     " BotX:" << line.endPoint.x <<
                     " BotY:" << line.endPoint.y <<endl;
        lineVec.push_back(line);
    }
    ui->label->SetCurrentLines(lineVec);
}

void MainWindow::on_pushButton_modifXML_clicked()
{
    QString strFileName = QFileDialog::getSaveFileName(this, tr("Save xml File"),
                                                       QDir::currentPath(),
                                                       tr("XBEL Files (*.xml)"));

    if(strFileName.isEmpty() || !strFileName.contains(".xml"))
    {
        QMessageBox::about(0, "warning", "set the right out put xml name.");
        return ;
    }

    std::vector<LineSegment> lines = ui->label->GetCurrentLines();
    int iGUILineCounts = lines.size();
    if(iGUILineCounts <= 0)
    {
        QMessageBox::about(0, "warning", "not lines update.");
        return ;
    }

    QDomDocument doc("cameraXML");
    if (!doc.setContent(m_strLoadedXML))
    {
        QMessageBox::about(0, "warning", "set content failed.");
        return ;
    }


    std::map<int, LineSegment> LinesMap;
    int iMapIndex = 0;
    for(auto it = lines.begin();it != lines.end(); it++)
    {
        LineSegment tempLine;
        if(it->startPoint.y > it->endPoint.y)
        {
            tempLine.startPoint = it->endPoint;
            tempLine.endPoint = it->startPoint;
        }
        else
        {
            tempLine = *it;
        }
        LinesMap[iMapIndex++] = tempLine;
    }

    m_strLoadedXML = doc.toString();

    QDomNodeList nodeList = doc.elementsByTagName("KEY");
    int iLength = nodeList.length();
    bool bFindCount = false;
    QString strValue;
    QString strAttributeName;
    int iLineCount = -1;
    int iLineNum = 0;
    int iFindCount = 0;
    bool bTopX = false;
    bool bTopY = false;
    bool bBotX = false;
    bool bBotY = false;
    while(iLength > 0)
    {
        strValue = "";
        QDomNode node = nodeList.at(iLength--);
        if(!bFindCount)
        {
            if(GetNodeValue( node, "RoadCount", strValue))
            {
//                std::cout<<node.nodeName().toStdString() <<":"<< strValue.toStdString()<<endl;
//                iLineCount = strValue.toInt();
                bFindCount = true;
                iLength = nodeList.length();

                iLineCount = iGUILineCounts;
                node.firstChildElement("VALUE").firstChild().setNodeValue(QString("%1").arg(iGUILineCounts));
            }
        }
        else
        {
            if(iLineNum < iLineCount)
            {
                strAttributeName = QString("Line%1TopX").arg(iLineNum);
                if(!bTopX && GetNodeValue( node, strAttributeName, strValue))
                {
                    std::cout<<strAttributeName.toStdString() <<":"<< strValue.toStdString()<<endl;
                    iFindCount++;
                    iLength = nodeList.length();
                    bTopX = true;

                    auto it = LinesMap.find(iLineNum);
                    LineSegment line = it->second;
                    node.firstChildElement("VALUE").firstChild().setNodeValue(QString("%1").arg(round(line.startPoint.x*100)));
                }

                strAttributeName = QString("Line%1TopY").arg(iLineNum);
                if(!bTopY && GetNodeValue( node, strAttributeName, strValue))
                {
                    std::cout<<strAttributeName.toStdString() <<":"<< strValue.toStdString()<<endl;
                    iFindCount ++;
                    iLength = nodeList.length();
                    bTopY = true;

                    auto it = LinesMap.find(iLineNum);
                    LineSegment line = it->second;
                    node.firstChildElement("VALUE").firstChild().setNodeValue(QString("%1").arg(round(line.startPoint.y*100)));
                }

                strAttributeName = QString("Line%1BottomX").arg(iLineNum);
                if(!bBotX && GetNodeValue( node, strAttributeName, strValue))
                {
                    std::cout<<strAttributeName.toStdString() <<":"<< strValue.toStdString()<<endl;
                    iFindCount ++;
                    iLength = nodeList.length();
                    bBotX = true;

                    auto it = LinesMap.find(iLineNum);
                    LineSegment line = it->second;
                    node.firstChildElement("VALUE").firstChild().setNodeValue(QString("%1").arg(round(line.endPoint.x*100)));
                }

                strAttributeName = QString("Line%1BottomY").arg(iLineNum);
                if(!bBotY && GetNodeValue( node, strAttributeName, strValue))
                {
                    std::cout<<strAttributeName.toStdString() <<":"<< strValue.toStdString()<<endl;
                    iFindCount ++;
                    iLength = nodeList.length();
                    bBotY = true;

                    auto it = LinesMap.find(iLineNum);
                    LineSegment line = it->second;
                    node.firstChildElement("VALUE").firstChild().setNodeValue(QString("%1").arg(round(line.endPoint.y*100)));
                }
            }

            if(iLineNum > 0 && iLineNum >= iLineCount)
            {
                break;
            }

            if(iFindCount == 4)
            {
                iFindCount = 0;
                bTopX = false;
                bTopY = false;
                bBotX = false;
                bBotY = false;
                iLineNum++;
            }
        }

    }

    QString finalXml = doc.toString();
    QFile fileFinal(strFileName);
    if (!fileFinal.open(QIODevice::WriteOnly|QIODevice::Text))
        return ;
    QTextStream in(&fileFinal);
    in<<finalXml;
    fileFinal.close();
}

bool MainWindow::GetNodeValue(QDomNode &node, QString attributeName, QString &value)
{
    if(!node.isNull()
            && node.isElement()
            && node.toElement().hasAttribute("name")
            && node.toElement().attribute("name") == attributeName)
    {
        QDomElement chileElement = node.firstChildElement("VALUE");
        if(!chileElement.isNull())
        {
           value = chileElement.text();
           return true;
        }
    }
    return false;
}

void MainWindow::on_pushButton_loadLocalImg_clicked()
{
    QString strFileName = QFileDialog::getOpenFileName(this, "choose file", "*.jpg");

    if(strFileName.isEmpty() ||
            (!strFileName.contains(".jpg") && !strFileName.contains(".jpeg"))
             )
    {
        QMessageBox::about(0, "warning", "please choose the right image.");
        return ;
    }

    QPixmap image(strFileName);
    ui->label->setScaledContents(true);
    ui->label->setPixmap(image);
}
