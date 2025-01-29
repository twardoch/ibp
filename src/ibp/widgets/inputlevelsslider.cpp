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
#include <QPainterPath>
#include <QMouseEvent>
#include <QGraphicsEffect>
#include <math.h>

#include "inputlevelsslider.h"
#include "../imgproc/intensitymapping.h"

using namespace ibp::imgproc;

namespace ibp {
namespace widgets {

InputLevelsSlider::InputLevelsSlider(QWidget *parent) :
    QWidget(parent),
    mBlackPoint(0.0),
    mWhitePoint(1.0),
    mGammaCorrection(1.0),
    mFunction(256, 1, QImage::Format_Indexed8),
    mHandlePressed(0),
    mHandleSelected(1)
{
//    QGraphicsDropShadowEffect * shadow = new QGraphicsDropShadowEffect();
//    shadow->setOffset(0, 1);
//    shadow->setBlurRadius(12);
//    shadow->setColor(QColor(0, 0, 0, 128));
//    this->setGraphicsEffect(shadow);

    setFocusPolicy(Qt::StrongFocus);

    QVector<QRgb> t;
    for (int i = 0; i < 256; i++)
        t.append(qRgb(i, i, i));
    mFunction.setColorTable(t);
    makeFunction();
}

void InputLevelsSlider::focusInEvent(QFocusEvent * e)
{
    update();
    QWidget::focusInEvent(e);
}
void InputLevelsSlider::focusOutEvent(QFocusEvent *e)
{
    update();
    QWidget::focusOutEvent(e);
}

void InputLevelsSlider::keyPressEvent(QKeyEvent * e)
{
    switch (e->key())
    {
    case Qt::Key_Up:
        mHandleSelected++;
        if (mHandleSelected > 3) mHandleSelected = 3;
        update();
        break;
    case Qt::Key_Down:
        mHandleSelected--;
        if (mHandleSelected < 1) mHandleSelected = 1;
        update();
        break;
    case Qt::Key_Right:
        if (mHandleSelected == 1)
            setBlackPoint(mBlackPoint + 0.01);
        else if (mHandleSelected == 2)
            setGammaCorrection(mGammaCorrection + 0.01);
        else
            setWhitePoint(mWhitePoint + 0.01);
        break;
    case Qt::Key_Left:
        if (mHandleSelected == 1)
            setBlackPoint(mBlackPoint - 0.01);
        else if (mHandleSelected == 2)
            setGammaCorrection(mGammaCorrection - 0.01);
        else
            setWhitePoint(mWhitePoint - 0.01);
        break;
    default:
        QWidget::keyPressEvent(e);
        return;
    }
}

void InputLevelsSlider::paintHandle(QPainter & p, const QPoint & pos, const QColor & c, bool selected)
{
    p.translate(pos);
    QPainterPath path(QPointF(0.0, 0.0));
    path.lineTo(4, 8);
    path.lineTo(-4, 8);
    path.closeSubpath();
    path.translate(0.5, 0.5);
    // shadow
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 32));
    p.drawPath(path.translated(3., 1.));
    p.setBrush(QColor(0, 0, 0, 64));
    p.drawPath(path.translated(1., 1.));
    // handle
    p.setBrush(c);
    p.setPen(QColor(0, 0, 0, 128));
    p.drawPath(path);
    // selection
    if (selected && this->hasFocus())
    {
        QColor h = palette().highlight().color();
        h.setAlpha(192);
        p.setPen(Qt::NoPen);
        p.setBrush(h);
        p.drawPath(path);
    }
    p.translate(-pos);
}

void InputLevelsSlider::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QPainter p(this);
    QRect r = this->rect().adjusted(kLeftMargin, kTopMargin, -kRightMargin, -kBottomMargin);

    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setPen(Qt::NoPen);
    // paint back shadow
    p.setBrush(QColor(0, 0, 0, 16));
    p.drawRoundedRect(r.adjusted(-2, -1, 2, 3), 3, 3);
    p.drawRoundedRect(r.adjusted(-1, 0, 1, 2), 2, 2);
    p.drawRoundedRect(r.adjusted(0, 1, 0, 1), 1, 1);
    // clipping
    QPainterPath clippingPath;
    clippingPath.addRoundedRect(r, 1, 1);
    p.setClipPath(clippingPath);
    // paint identity function
    QLinearGradient grd(r.topLeft(), r.topRight());
    grd.setColorAt(0.0, QColor(0, 0, 0));
    grd.setColorAt(1.0, QColor(255, 255, 255));
    p.setBrush(grd);
    p.drawRect(r.adjusted(0, 0, 0, -1));
    // paint function
    p.drawImage(r.adjusted(0, r.center().y(), 0, 0), mFunction);
    //handles
    p.setClipping(false);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setRenderHint(QPainter::SmoothPixmapTransform, false);

    double y, xB, xW, xG;
    y = r.bottom() - 2;
    xB = (r.width() - 1) * mBlackPoint + kLeftMargin;
    xW = (r.width() - 1) * mWhitePoint + kLeftMargin;
    xG = (xW - xB - 2) * pow(0.5, mGammaCorrection) + xB + 1;

    p.setRenderHint(QPainter::Antialiasing);
    paintHandle(p, QPoint(xG, y), QColor(128, 128, 128), mHandleSelected == 2);
    paintHandle(p, QPoint(xB, y), QColor(64, 64, 64), mHandleSelected == 1);
    paintHandle(p, QPoint(xW, y), QColor(255, 255, 255), mHandleSelected == 3);
}

void InputLevelsSlider::mousePressEvent(QMouseEvent *e)
{
    if (mHandlePressed != 0 || !(e->buttons() & Qt::LeftButton))
        return;

    QRect r = this->rect().adjusted(kLeftMargin, kTopMargin, -kRightMargin, -kBottomMargin);
    double xB, xW, xG, xE;
    xB = (r.width() - 1) * mBlackPoint + kLeftMargin;
    xW = (r.width() - 1) * mWhitePoint + kLeftMargin;
    xG = (xW - xB - 2) * pow(0.5, mGammaCorrection) + xB + 1;
    xE = e->x();

    double dB = fabs(xE - xB);
    double dW = fabs(xE - xW);
    double dG = fabs(xE - xG);

    int handle = dB < dW ? (dB < dG ? 1 : 2) : (dW < dG ? 3 : 2);
    if (handle == 1)
    {
        setBlackPoint((xE - kLeftMargin) / (r.width() - 1));
    }
    else if (handle == 2)
    {
        xG = (xE - xB - 1) / (xW - xB - 2);
        if (xG <= kPow1Over2ToThe10) xG = kPow1Over2ToThe10;
        setGammaCorrection(log(xG) / kLog1Over2);
    }
    else
    {
        setWhitePoint((xE - kLeftMargin) / (r.width() - 1));
    }

    mHandlePressed = handle;
    if (mHandlePressed != mHandleSelected)
    {
        mHandleSelected = mHandlePressed;
        update();
    }
}
void InputLevelsSlider::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->buttons() & Qt::LeftButton)
        return;

    mHandlePressed = 0;
}
void InputLevelsSlider::mouseMoveEvent(QMouseEvent *e)
{
    if (mHandlePressed == 0 || !(e->buttons() & Qt::LeftButton))
        return;

    QRect r = this->rect().adjusted(kLeftMargin, kTopMargin, -kRightMargin, -kBottomMargin);
    double xB, xW, xG, xE;
    xB = (r.width() - 1) * mBlackPoint + kLeftMargin;
    xW = (r.width() - 1) * mWhitePoint + kLeftMargin;
    xE = e->x();

    if (mHandlePressed == 1)
    {
        setBlackPoint((xE - kLeftMargin) / (r.width() - 1));
    }
    else if (mHandlePressed == 2)
    {
        xG = (xE - xB - 1) / (xW - xB - 2);
        if (xG <= kPow1Over2ToThe10) xG = kPow1Over2ToThe10;
        setGammaCorrection(log(xG) / kLog1Over2);
    }
    else
    {
        setWhitePoint((xE - kLeftMargin) / (r.width() - 1));
    }
}

double InputLevelsSlider::blackPoint()
{
    return mBlackPoint;
}
double InputLevelsSlider::whitePoint()
{
    return mWhitePoint;
}
double InputLevelsSlider::gammaCorrection()
{
    return mGammaCorrection;
}

void InputLevelsSlider::setBlackPoint(double v)
{
    if (v == mBlackPoint) return;
    if (v > mWhitePoint - 0.01) v = mWhitePoint - 0.01;
    if (v < 0.0) v = 0.0;
    mBlackPoint = v;
    makeFunction();
    update();
    emit blackPointChanged(v);
}
void InputLevelsSlider::setWhitePoint(double v)
{
    if (v == mWhitePoint) return;
    if (v < mBlackPoint + 0.01) v = mBlackPoint + 0.01;
    if (v > 1.0) v = 1.0;
    mWhitePoint = v;
    makeFunction();
    update();
    emit whitePointChanged(v);
}
void InputLevelsSlider::setGammaCorrection(double v)
{
    if (v == mGammaCorrection) return;
    if (v < 0.1) v = 0.1;
    if (v > 10.0) v = 10.0;
    mGammaCorrection = v;
    makeFunction();
    update();
    emit gammaCorrectionChanged(v);
}

void InputLevelsSlider::setValues(double b, double w, double g)
{
    if (b == mBlackPoint && w == mWhitePoint && g == mGammaCorrection) return;
    if (b > w - 0.01) b = w - 0.01;
    if (b < 0.0) b = 0.0;
    if (w < b + 0.01) w = b + 0.01;
    if (w > 1.0) w = 1.0;
    if (g < 0.1) g = 0.1;
    if (g > 10.0) g = 10.0;
    mBlackPoint = b;
    mWhitePoint = w;
    mGammaCorrection = g;
    makeFunction();
    update();
    emit blackPointChanged(b);
    emit whitePointChanged(w);
    emit gammaCorrectionChanged(g);
}

void InputLevelsSlider::makeFunction()
{
    generateLevelsLUT(mFunction.bits(), mGammaCorrection, mBlackPoint, mWhitePoint);
}

}}
