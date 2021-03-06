#ifndef CHAIKINCURVE_H
#define CHAIKINCURVE_H

#include <QGraphicsItem>
#include <QVector>
#include <QVector3D>
#include <QGraphicsScene>
#include <QWidget>
#include <QPainter>


class ChaikinCurve : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit ChaikinCurve(int iterationCount);
    Q_PROPERTY(QPoint centrePoint READ getCentrePoint WRITE setCentrePoint NOTIFY centrePointChanged)

    void setCentrePoint(QPoint pt) { mCentrePoint = pt; this->update(); }
    QPoint getCentrePoint() const { return mCentrePoint; }
    bool mAnimationActive;
private:
    QPoint mCentrePoint;

signals:
    void centrePointChanged(QPoint);

public slots:
    void IncreaseLod();
    void DecreaseLod();
    void AnimationComplete() { mAnimationActive = false; update(); }

private:
    int mIterationCount;
    QBrush *mYellowBrush;
    QBrush *mGreenBrush;
    QPen *mControlPointPen;
    QVector<QVector3D> mCurvePoints;
    QVector<QVector3D> mOriginalCurvePoints;
    bool mControlPointVisible;
    bool mDragInProgress;
    int mControlPointDragIndex;

    QPainterPath GetPathOfLines() const;
    QPainterPath GetPathOfControlPoints() const;
    void SetNewPointPosition(int controlPointIndex, QPointF dragDropPoint);
    void InitializeCurvePoints();
    int GetIndexOfControlPoint(QPointF pointerPosition);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

public:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF boundingRect() const;
    QPainterPath shape() const;

    void AnimateEvent();
};
#endif // CHAIKINCURVE_H
