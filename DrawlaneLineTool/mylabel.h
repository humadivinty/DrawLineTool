#ifndef MYLABEL_H
#define MYLABEL_H

#include"GraphBase.h"

#include <QWidget>
#include<QLabel>
#include<QMouseEvent>
#include<QPaintEvent>
#include <vector>
#include<memory>

class MyLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MyLabel(QWidget *parent = 0);

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
     void mouseReleaseEvent(QMouseEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

    std::vector<LineSegment> GetCurrentLines();
    void SetCurrentLines(std::vector<LineSegment>& lines );

private:
    void drawLineSeg(QPainter* p);
    void selSeg(QPointF&pt);
    std::shared_ptr<LINESEG> getSeleled();//获取选中的线段
private:

    bool bLBtnDown = false;//左键按下
    bool bFirstDraw;

    QPointF startPoint, endPoint;//绘制线的起点和终点

    std::shared_ptr<LINESEG> tempLine = nullptr;
    std::vector<std::shared_ptr<LINESEG> > lineSegs;//线段列表

    std::shared_ptr<LINESEG> selectLineSeg = nullptr;//选中的线段

    QPointF movePoint;
};

#endif // MYLABEL_H
