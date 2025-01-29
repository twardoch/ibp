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

#include <QRegularExpression>
#include <math.h>
#include <opencv2/imgproc.hpp>

#include "filter.h"
#include "filterwidget.h"
#include <imgproc/types.h>
#include <imgproc/lut.h>
#include <imgproc/colorconversion.h>
#include "../misc/nearestneighborsplineinterpolator1D.h"
#include "../misc/linearsplineinterpolator1D.h"
#include "../misc/cubicsplineinterpolator1D.h"
#include <misc/util.h>

Filter::Filter() :
    mSplineInterpolatorHue(0),
    mSplineInterpolatorSaturation(0),
    mSplineInterpolatorLightness(0),
    mInterpolationModeHue(Smooth),
    mInterpolationModeSaturation(Smooth),
    mInterpolationModeLightness(Smooth),
    mIsInvertedHue(false),
    mIsInvertedSaturation(false),
    mIsInvertedLightness(false),
    mOutputMode(KeyedImage),
    mPreblurRadius(0.)
{
    mSplineInterpolatorHue = new CubicSplineInterpolator1D();
    mSplineInterpolatorHue->addKnot(.0, 0.);
    mSplineInterpolatorHue->addKnot(.33, 1.);
    mSplineInterpolatorHue->addKnot(.66, 0.);
    mSplineInterpolatorHue->setExtrapolationMode(Interpolator1D::ExtrapolationMode_Repeat,
                                                 Interpolator1D::ExtrapolationMode_Repeat);
    ((CubicSplineInterpolator1D*)mSplineInterpolatorHue)->setBoundaryConditions(
                                                          CubicSplineInterpolator1D::BoundaryConditions_Periodic,
                                                          CubicSplineInterpolator1D::BoundaryConditions_Periodic);
    makeLUT(ColorChannel_Hue);
    mSplineInterpolatorSaturation = new CubicSplineInterpolator1D();
    mSplineInterpolatorSaturation->addKnot(0., 0.);
    mSplineInterpolatorSaturation->addKnot(1., 1.);
    makeLUT(ColorChannel_Saturation);
    mSplineInterpolatorLightness = new CubicSplineInterpolator1D();
    mSplineInterpolatorLightness->addKnot(0., 0.);
    mSplineInterpolatorLightness->addKnot(.5, 1.);
    mSplineInterpolatorLightness->addKnot(1., 0.);
    makeLUT(ColorChannel_Lightness);
}

Filter::~Filter()
{
    if (mSplineInterpolatorHue)
        delete mSplineInterpolatorHue;
    if (mSplineInterpolatorSaturation)
        delete mSplineInterpolatorSaturation;
    if (mSplineInterpolatorLightness)
        delete mSplineInterpolatorLightness;
}

ImageFilter * Filter::clone()
{
    Filter * f = new Filter();

    if (f->mSplineInterpolatorHue)
        delete f->mSplineInterpolatorHue;
    f->mSplineInterpolatorHue = mSplineInterpolatorHue->clone();
    f->mInterpolationModeHue = mInterpolationModeHue;
    f->mIsInvertedHue = mIsInvertedHue;
    f->makeLUT(ColorChannel_Hue);

    if (f->mSplineInterpolatorSaturation)
        delete f->mSplineInterpolatorSaturation;
    f->mSplineInterpolatorSaturation = mSplineInterpolatorSaturation->clone();
    f->mInterpolationModeSaturation = mInterpolationModeSaturation;
    f->mIsInvertedSaturation = mIsInvertedSaturation;
    f->makeLUT(ColorChannel_Saturation);

    if (f->mSplineInterpolatorLightness)
        delete f->mSplineInterpolatorLightness;
    f->mSplineInterpolatorLightness = mSplineInterpolatorLightness->clone();
    f->mInterpolationModeLightness = mInterpolationModeLightness;
    f->mIsInvertedLightness = mIsInvertedLightness;
    f->makeLUT(ColorChannel_Lightness);

    f->mOutputMode = mOutputMode;
    f->mPreblurRadius = mPreblurRadius;

    return f;
}

extern "C" QHash<QString, QString> getIBPPluginInfo();
QHash<QString, QString> Filter::info()
{
    return getIBPPluginInfo();
}

QImage Filter::process(const QImage & inputImage)
{
    if (inputImage.isNull() || inputImage.format() != QImage::Format_ARGB32)
        return inputImage;

    QImage i = QImage(inputImage.width(), inputImage.height(), QImage::Format_ARGB32);
    register int totalPixels = i.width() * i.height();
    HSL * hslImage = (HSL *)malloc(totalPixels * sizeof(HSL));
    register BGRA * bits = (BGRA*)inputImage.bits();
    register BGRA * bits2 = (BGRA*)i.bits();
    register HSL * bitsHSL = hslImage;

    if (qFuzzyIsNull(mPreblurRadius))
        convertBGRToHSL(inputImage.bits(), (unsigned char *)hslImage, totalPixels);
    else
    {
        QImage iBlurred = QImage(inputImage.width(), inputImage.height(), QImage::Format_ARGB32);
        cv::Mat mInput(inputImage.height(), inputImage.width(), CV_8UC4, (void *)inputImage.bits());
        cv::Mat mBlurred(iBlurred.height(), iBlurred.width(), CV_8UC4, iBlurred.bits());
        double sigma = (mPreblurRadius + .5) / 2.45;
        cv::GaussianBlur(mInput, mBlurred, cv::Size(0, 0), sigma);
        convertBGRToHSL(iBlurred.bits(), (unsigned char *)hslImage, totalPixels);
    }

    if (mOutputMode == KeyedImage)
        while (totalPixels--)
        {
            bits2->r = bits->r;
            bits2->g = bits->g;
            bits2->b = bits->b;
            bits2->a = lut01[bits->a][255 -
                       lut01[mLutHue[bitsHSL->h]][lut01[mLutSaturation[bitsHSL->s]][mLutLightness[bitsHSL->l]]]];;
            bits++;
            bits2++;
            bitsHSL++;
        }
    else
        while (totalPixels--)
        {
            bits2->r = bits2->g = bits2->b =
                    lut01[bits->a][255 -
                    lut01[mLutHue[bitsHSL->h]][
                    lut01[mLutSaturation[bitsHSL->s]][
                    mLutLightness[bitsHSL->l]]]];
            bits2->a = 255;
            bits++;
            bits2++;
            bitsHSL++;
        }

    free(hslImage);

    return i;
}

bool Filter::loadParameters(QSettings &s)
{
    QString interpolationModeStr;
    InterpolationMode interpolationModeHue, interpolationModeSaturation, interpolationModeLightness;
    QString knotsStr;
    QStringList knotsList;
    QStringList knotList;
    double x, y;
    Interpolator1DKnots knotsHue, knotsSaturation, knotsLightness;
    bool isInvertedHue, isInvertedSaturation, isInvertedLightness;
    QString outputModeStr;
    OutputMode outputMode;
    double preblurRadius;
    bool ok;

    interpolationModeStr = s.value("hueinterpolationmode", "smooth").toString();
    if (interpolationModeStr == "flat")
        interpolationModeHue = Flat;
    else if (interpolationModeStr == "linear")
        interpolationModeHue = Linear;
    else if (interpolationModeStr == "smooth")
        interpolationModeHue = Smooth;
    else
        return false;
    interpolationModeStr = s.value("saturationinterpolationmode", "smooth").toString();
    if (interpolationModeStr == "flat")
        interpolationModeSaturation = Flat;
    else if (interpolationModeStr == "linear")
        interpolationModeSaturation = Linear;
    else if (interpolationModeStr == "smooth")
        interpolationModeSaturation = Smooth;
    else
        return false;
    interpolationModeStr = s.value("lightnessinterpolationmode", "smooth").toString();
    if (interpolationModeStr == "flat")
        interpolationModeLightness = Flat;
    else if (interpolationModeStr == "linear")
        interpolationModeLightness = Linear;
    else if (interpolationModeStr == "smooth")
        interpolationModeLightness = Smooth;
    else
        return false;

    knotsStr = s.value("hueknots", "0.0 0.0, 1.0 1.0").toString();
    knotsList = knotsStr.split(QRegularExpression("\\s*,\\s*"), Qt::SkipEmptyParts);
    if (knotsList.size() < 2)
        return false;
    for (int i = 0; i < knotsList.size(); i++)
    {
        knotList = knotsList.at(i).split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (knotList.size() != 2)
            return false;
        x = knotList.at(0).toDouble(&ok);
        if (!ok || x < 0. || x > 1.)
            return false;
        y = knotList.at(1).toDouble(&ok);
        if (!ok || y < 0. || y > 1.)
            return false;
        knotsHue.append(Interpolator1DKnot(x, y));
    }
    // append last knot to make it periodic
    knotsHue.append(Interpolator1DKnot(IBP_maximum(1.001, knotsHue.first().x() + 1.), knotsHue.first().y()));
    knotsStr = s.value("saturationknots", "0.0 0.0, 1.0 1.0").toString();
    knotsList = knotsStr.split(QRegularExpression("\\s*,\\s*"), Qt::SkipEmptyParts);
    if (knotsList.size() < 2)
        return false;
    for (int i = 0; i < knotsList.size(); i++)
    {
        knotList = knotsList.at(i).split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (knotList.size() != 2)
            return false;
        x = knotList.at(0).toDouble(&ok);
        if (!ok || x < 0. || x > 1.)
            return false;
        y = knotList.at(1).toDouble(&ok);
        if (!ok || y < 0. || y > 1.)
            return false;
        knotsSaturation.append(Interpolator1DKnot(x, y));
    }
    knotsStr = s.value("lightnessknots", "0.0 0.0, 1.0 1.0").toString();
    knotsList = knotsStr.split(QRegularExpression("\\s*,\\s*"), Qt::SkipEmptyParts);
    if (knotsList.size() < 2)
        return false;
    for (int i = 0; i < knotsList.size(); i++)
    {
        knotList = knotsList.at(i).split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (knotList.size() != 2)
            return false;
        x = knotList.at(0).toDouble(&ok);
        if (!ok || x < 0. || x > 1.)
            return false;
        y = knotList.at(1).toDouble(&ok);
        if (!ok || y < 0. || y > 1.)
            return false;
        knotsLightness.append(Interpolator1DKnot(x, y));
    }

    isInvertedHue = s.value("hueisinverted", false).toBool();
    isInvertedSaturation = s.value("saturationisinverted", false).toBool();
    isInvertedLightness = s.value("lightnessisinverted", false).toBool();

    outputModeStr = s.value("outputmode", "keyedimage").toString();
    if (outputModeStr == "keyedimage")
        outputMode = KeyedImage;
    else if (outputModeStr == "matte")
        outputMode = Matte;
    else
        return false;

    preblurRadius = s.value("preblurradius", 0.).toDouble(&ok);
    if (!ok || preblurRadius < 0. || preblurRadius > 100.)
        return false;

    setHueInterpolationMode(interpolationModeHue);
    setHueKnots(knotsHue);
    setHueInverted(isInvertedHue);
    setSaturationInterpolationMode(interpolationModeSaturation);
    setSaturationKnots(knotsSaturation);
    setSaturationInverted(isInvertedSaturation);
    setLightnessInterpolationMode(interpolationModeLightness);
    setLightnessKnots(knotsLightness);
    setLightnessInverted(isInvertedLightness);
    setOutputMode(outputMode);
    setPreblurRadius(preblurRadius);

    return true;
}

bool Filter::saveParameters(QSettings &s)
{
    s.setValue("hueinterpolationmode", mInterpolationModeHue == Flat ? "flat" :
                                       mInterpolationModeHue == Linear ? "linear" : "smooth");
    s.setValue("saturationinterpolationmode", mInterpolationModeSaturation == Flat ? "flat" :
                                              mInterpolationModeSaturation == Linear ? "linear" : "smooth");
    s.setValue("lightnessinterpolationmode", mInterpolationModeLightness == Flat ? "flat" :
                                             mInterpolationModeLightness == Linear ? "linear" : "smooth");

    Interpolator1DKnots knots = mSplineInterpolatorHue->knots();
    QString knotsStr = "";
    for (int i = 0; i < knots.size() - 1; i++)
    {
        knotsStr += QString::number(knots.at(i).x(), 'f', 2) + " " +
                    QString::number(knots.at(i).y(), 'f', 2);
        if (i < knots.size() - 2)
            knotsStr += ", ";
    }
    s.setValue("hueknots", knotsStr);
    knots = mSplineInterpolatorSaturation->knots();
    knotsStr = "";
    for (int i = 0; i < knots.size(); i++)
    {
        knotsStr += QString::number(knots.at(i).x(), 'f', 2) + " " +
                    QString::number(knots.at(i).y(), 'f', 2);
        if (i < knots.size() - 1)
            knotsStr += ", ";
    }
    s.setValue("saturationknots", knotsStr);
    knots = mSplineInterpolatorLightness->knots();
    knotsStr = "";
    for (int i = 0; i < knots.size(); i++)
    {
        knotsStr += QString::number(knots.at(i).x(), 'f', 2) + " " +
                    QString::number(knots.at(i).y(), 'f', 2);
        if (i < knots.size() - 1)
            knotsStr += ", ";
    }
    s.setValue("lightnessknots", knotsStr);

    s.setValue("hueisinverted", mIsInvertedHue);
    s.setValue("saturationisinverted", mIsInvertedSaturation);
    s.setValue("lightnessisinverted", mIsInvertedLightness);

    s.setValue("outputmode", mOutputMode == KeyedImage ? "keyedimage" : "matte");

    s.setValue("preblurradius", mPreblurRadius);

    return true;
}

QWidget *Filter::widget(QWidget *parent)
{
    FilterWidget * fw = new FilterWidget(parent);
    fw->setHueInterpolationMode(mInterpolationModeHue);
    fw->setHueKnots(mSplineInterpolatorHue->knots());
    fw->setHueInverted(mIsInvertedHue);
    fw->setSaturationInterpolationMode(mInterpolationModeSaturation);
    fw->setSaturationKnots(mSplineInterpolatorSaturation->knots());
    fw->setSaturationInverted(mIsInvertedSaturation);
    fw->setLightnessInterpolationMode(mInterpolationModeLightness);
    fw->setLightnessKnots(mSplineInterpolatorLightness->knots());
    fw->setLightnessInverted(mIsInvertedLightness);
    fw->setOutputMode(mOutputMode);
    fw->setPreblurRadius(mPreblurRadius);

    connect(this, SIGNAL(hueInterpolationModeChanged(Filter::InterpolationMode)),
            fw, SLOT(setHueInterpolationMode(Filter::InterpolationMode)));
    connect(this, SIGNAL(hueKnotsChanged(Interpolator1DKnots)),
            fw, SLOT(setHueKnots(Interpolator1DKnots)));
    connect(this, SIGNAL(hueInvertedChanged(bool)),
            fw, SLOT(setHueInverted(bool)));
    connect(this, SIGNAL(saturationInterpolationModeChanged(Filter::InterpolationMode)),
            fw, SLOT(setSaturationInterpolationMode(Filter::InterpolationMode)));
    connect(this, SIGNAL(saturationKnotsChanged(Interpolator1DKnots)),
            fw, SLOT(setSaturationKnots(Interpolator1DKnots)));
    connect(this, SIGNAL(saturationInvertedChanged(bool)),
            fw, SLOT(setSaturationInverted(bool)));
    connect(this, SIGNAL(lightnessInterpolationModeChanged(Filter::InterpolationMode)),
            fw, SLOT(setLightnessInterpolationMode(Filter::InterpolationMode)));
    connect(this, SIGNAL(lightnessKnotsChanged(Interpolator1DKnots)),
            fw, SLOT(setLightnessKnots(Interpolator1DKnots)));
    connect(this, SIGNAL(lightnessInvertedChanged(bool)),
            fw, SLOT(setLightnessInverted(bool)));
    connect(this, SIGNAL(outputModeChanged(Filter::OutputMode)),
            fw, SLOT(setOutputMode(Filter::OutputMode)));
    connect(this, SIGNAL(preblurRadiusChanged(double)),
            fw, SLOT(setPreblurRadius(double)));

    connect(fw, SIGNAL(hueInterpolationModeChanged(Filter::InterpolationMode)),
            this, SLOT(setHueInterpolationMode(Filter::InterpolationMode)));
    connect(fw, SIGNAL(hueKnotsChanged(Interpolator1DKnots)),
            this, SLOT(setHueKnots(Interpolator1DKnots)));
    connect(fw, SIGNAL(hueInvertedChanged(bool)),
            this, SLOT(setHueInverted(bool)));
    connect(fw, SIGNAL(saturationInterpolationModeChanged(Filter::InterpolationMode)),
            this, SLOT(setSaturationInterpolationMode(Filter::InterpolationMode)));
    connect(fw, SIGNAL(saturationKnotsChanged(Interpolator1DKnots)),
            this, SLOT(setSaturationKnots(Interpolator1DKnots)));
    connect(fw, SIGNAL(saturationInvertedChanged(bool)),
            this, SLOT(setSaturationInverted(bool)));
    connect(fw, SIGNAL(lightnessInterpolationModeChanged(Filter::InterpolationMode)),
            this, SLOT(setLightnessInterpolationMode(Filter::InterpolationMode)));
    connect(fw, SIGNAL(lightnessKnotsChanged(Interpolator1DKnots)),
            this, SLOT(setLightnessKnots(Interpolator1DKnots)));
    connect(fw, SIGNAL(lightnessInvertedChanged(bool)),
            this, SLOT(setLightnessInverted(bool)));
    connect(fw, SIGNAL(outputModeChanged(Filter::OutputMode)),
            this, SLOT(setOutputMode(Filter::OutputMode)));
    connect(fw, SIGNAL(preblurRadiusChanged(double)),
            this, SLOT(setPreblurRadius(double)));

    return fw;
}

void Filter::setHueKnots(const Interpolator1DKnots &k)
{
    if (mSplineInterpolatorHue->knots() == k)
        return;
    mSplineInterpolatorHue->setKnots(k);
    makeLUT(ColorChannel_Hue);
    emit hueKnotsChanged(k);
    emit parametersChanged();
}

void Filter::setHueInterpolationMode(Filter::InterpolationMode im)
{
    if (mInterpolationModeHue == im)
        return;

    mInterpolationModeHue = im;

    Interpolator1D * tmp = mSplineInterpolatorHue;

    if (im == Flat)
        mSplineInterpolatorHue = new NearestNeighborSplineInterpolator1D();
    else if (im == Linear)
        mSplineInterpolatorHue = new LinearSplineInterpolator1D();
    else
        mSplineInterpolatorHue = new CubicSplineInterpolator1D();

    mSplineInterpolatorHue->setKnots(tmp->knots());

    delete tmp;

    makeLUT(ColorChannel_Hue);

    emit hueInterpolationModeChanged(im);
    emit parametersChanged();
}

void Filter::setHueInverted(bool i)
{
    if (mIsInvertedHue == i)
        return;

    mIsInvertedHue = i;

    makeLUT(ColorChannel_Hue);

    emit hueInvertedChanged(i);
    emit parametersChanged();
}

void Filter::setSaturationKnots(const Interpolator1DKnots &k)
{
    if (mSplineInterpolatorSaturation->knots() == k)
        return;
    mSplineInterpolatorSaturation->setKnots(k);
    makeLUT(ColorChannel_Saturation);
    emit saturationKnotsChanged(k);
    emit parametersChanged();
}

void Filter::setSaturationInterpolationMode(Filter::InterpolationMode im)
{
    if (mInterpolationModeSaturation == im)
        return;

    mInterpolationModeSaturation = im;

    Interpolator1D * tmp = mSplineInterpolatorSaturation;

    if (im == Flat)
        mSplineInterpolatorSaturation = new NearestNeighborSplineInterpolator1D();
    else if (im == Linear)
        mSplineInterpolatorSaturation = new LinearSplineInterpolator1D();
    else
        mSplineInterpolatorSaturation = new CubicSplineInterpolator1D();

    mSplineInterpolatorSaturation->setKnots(tmp->knots());

    delete tmp;

    makeLUT(ColorChannel_Saturation);

    emit saturationInterpolationModeChanged(im);
    emit parametersChanged();
}

void Filter::setSaturationInverted(bool i)
{
    if (mIsInvertedSaturation == i)
        return;

    mIsInvertedSaturation = i;

    makeLUT(ColorChannel_Saturation);

    emit saturationInvertedChanged(i);
    emit parametersChanged();
}

void Filter::setLightnessKnots(const Interpolator1DKnots &k)
{
    if (mSplineInterpolatorLightness->knots() == k)
        return;
    mSplineInterpolatorLightness->setKnots(k);
    makeLUT(ColorChannel_Lightness);
    emit lightnessKnotsChanged(k);
    emit parametersChanged();
}

void Filter::setLightnessInterpolationMode(Filter::InterpolationMode im)
{
    if (mInterpolationModeLightness == im)
        return;

    mInterpolationModeLightness = im;

    Interpolator1D * tmp = mSplineInterpolatorLightness;

    if (im == Flat)
        mSplineInterpolatorLightness = new NearestNeighborSplineInterpolator1D();
    else if (im == Linear)
        mSplineInterpolatorLightness = new LinearSplineInterpolator1D();
    else
        mSplineInterpolatorLightness = new CubicSplineInterpolator1D();

    mSplineInterpolatorLightness->setKnots(tmp->knots());

    delete tmp;

    makeLUT(ColorChannel_Lightness);

    emit lightnessInterpolationModeChanged(im);
    emit parametersChanged();
}

void Filter::setLightnessInverted(bool i)
{
    if (mIsInvertedLightness == i)
        return;

    mIsInvertedLightness = i;

    makeLUT(ColorChannel_Lightness);

    emit lightnessInvertedChanged(i);
    emit parametersChanged();
}

void Filter::setOutputMode(Filter::OutputMode om)
{
    if (mOutputMode == om)
        return;

    mOutputMode = om;

    emit outputModeChanged(om);
    emit parametersChanged();
}

void Filter::setPreblurRadius(double pbr)
{
    if (qFuzzyCompare(pbr, mPreblurRadius))
        return;

    mPreblurRadius = pbr;

    emit preblurRadiusChanged(pbr);
    emit parametersChanged();
}

void Filter::makeLUT(ColorChannel c)
{
    unsigned char * lut = c == ColorChannel_Hue ? mLutHue :
                          c == ColorChannel_Saturation ? mLutSaturation : mLutLightness;
    Interpolator1D * splineInterpolator = c == ColorChannel_Hue ? mSplineInterpolatorHue :
                                              c == ColorChannel_Saturation ? mSplineInterpolatorSaturation :
                                              mSplineInterpolatorLightness;
    bool inverted = c == ColorChannel_Hue ? mIsInvertedHue :
                    c == ColorChannel_Saturation ? mIsInvertedSaturation : mIsInvertedLightness;

    if (inverted)
        for (int i = 0; i < 256; i++)
            lut[i] = 255 - IBP_clamp(0, round(splineInterpolator->f(i / 255.) * 255.), 255);
    else
        for (int i = 0; i < 256; i++)
            lut[i] = IBP_clamp(0, round(splineInterpolator->f(i / 255.) * 255.), 255);
}
