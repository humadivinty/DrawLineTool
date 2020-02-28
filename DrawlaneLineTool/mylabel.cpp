#include "mylabel.h"
#include<QPainter>
#include<QDebug>
#include <vector>

MyLabel::MyLabel(QWidget *parent) : QLabel(parent)
{
    setMouseTracking(true);
}

void MyLabel::mousePressEvent(QMouseEvent *event)
{
    qDebug()<<"mousePressEvent "<<event->pos();

    switch(event->button())
    {
    case Qt::LeftButton:
        bLBtnDown = true;
        selectLineSeg = nullptr;
        selectLineSeg = getSeleled();
        if(selectLineSeg != nullptr)
        {//选中线段

            selectLineSeg->bDraw = false;

            if(selectLineSeg->bSelStartPt)
            {//选中起点
                qDebug()<<"mousePressEvent  selectLineSeg->bSelStartPt";

                startPoint.setX(selectLineSeg->seg->endPoint.x);
                startPoint.setY(selectLineSeg->seg->endPoint.y);

                endPoint.setX(selectLineSeg->seg->startPoint.x);
                endPoint.setY(selectLineSeg->seg->startPoint.y);
            }
            else if(selectLineSeg->bSelEndPt)
            {//选中终点
                qDebug()<<"mousePressEvent  selectLineSeg->bSelEndPt";

                startPoint.setX(selectLineSeg->seg->startPoint.x);
                startPoint.setY(selectLineSeg->seg->startPoint.y);

                endPoint.setX(selectLineSeg->seg->endPoint.x);
                endPoint.setY(selectLineSeg->seg->endPoint.y);
            }
            else if(selectLineSeg->bSelLine)
            {//选中线段
                qDebug()<<"mousePressEvent  selectLineSeg->bSelLine";

                movePoint = event->pos();

                startPoint.setX(selectLineSeg->seg->startPoint.x);
                startPoint.setY(selectLineSeg->seg->startPoint.y);

                endPoint.setX(selectLineSeg->seg->endPoint.x);
                endPoint.setY(selectLineSeg->seg->endPoint.y);
            }
            else
            {
                qDebug()<<"mousePressEvent  nothing";
            }
            update();
        }
        else
        {//未选中
            //qDebug()<<"mousePressEvent  unselect"<< event->pos();

            startPoint = event->pos();
            endPoint = startPoint;

            tempLine = std::make_shared<LINESEG>();
            tempLine->seg->startPoint.x = startPoint.x();
            tempLine->seg->startPoint.y = startPoint.y();
        }
        break;
    case Qt::RightButton:
        selectLineSeg = nullptr;
        selectLineSeg = getSeleled();

        if(selectLineSeg == nullptr)
            break;

        for(auto it = lineSegs.begin(); it != lineSegs.end(); it++)
        {
            std::shared_ptr<LINESEG> temp = *it;
            if( temp->seg->startPoint.x == selectLineSeg->seg->startPoint.x
                    && temp->seg->startPoint.y == selectLineSeg->seg->startPoint.y
                    &&  temp->seg->endPoint.y == selectLineSeg->seg->endPoint.y
                    &&  temp->seg->endPoint.y == selectLineSeg->seg->endPoint.y
                    )
            {
                qDebug()<<"delete line "<<temp->seg->startPoint.x << ","<<temp->seg->startPoint.y
                          << "--"
                       <<temp->seg->endPoint.x << ","<<temp->seg->endPoint.y;

                lineSegs.erase(it);
                selectLineSeg = nullptr;

                startPoint= {0.0, 0.0};
                endPoint = {0.0, 0.0};
                break;
            }
        }
        update();
//        if(lineSegs.size() == 0)
//        {
//            startPoint= {0.0, 0.0};
//            endPoint = {0.0, 0.0};
//        }

        break;
    default:
        break;
    }
}


void MyLabel::mouseMoveEvent(QMouseEvent *event)
{    
//    qreal x = 0.0;
//    qreal y = 0.0;
//    if(event->pos().x() < 0)
//    {
//        x = 0.0;
//    }
//    else if(event->pos().x() > width())
//    {
//        x = width();
//    }
//    else
//    {
//        x = event->pos().x();
//    }

//    if(event->pos().y() < 0)
//    {
//        y = 0.0;
//    }
//    else if(event->pos().y() > height())
//    {
//        y = height();
//    }
//    else
//    {
//        y = event->pos().y();
//    }
//    QPointF movePt(x, y);
    QPointF movePt = event->pos();
    //qDebug()<<"mouseMoveEvent "<<event->pos();

    if (selectLineSeg != nullptr)
    {//选中线段
        if(bLBtnDown)
        {//鼠标按下
             //qDebug()<<"mouseMoveEvent  selectLineSeg != nullptr "<<"star point "<<selectLineSeg->bSelStartPt << "end point "<< selectLineSeg->bSelEndPt;

            if(selectLineSeg->bSelStartPt || selectLineSeg->bSelEndPt)
            {//选中起点或者终点
                endPoint = movePt;
            }
            else if(selectLineSeg->bSelLine)
            {//选中线段
                double disX = movePt.x() - movePoint.x();
                double disY = movePt.y() - movePoint.y();

                startPoint.setX(startPoint.x() + disX);
                startPoint.setY(startPoint.y() + disY);

                endPoint.setX(endPoint.x() + disX);
                endPoint.setY(endPoint.y() + disY);

                movePoint = movePt;
            }
        }
    }
    else
    {//未选中线段
        if(bLBtnDown)
        {
            endPoint = movePt;
        } else
        {
            selSeg(movePt);
        }
    }
    update();
}

void MyLabel::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug()<<"mouseReleaseEvent "<<event->pos();

    switch(event->button())
    {
    case Qt::LeftButton:
        bLBtnDown = false;
        if(selectLineSeg != nullptr)
        {

            if(selectLineSeg->bSelStartPt)
            {//选中起点
                selectLineSeg->seg->startPoint.x = event->pos().x();
                selectLineSeg->seg->startPoint.y = event->pos().y();

            }
            else if(selectLineSeg->bSelEndPt)
            {//选中终点
                selectLineSeg->seg->endPoint.x = event->pos().x();
                selectLineSeg->seg->endPoint.y = event->pos().y();
            }
            else if(selectLineSeg->bSelLine)
            {//选中线段
                selectLineSeg->seg->startPoint.x = startPoint.x();
                selectLineSeg->seg->startPoint.y = startPoint.y();

                selectLineSeg->seg->endPoint.x = endPoint.x();
                selectLineSeg->seg->endPoint.y = endPoint.y();
            }
            selectLineSeg->bDraw = true;

            selectLineSeg->bSelStartPt = false;
            selectLineSeg->bSelEndPt = false;
            selectLineSeg->bSelLine = false;
            selectLineSeg = nullptr;
        }
        else
        {
            tempLine->seg->endPoint.x = event->pos().x();
            tempLine->seg->endPoint.y = event->pos().y();
            tempLine->bDraw = true;
            lineSegs.push_back(tempLine);
        }

        break;

    case Qt::RightButton:

        break;
    default:
        break;
    }
}

void MyLabel::paintEvent(QPaintEvent *event)
{
    //qDebug()<<"paintEvent";
    QLabel::paintEvent(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    drawLineSeg(&p);//绘制线段

    QPen pen;
    pen.setColor(QColor(255, 0, 0));
    pen.setWidth(2);
    p.setPen(pen);
    p.drawLine(startPoint, endPoint);

    //qDebug()<<"paintEvent draw line"<<startPoint <<", "<< endPoint;
}

std::vector<LineSegment> MyLabel::GetCurrentLines()
{
    std::vector<LineSegment> lines;
    const int width = this->width();
    const int height = this->height();
    double x = 0.0;
    double y = 0.0;

    auto funcCompear = [](double value, int base)
    {
        if(value <= 0.0)
            return 0.0;
        if(value >= base)
            return 1.0;
        return value/base;
    };

    for(auto it = lineSegs.begin(); it!= lineSegs.end(); it++)
    {
        std::shared_ptr<LINESEG> pL = *it;
        LineSegment line;
        x = funcCompear(pL->seg->startPoint.x, width);
        y = funcCompear(pL->seg->startPoint.y, height);
        line.startPoint = PointEx(x, y);

        x = funcCompear(pL->seg->endPoint.x, width);
        y = funcCompear(pL->seg->endPoint.y, height);

        line.endPoint = PointEx(x, y);
        lines.push_back(line);
    }
    return lines;
}

void MyLabel::SetCurrentLines(std::vector<LineSegment> &lines)
{
    const int width = this->width();
    const int height = this->height();
    double x = 0.0;
    double y = 0.0;

    lineSegs.clear();

    auto converToCoordinate = [](int percentage, int baseValue)
    {
        return percentage* baseValue;
    };

    for(auto it = lines.begin(); it!= lines.end; it++)
    {
        LineSegment lineInput = *it;
        std::shared_ptr<LINESEG> pL = std::make_shared<LINESEG>();
        pL->seg->startPoint.x = converToCoordinate(lineInput.startPoint.x, width);
        pL->seg->startPoint.y = converToCoordinate(lineInput.startPoint.y, height);

        pL->seg->endPoint.x = converToCoordinate(lineInput.endPoint.x, width);
        pL->seg->endPoint.y = converToCoordinate(lineInput.endPoint.y, height);

        lineSegs.push_back(PL);
    }
    update();
}

void MyLabel::drawLineSeg(QPainter *p)
{
    QPen pen;
    pen.setColor(QColor(255, 0, 0));

    int num = lineSegs.size();
    QPointF pt1, pt2;
    for(int i = 0; i < num; i++)
    {
        std::shared_ptr<LINESEG> oneLine = lineSegs.at(i);
        if (oneLine->bDraw)
        {//是否绘制
            pt1.setX(oneLine->seg->startPoint.x);
            pt1.setY(oneLine->seg->startPoint.y);

            pt2.setX(oneLine->seg->endPoint.x);
            pt2.setY(oneLine->seg->endPoint.y);

            if(oneLine->bSelLine||oneLine->bSelStartPt||oneLine->bSelEndPt)
            {//选中
                pen.setWidth(4);
                p->setPen(pen);
                if(oneLine->bSelLine)
                {
                    p->drawLine(pt1, pt2);
                }
                if(oneLine->bSelStartPt)
                {
                    p->drawEllipse(pt1, 5, 5);
                    pen.setWidth(2);
                    p->setPen(pen);
                    p->drawLine(pt1, pt2);
                }
                if(oneLine->bSelEndPt)
                {
                    p->drawEllipse(pt2, 5, 5);
                    pen.setWidth(2);
                    p->setPen(pen);
                    p->drawLine(pt1, pt2);
                }
            }
            else if(oneLine->bSelLine == false
                    && oneLine->bSelStartPt == false
                    && oneLine->bSelEndPt == false)
            {
                pen.setWidth(2);
                p->setPen(pen);
                p->drawLine(pt1, pt2);

                //qDebug()<<"draw line "<<pt1<<","<<pt2;
            }
        }
        bFirstDraw = false;
    }
}

void MyLabel::selSeg(QPointF &pt)
{
    int num = lineSegs.size();

    for(int i = 0; i < num; i++)
    {
        std::shared_ptr<LINESEG> oneLine = lineSegs.at(i);
        LineSegment* oneLineDeg = oneLine->seg;

        PointEx ptEx(pt.x(), pt.y());
        PointEx np;//线段上的点
        double dis = pToLinesegDist(ptEx, *oneLineDeg, np);
        np = perpendicular(ptEx, *oneLineDeg);
        if(dis < 5 && dis >= 0.0)
        {
            double relationValue = relation(np, *oneLineDeg);
            if(relationValue < EP)
            {//end point
                oneLine->bSelStartPt = true;
                oneLine->bSelLine = false;
                oneLine->bSelEndPt = false;
            }
            else if( relationValue - 1.0 < EP)
            {//on the line
                oneLine->bSelStartPt = false;
                oneLine->bSelLine = true;
                oneLine->bSelEndPt = false;
            }
            else if(relationValue >= 1.0 )
            {//
                oneLine->bSelStartPt = false;
                oneLine->bSelLine = false;
                oneLine->bSelEndPt = true;
            }


//            if(abs(l)< EP)
//            {//起点
//                oneLine->bSelStartPt = true;
//                oneLine->bSelLine = false;
//                oneLine->bSelEndPt = false;
//            }
//            else if(abs(l - 1.0) < EP)
//            {//终点
//                oneLine->bSelStartPt = false;
//                oneLine->bSelLine = false;
//                oneLine->bSelEndPt = true;
//            }
//            else if(l < 1 && l > 0)
//            {//整条线
//                oneLine->bSelStartPt = false;
//                oneLine->bSelLine = true;
//                oneLine->bSelEndPt = false;
//            }
        }
        else
        {
            oneLine->bSelLine = false;
            oneLine->bSelEndPt = false;
            oneLine->bSelStartPt = false;
        }
    }
}

 std::shared_ptr<LINESEG> MyLabel::getSeleled()
{
    int num = lineSegs.size();

    for(int i = 0; i < num; i++)
    {
         std::shared_ptr<LINESEG> oneLine = lineSegs.at(i);
        if(oneLine->bSelLine
                || oneLine->bSelStartPt
                || oneLine->bSelEndPt)
        {//选中
            return oneLine;
        }
    }
    return nullptr;
}
