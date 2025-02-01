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

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFileInfo>
#ifdef USE_USER_CONFIGURATION_FOLDER
#include <QStandardPaths>
#endif

#include "mainwindow.h"
#include "../misc/configurationmanager.h"
#include "../widgets/style.h"

using namespace ibp::misc;
using namespace ibp::widgets;

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(widgets);

    QApplication a(argc, argv);
    a.setApplicationName("Image Batch Processor");
    a.setApplicationDisplayName("Image Batch Processor");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("pub");
    a.setOrganizationDomain("twardoch.github.io");

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Image Batch Processor"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption filterListOption(QStringList() << "l" << "list",
                                        QObject::tr("Image filter list file to load."),
                                        "file");
    parser.addOption(filterListOption);

    QCommandLineOption inputImageOption(QStringList() << "i" << "input",
                                        QObject::tr("Input image file."),
                                        "file");
    parser.addOption(inputImageOption);

    QCommandLineOption outputImageOption(QStringList() << "o" << "output",
                                         QObject::tr("Output image file."),
                                         "file");
    parser.addOption(outputImageOption);

    // Add support for positional arguments (additional image files)
    parser.addPositionalArgument("images", QObject::tr("Additional image files to open."));

    parser.process(a);

    setIBPStyle();

#ifdef USE_USER_CONFIGURATION_FOLDER
    ConfigurationManager::setFolder(QStandardPaths::writableLocation(GenericDataLocation) + "/ibp");
#else
    ConfigurationManager::setFolder(a.applicationDirPath() + "/settings");
#endif
    ConfigurationManager::setFileName("imagebatchprocessor.cfg");

    // CLI mode - process image and exit
    if (parser.isSet(outputImageOption))
    {
        if (!parser.isSet(inputImageOption))
        {
            qWarning().noquote() << "Error: Input image (-i) must be specified when using output (-o)";
            return 1;
        }

        QString filterListFile = parser.isSet(filterListOption) ? parser.value(filterListOption) : QString();
        QString inputImageFile = parser.value(inputImageOption);
        QString outputImageFile = parser.value(outputImageOption);

        qDebug() << "Processing with:";
        if (!filterListFile.isEmpty())
            qDebug() << "  Filter list:" << filterListFile;
        qDebug() << "  Input image:" << inputImageFile;
        qDebug() << "  Output image:" << outputImageFile;

        if (!filterListFile.isEmpty() && !QFileInfo(filterListFile).exists())
        {
            qWarning().noquote() << QString("Error: Filter list file does not exist: %1").arg(filterListFile);
            return 1;
        }

        if (!QFileInfo(inputImageFile).exists())
        {
            qWarning().noquote() << QString("Error: Input image file does not exist: %1").arg(inputImageFile);
            return 1;
        }

        MainWindow w;
        if (!w.applyFiltersAndSave(filterListFile, inputImageFile, outputImageFile))
        {
            qWarning().noquote() << "Error: Failed to process image.";
            return 1;
        }

        return 0;
    }

    // GUI mode
    MainWindow w;
    w.setWindowTitle(a.applicationDisplayName());

    // Load initial image if specified
    if (parser.isSet(inputImageOption))
    {
        QString inputImageFile = parser.value(inputImageOption);
        if (!QFileInfo(inputImageFile).exists())
        {
            qWarning().noquote() << QString("Warning: Input image file does not exist: %1").arg(inputImageFile);
        }
        else
        {
            w.viewEditLoadInputImage(inputImageFile);
        }
    }

    // Load filter list if specified
    if (parser.isSet(filterListOption))
    {
        QString filterListFile = parser.value(filterListOption);
        if (!QFileInfo(filterListFile).exists())
        {
            qWarning().noquote() << QString("Warning: Filter list file does not exist: %1").arg(filterListFile);
        }
        else
        {
            w.viewEditLoadImageFilterList(filterListFile);
        }
    }

    // Load additional images from positional arguments
    const QStringList additionalImages = parser.positionalArguments();
    for (const QString& imagePath : additionalImages)
    {
        if (!QFileInfo(imagePath).exists())
        {
            qWarning().noquote() << QString("Warning: Additional image file does not exist: %1").arg(imagePath);
            continue;
        }
        // TODO: Implement a method to open additional images in new tabs/windows
        qDebug() << "Opening additional image:" << imagePath;
    }

    w.show();
    return a.exec();
}
