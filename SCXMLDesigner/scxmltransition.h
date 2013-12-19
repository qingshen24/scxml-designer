#ifndef SCXMLTRANSITION_H
#define SCXMLTRANSITION_H

#include <QAbstractTransition>
#include "scxmlstate.h"
#include "metadatasupport.h"


class SCXMLTransition : public QAbstractTransition, public QGraphicsItem, public MetaDataSupport
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    explicit SCXMLTransition(SCXMLState *parent = 0);

    qreal GetX1() { return mX1; }
    qreal GetY1() { return mY1; }
    qreal GetX2() { return mX2; }
    qreal GetY2() { return mY2; }
    QString GetDescription() { return mDescription; }

    void SetX1(qreal value) { mX1 = value; }
    void SetY1(qreal value) { mY1 = value; }
    void SetX2(qreal value) { mX2 = value; }
    void SetY2(qreal value) { mY2 = value; }
    void SetDescription(QString value) { mDescription = value; }

    void setTransitionType(QString transitionType) { mTransitionType = transitionType; }
    QString getTransitionType() { return mTransitionType; }

    //FIXME: these two virtuals need to be implemented fully at some point
    bool eventTest(QEvent * event) { Q_UNUSED(event); return true; }
    void onTransition(QEvent * event) { Q_UNUSED(event) }

    // MetaDataSupport overrides
    void ApplyMetaData(QMap<QString, QString>* mapMetaData);
    QString GetMetaDataString();

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    // QGraphicsItem overrides
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void Update();
    void Connect(SCXMLState* parentState, SCXMLState* targetState)
    {
        setTargetState(targetState);
        parentState->addTransition(this);
        targetState->AddIncomingTransition(this);
    }

    bool CalculatePaths(QPainterPath *bezierPath, QPainterPath *arrowHeadPath,
                        QPainterPath * controlLine1Path, QPainterPath *controlLine2Path) const;
signals:
    
public slots:
    
private:
    SCXMLState* mParentState;
    QString mTransitionType;
    qreal mX1;
    qreal mX2;
    qreal mY1;
    qreal mY2;
    QString mDescription;
    QPoint mPointControl1;
    QPoint mPointControl2;
    bool mMovingControlPoint1;
    bool mMovingControlPoint2;
    qreal mNewControlPoint1StartX;
    qreal mNewControlPoint1StartY;
    qreal mNewControlPoint2StartX;
    qreal mNewControlPoint2StartY;
};

#endif // SCXMLTRANSITION_H
