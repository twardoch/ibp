//
// MIT License
// 
// Copyright (c) Deif Lou
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QPainterPath>
#include <math.h>

#include "curves.h"
#include "../misc/util.h"
#include "../misc/nearestneighborsplineinterpolator1D.h"
#include "../misc/linearsplineinterpolator1D.h"
#include "../misc/cubicsplineinterpolator1D.h"

namespace ibp {
namespace widgets {

const QSize Curves::kKnotSize(10, 10);

Curves::Curves(QWidget *parent) :
    QWidget(parent),
    mZoomFactor(1.),
    mOffset(0.),
    mIsPeriodic(false),
    mIsInputEnabled(true),
    mInputStatus(NoStatus),
    mKnotIndex(-1),
    mSplineInterpolator(0),
    mInterpolationMode(Cubic),
    mPaintDelegate(0),
    mWidgetState(QStyle::State_None),
    mKnotStates(2, QStyle::State_None),
    mScrollBar(0),
    mEmitScrolbarSignals(true)
{
    this->setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    mSplineInterpolator = new CubicSplineInterpolator1D();
    mSplineInterpolator->addKnot(0., 0.);
    mSplineInterpolator->addKnot(1., 1.);

    mScrollBar = new QScrollBar(Qt::Horizontal, this);
    mScrollBar->hide();
    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setContentsMargins(kLeftMargin, kTopMargin, kRightMargin, kBottomMargin);
    layout->addStretch(1);
    layout->addWidget(mScrollBar);

    this->connect(mScrollBar, SIGNAL(valueChanged(int)), this, SLOT(On_mScrollBar_valueChanged(int)));
    mScrollBar->installEventFilter(this);

    updateScrollBar();
}

void Curves::updateScrollBar()
{
    int w = graphRect().width();
    mScrollBar->setRange(0, w * mZoomFactor - w);
    mScrollBar->setValue(mOffset);
    mScrollBar->setPageStep(w);
}

QRect Curves::rectWithoutMargins() const
{
    return this->rect().adjusted(kLeftMargin, kTopMargin, -kRightMargin,
                                 -kBottomMargin - (mZoomFactor > 1. ? mScrollBar->height() : 0));
}

QRect Curves::graphRect() const
{
    return mPaintDelegate ? mPaintDelegate->graphRect(rectWithoutMargins()) : rectWithoutMargins();
}

int Curves::knotUnderCoords(const QPoint &p, bool addKnotIfPossible)
{
    QRect gr = graphRect();
    double kx, ky;
    const double rx = (kKnotSize.width() / 2.) / ((double)gr.width() * mZoomFactor);
    const double ry = (kKnotSize.height() / 2.) / ((double)gr.height());
    const double x = mapToSplineInterpolator(p.x());
    const double y = 1. - (p.y() - gr.top()) / ((double)gr.height());
    const double minimumDistance = kMinimumDistanceToAddKnot / ((double)gr.height());
    int index = -1;

    for (int i = 0; i < mSplineInterpolator->size() - mIsPeriodic ? 1 : 0; i++)
    {
        kx = mSplineInterpolator->knot(i).x();
        ky = mSplineInterpolator->knot(i).y();
        if (x > kx - rx && x < kx + rx && y > ky - ry && y < ky + ry)
            return i;
    }

    if (addKnotIfPossible && mSplineInterpolator->size() - (mIsPeriodic ? 1 : 0) < kMaximumNumberOfKnots &&
        fabs(IBP_clamp(0., mSplineInterpolator->f(x), 1.) - y) < minimumDistance)
    {
        if (!mSplineInterpolator->addKnot(x, y, false, &index) || index == -1)
            return -1;

        const double min = index > 0 ?
                    mSplineInterpolator->knot(index - 1).x() + kMinimumDistanceBetweenKnots : 0.;
        const double max = index < mSplineInterpolator->size() - (mIsPeriodic ? 2 : 1) ?
                    mSplineInterpolator->knot(index + 1).x() - kMinimumDistanceBetweenKnots : 1.;
        mSplineInterpolator->setKnot(index, IBP_clamp(min, x, max), IBP_clamp(0., y, 1.));

        if (mIsPeriodic && index == 0)
            mSplineInterpolator->setKnot(mSplineInterpolator->size() - 1,
                                         IBP_clamp(min, x, max) + 1., IBP_clamp(0., y, 1.));

        if (mKnotIndex >= index)
            mKnotIndex++;
        mKnotStates.insert(index, QStyle::State_None);
        if (mPaintDelegate)
            mPaintDelegate->update(CurvesPaintDelegate::KnotsChanged, this, rectWithoutMargins());
        emit knotsChanged(mSplineInterpolator->knots());
        update();
        return index;
    }

    return -1;
}

void Curves::paintEvent(QPaintEvent *)
{
    if (!mPaintDelegate)
        return;

    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QRect r = this->rect().adjusted(kLeftMargin, kTopMargin, -kRightMargin, -kBottomMargin);

    // shadow
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 32));
    p.drawRoundedRect(r.adjusted(-2, -1, 2, 3), 3, 3);
    p.drawRoundedRect(r.adjusted(-1, 0, 1, 2), 2, 2);
    p.setBrush(QColor(0, 0, 0, 50));
    p.drawRoundedRect(r.adjusted(0, 1, 0, 1), 1, 1);

    // ----
    QStyle::State widgetState = mWidgetState |
                                (isEnabled() ? QStyle::State_Enabled : QStyle::State_None) |
                                (hasFocus() ? QStyle::State_HasFocus : QStyle::State_None) |
                                (!mIsInputEnabled ? QStyle::State_ReadOnly : QStyle::State_None);

    // ----
    if (mZoomFactor > 1.)
        r.adjust(0, 0, 0, -mScrollBar->height());

    // clip
    QPainterPath clippingPath;
    clippingPath.addRoundedRect(r, 1, 1);
    p.setClipPath(clippingPath);

    // graph
    QRect graphRectCopy = graphRect();
    QPolygonF poly;
    for (int i = r.left(); i <= r.right(); i++)
        poly.append(QPointF(i, (1. - valueAt(mapToSplineInterpolator(i))) *
                                graphRectCopy.height() + graphRectCopy.top()));

    // knots
    QVector<QPointF> knotPositions;
    if (graphRectCopy.width() >= kMinimumSizeForInput &&
        graphRectCopy.height() >= kMinimumSizeForInput)
        for (int i = 0; i < mSplineInterpolator->size() - mIsPeriodic ? 1 : 0; i++)
            knotPositions << QPointF(mapFromSplineInterpolator(mSplineInterpolator->knot(i).x()),
                                     (1. - mSplineInterpolator->knot(i).y()) * graphRectCopy.height() +
                                     graphRectCopy.top());

    p.save();
    mPaintDelegate->paint(p, this, r, widgetState, poly, knotPositions, mKnotStates, kKnotSize);
    p.restore();

    // focus rect
    if (widgetState & QStyle::State_HasFocus)
    {
        p.setPen(QPen(this->palette().color(QPalette::Highlight), 4));
        p.setBrush(Qt::NoBrush);
        p.drawRect(r);
    }

    // paint if disabled
    if (!(widgetState & QStyle::State_Enabled))
    {
        QColor disabledColor = this->palette().color(QPalette::Button);
        disabledColor.setAlpha(200);
        p.fillRect(r, disabledColor);
    }
}

void Curves::mousePressEvent(QMouseEvent * e)
{
    if (!mIsInputEnabled || mInputStatus != NoStatus)
        return;

    QRect gr = graphRect();
    if (gr.width() < kMinimumSizeForInput || gr.height() < kMinimumSizeForInput)
        return;

    int index;
    bool mustEmitSignal = false;

    if (e->button() == Qt::LeftButton)
    {
        if (mKnotIndex > -1)
            mKnotStates[mKnotIndex] &= ~QStyle::State_Selected;

        // add a knot and check if there is a knot in this position
        index = knotUnderCoords(e->pos(), true);
        mustEmitSignal = index != mKnotIndex;
        if (index != -1)
        {
            mKnotIndex = index;
            mInputStatus = DraggingKnot;
            mKnotStates[mKnotIndex] |= QStyle::State_Selected | QStyle::State_Sunken;
            update();
            if (mustEmitSignal)
                emit selectedKnotChanged(mKnotIndex);
            return;
        }
        // clicked in non-input area, deselect selected item
        mKnotIndex = -1;
        mInputStatus = NoStatus;
        update();
        if (mustEmitSignal)
            emit selectedKnotChanged(mKnotIndex);
        return;
    }

    if (e->button() == Qt::RightButton && mSplineInterpolator->size() > (mIsPeriodic ? 3 : 2))
    {
        index = knotUnderCoords(e->pos());
        if (index != -1)
        {
            mKnotStates.remove(index);
            if (index == mKnotIndex)
            {
                mKnotIndex = -1;
                mustEmitSignal = true;
            }
            else if (mKnotIndex > index)
            {
                mKnotIndex--;
                mustEmitSignal = true;
            }

            mSplineInterpolator->removeKnot(index);

            if (mIsPeriodic && index == 0)
                mSplineInterpolator->setKnot(mSplineInterpolator->size() - 1,
                                             mSplineInterpolator->knot(0).x() + 1.,
                                             mSplineInterpolator->knot(0).y());

            if (mPaintDelegate)
                mPaintDelegate->update(CurvesPaintDelegate::KnotsChanged, this, rectWithoutMargins());
            update();
            emit knotsChanged(mSplineInterpolator->knots());
            if (mustEmitSignal)
                emit selectedKnotChanged(mKnotIndex);
            return;
        }
    }
}

void Curves::mouseReleaseEvent(QMouseEvent * e)
{
    if (!mIsInputEnabled)
        return;

    if (e->button() == Qt::LeftButton)
    {
        for (int i = 0; i < mKnotStates.size(); i++)
            mKnotStates[i] &= ~QStyle::State_Sunken;
        mInputStatus = NoStatus;
        mouseMoveEvent(e);
    }
}

void Curves::mouseMoveEvent(QMouseEvent * e)
{
    if (!mIsInputEnabled)
        return;

    mWidgetState = mWidgetState | QStyle::State_MouseOver;

    // dragging knot
    if (mInputStatus == DraggingKnot && mKnotIndex > -1 && e->buttons() & Qt::LeftButton)
    {
        QRect gr = graphRect();
        QPointF position = e->pos();

        const double min = mKnotIndex > 0 ?
                    mSplineInterpolator->knot(mKnotIndex - 1).x() + kMinimumDistanceBetweenKnots : 0.;
        const double max = IBP_minimum(1., mKnotIndex < mSplineInterpolator->size() - 1 ?
                    mSplineInterpolator->knot(mKnotIndex + 1).x() - kMinimumDistanceBetweenKnots : 1.);

        position.setX(IBP_clamp(min, mapToSplineInterpolator(position.x()), max));
        position.setY(IBP_clamp(0., 1. - (position.y() - gr.top()) / (double)gr.height(), 1.));
        mSplineInterpolator->setKnot(mKnotIndex, position.x(), position.y());

        if (mIsPeriodic && mKnotIndex == 0)
            mSplineInterpolator->setKnot(mSplineInterpolator->size() - 1,
                                         IBP_maximum(1.001, position.x() + 1.),
                                         position.y());

        if (mPaintDelegate)
            mPaintDelegate->update(CurvesPaintDelegate::KnotsChanged, this, rectWithoutMargins());

        update();
        emit knotsChanged(mSplineInterpolator->knots());
        return;
    }

    // not dragging
    this->unsetCursor();

    if (!rectWithoutMargins().contains(e->pos()))
        return;

    for (int i = 0; i < mKnotStates.size(); i++)
        mKnotStates[i] &= ~QStyle::State_MouseOver;

    const int index = knotUnderCoords(e->pos());
    if (index != -1)
        mKnotStates[index] |= QStyle::State_MouseOver;

    update();
}

void Curves::leaveEvent(QEvent *)
{
    mWidgetState = mWidgetState & ~QStyle::State_MouseOver;
    for (int i = 0; i < mKnotStates.size(); i++)
        mKnotStates[i] &= ~QStyle::State_MouseOver;
    unsetCursor();
    update();
}

void Curves::resizeEvent(QResizeEvent * e)
{
    if (e->oldSize().isEmpty())
        return;
    setOffset(mOffset * e->size().width() / e->oldSize().width());
}

bool Curves::eventFilter(QObject *o, QEvent *e)
{
    if (o == mScrollBar && e->type() == QEvent::Enter)
        leaveEvent(e);
    return false;
}

void Curves::keyPressEvent(QKeyEvent *e)
{
    if (mKnotIndex == -1)
        return;

    Interpolator1DKnot k = mSplineInterpolator->knot(mKnotIndex);
    const double min = mKnotIndex > 0 ?
                mSplineInterpolator->knot(mKnotIndex - 1).x() + kMinimumDistanceBetweenKnots : 0.;
    const double max = IBP_minimum(1., mKnotIndex < mSplineInterpolator->size() - 1 ?
                mSplineInterpolator->knot(mKnotIndex + 1).x() - kMinimumDistanceBetweenKnots : 1.);

    switch (e->key())
    {
    case Qt::Key_Right:
        if (e->modifiers() & Qt::ControlModifier)
        {
            mKnotStates[mKnotIndex] &= ~QStyle::State_Selected;

            mKnotIndex++;
            if (mKnotIndex == mSplineInterpolator->size() - (mIsPeriodic ? 1 : 0))
                mKnotIndex--;

            mKnotStates[mKnotIndex] |= QStyle::State_Selected;
            update();
            emit selectedKnotChanged(mKnotIndex);
        }
        else
        {
            k.setX(IBP_clamp(min, k.x() + kKeypressIncrement, max));
            mSplineInterpolator->setKnot(mKnotIndex, k);

            if (mIsPeriodic && mKnotIndex == 0)
                mSplineInterpolator->setKnot(mSplineInterpolator->size() - 1, IBP_maximum(1.001, k.x() + 1.), k.y());

            if (mPaintDelegate)
                mPaintDelegate->update(CurvesPaintDelegate::KnotsChanged, this, rectWithoutMargins());
            update();
            emit knotsChanged(mSplineInterpolator->knots());
        }
        break;
    case Qt::Key_Left:
        if (e->modifiers() & Qt::ControlModifier)
        {
            mKnotStates[mKnotIndex] &= ~QStyle::State_Selected;

            mKnotIndex--;
            if (mKnotIndex == -1)
                mKnotIndex = 0;

            mKnotStates[mKnotIndex] |= QStyle::State_Selected;
            update();
            emit selectedKnotChanged(mKnotIndex);
        }
        else
        {
            k.setX(IBP_clamp(min, k.x() - kKeypressIncrement, max));
            mSplineInterpolator->setKnot(mKnotIndex, k);

            if (mIsPeriodic && mKnotIndex == 0)
                mSplineInterpolator->setKnot(mSplineInterpolator->size() - 1, IBP_maximum(1.001, k.x() + 1.), k.y());

            if (mPaintDelegate)
                mPaintDelegate->update(CurvesPaintDelegate::KnotsChanged, this, rectWithoutMargins());
            update();
            emit knotsChanged(mSplineInterpolator->knots());
        }
        break;
    case Qt::Key_Up:
        k.setY(IBP_minimum(k.y() + kKeypressIncrement, 1.));
        mSplineInterpolator->setKnot(mKnotIndex, k);

        if (mIsPeriodic && mKnotIndex == 0)
            mSplineInterpolator->setKnot(mSplineInterpolator->size() - 1, k.x() + 1., k.y());

        if (mPaintDelegate)
            mPaintDelegate->update(CurvesPaintDelegate::KnotsChanged, this, rectWithoutMargins());
        update();
        emit knotsChanged(mSplineInterpolator->knots());
        break;
    case Qt::Key_Down:
        k.setY(IBP_maximum(k.y() - kKeypressIncrement, 0.));
        mSplineInterpolator->setKnot(mKnotIndex, k);

        if (mIsPeriodic && mKnotIndex == 0)
            mSplineInterpolator->setKnot(mSplineInterpolator->size() - 1, k.x() + 1., k.y());

        if (mPaintDelegate)
            mPaintDelegate->update(CurvesPaintDelegate::KnotsChanged, this, rectWithoutMargins());
        update();
        emit knotsChanged(mSplineInterpolator->knots());
        break;
    default:
        QWidget::keyPressEvent(e);
        return;
    }
}

double Curves::zoomFactor() const
{
    return mZoomFactor;
}

double Curves::offset() const
{
    return mOffset;
}

bool Curves::isPeriodic() const
{
    return mIsPeriodic;
}

bool Curves::isInputEnabled() const
{
    return mIsInputEnabled;
}

const Interpolator1DKnots & Curves::knots() const
{
    return mSplineInterpolator->knots();
}

int Curves::selectedKnotIndex() const
{
    return mKnotIndex;
}

const Interpolator1DKnot & Curves::selectedKnot() const
{
    static const Interpolator1DKnot nullSplineInterpolatorKnot;

    if (mKnotIndex == -1)
        return nullSplineInterpolatorKnot;
    return mSplineInterpolator->knot(mKnotIndex);
}

Curves::InterpolationMode Curves::interpolationMode() const
{
    return mInterpolationMode;
}

CurvesPaintDelegate *Curves::paintDelegate() const
{
    return mPaintDelegate;
}

double Curves::valueAt(double v)
{
    return IBP_clamp(0., mSplineInterpolator->f(v), 1.);
}

double Curves::mapToSplineInterpolator(double v) const
{
    QRect r = graphRect();
    return (v - r.left() + mOffset) / ((r.width() - 1) * mZoomFactor);
}

double Curves::mapFromSplineInterpolator(double v) const
{
    QRect r = graphRect();
    return (v * (r.width() - 1) * mZoomFactor) - mOffset + r.left();
}

void Curves::setZoomFactor(double v)
{
    if (qFuzzyCompare(mZoomFactor, v))
        return;
    double oldZoomFactor = mZoomFactor;
    mZoomFactor = v;
    if (v > 1.)
        setOffset(mOffset * mZoomFactor / oldZoomFactor);
    else
        center();
    mScrollBar->setVisible(mZoomFactor > 1.);
    updateScrollBar();
    if (mPaintDelegate)
        mPaintDelegate->update(CurvesPaintDelegate::ZoomFactorChanged, this, rectWithoutMargins());
    update();
    emit zoomFactorChanged(v);
}

void Curves::setOffset(double v)
{
    if (qFuzzyCompare(mOffset, v))
        return;
    int w = graphRect().width();
    mOffset = IBP_clamp(0., v, w * mZoomFactor - w);
    mEmitScrolbarSignals = false;
    updateScrollBar();
    mEmitScrolbarSignals = true;
    if (mPaintDelegate)
        mPaintDelegate->update(CurvesPaintDelegate::OffsetChanged, this, rectWithoutMargins());
    update();
    emit offsetChanged(v);
}

void Curves::center()
{
    int w = graphRect().width();
    setOffset((w * mZoomFactor - w) / 2);
}

void Curves::fit()
{
    setZoomFactor(1.);
    center();
}

void Curves::setPeriodic(bool v)
{
    if (mIsPeriodic == v)
        return;
    mIsPeriodic = v;

    if (mIsPeriodic)
    {
        mSplineInterpolator->addKnot(IBP_maximum(1.001, mSplineInterpolator->knot(0).x() + 1.),
                                     mSplineInterpolator->knot(0).y());
        mSplineInterpolator->setExtrapolationMode(Interpolator1D::ExtrapolationMode_Repeat,
                                                  Interpolator1D::ExtrapolationMode_Repeat);
        if (mInterpolationMode == Cubic)
            ((CubicSplineInterpolator1D *)mSplineInterpolator)->setBoundaryConditions(
                                                    CubicSplineInterpolator1D::BoundaryConditions_Periodic,
                                                    CubicSplineInterpolator1D::BoundaryConditions_Periodic);
    }
    else
    {
        mSplineInterpolator->removeKnot(mSplineInterpolator->size() - 1);
        mSplineInterpolator->setExtrapolationMode(Interpolator1D::ExtrapolationMode_Clamp,
                                                  Interpolator1D::ExtrapolationMode_Clamp);
        ((CubicSplineInterpolator1D *)mSplineInterpolator)->setBoundaryConditions(
                                                  CubicSplineInterpolator1D::BoundaryConditions_Natural,
                                                  CubicSplineInterpolator1D::BoundaryConditions_Natural);
    }
    if (mPaintDelegate)
        mPaintDelegate->update(CurvesPaintDelegate::PeriodicChanged, this, rectWithoutMargins());

    update();
    emit periodicChanged(v);
}

void Curves::setInputEnabled(bool v)
{
    if (mIsInputEnabled == v)
        return;
    mIsInputEnabled = v;
    if (mPaintDelegate)
        mPaintDelegate->update(CurvesPaintDelegate::InputEnabledChanged, this, rectWithoutMargins());
    update();
    emit inputEnabledChanged(v);
}

void Curves::setKnots(const Interpolator1DKnots &k)
{
    mSplineInterpolator->setKnots(k);
    mKnotStates = QVector<QStyle::State>(k.size(), QStyle::State_None);
    mKnotIndex = -1;
    if (mIsPeriodic)
        mSplineInterpolator->addKnot(IBP_maximum(1.001, k[0].x() + 1.), k[0].y());
    if (mPaintDelegate)
        mPaintDelegate->update(CurvesPaintDelegate::KnotsChanged, this, rectWithoutMargins());
    update();
    emit knotsChanged(k);
    emit selectedKnotChanged(mKnotIndex);
}

void Curves::setSelectedKnot(double x, double y)
{
    const double min = mKnotIndex > 0 ?
                mSplineInterpolator->knot(mKnotIndex - 1).x() + kMinimumDistanceBetweenKnots : 0.;
    const double max = mKnotIndex < mSplineInterpolator->size() - 1 ?
                mSplineInterpolator->knot(mKnotIndex + 1).x() - kMinimumDistanceBetweenKnots : 1.;

    mSplineInterpolator->setKnot(mKnotIndex, IBP_clamp(min, x, max), IBP_clamp(0., y, 1.));

    if (mPaintDelegate)
        mPaintDelegate->update(CurvesPaintDelegate::KnotsChanged, this, rectWithoutMargins());
    update();
    emit knotsChanged(mSplineInterpolator->knots());
}

void Curves::setSelectedKnot(const Interpolator1DKnot &k)
{
    setSelectedKnot(k.x(), k.y());
}

void Curves::setInterpolationMode(Curves::InterpolationMode m)
{
    if (mInterpolationMode == m)
        return;

    mInterpolationMode = m;

    Interpolator1D * tmpSplineInterpolator = mSplineInterpolator;

    if (mInterpolationMode == NearestNeighbor)
        mSplineInterpolator = new NearestNeighborSplineInterpolator1D();
    else if (mInterpolationMode == Linear)
        mSplineInterpolator = new LinearSplineInterpolator1D();
    else
    {
        mSplineInterpolator = new CubicSplineInterpolator1D();
        ((CubicSplineInterpolator1D *)mSplineInterpolator)->setBoundaryConditions(
                    mIsPeriodic ? CubicSplineInterpolator1D::BoundaryConditions_Periodic :
                                  CubicSplineInterpolator1D::BoundaryConditions_Natural,
                    mIsPeriodic ? CubicSplineInterpolator1D::BoundaryConditions_Periodic :
                                  CubicSplineInterpolator1D::BoundaryConditions_Natural);
    }

    mSplineInterpolator->setKnots(tmpSplineInterpolator->knots());
    mSplineInterpolator->setExtrapolationMode(tmpSplineInterpolator->floorExtrapolationMode(),
                                              tmpSplineInterpolator->ceilExtrapolationMode(),
                                              tmpSplineInterpolator->floorExtrapolationValue(),
                                              tmpSplineInterpolator->ceilExtrapolationValue());
    delete tmpSplineInterpolator;

    if (mPaintDelegate)
        mPaintDelegate->update(CurvesPaintDelegate::InterpolationModeChanged, this, rectWithoutMargins());
    update();
    emit interpolationModeChanged(m);
}

void Curves::setPaintDelegate(CurvesPaintDelegate *pd)
{
    if (pd == mPaintDelegate)
        return;
    mPaintDelegate = pd;
    if (mPaintDelegate)
    {
        mPaintDelegate->update(CurvesPaintDelegate::KnotsChanged, this, rectWithoutMargins());
        mPaintDelegate->update(CurvesPaintDelegate::ZoomFactorChanged, this, rectWithoutMargins());
        mPaintDelegate->update(CurvesPaintDelegate::OffsetChanged, this, rectWithoutMargins());
        mPaintDelegate->update(CurvesPaintDelegate::PeriodicChanged, this, rectWithoutMargins());
        mPaintDelegate->update(CurvesPaintDelegate::InputEnabledChanged, this, rectWithoutMargins());
        mPaintDelegate->update(CurvesPaintDelegate::InterpolationModeChanged, this, rectWithoutMargins());

        this->connect(mPaintDelegate, SIGNAL(updateRequired()), this, SLOT(update()));
    }
    update();
    emit paintDelegateChanged(pd);
}

void Curves::On_mScrollBar_valueChanged(int v)
{
    if (mEmitScrolbarSignals)
        setOffset(v);
}

}}
