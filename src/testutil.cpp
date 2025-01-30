#include "testutil.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include "../plugins/pluginmanager.h"

namespace ibp {
namespace imgproc {

bool TestUtil::runFilterTests(const QString& imagePath)
{
    QImage inputImage(imagePath);
    if (inputImage.isNull()) {
        qWarning() << "Failed to load image:" << imagePath;
        return false;
    }

    // Get all available filters
    auto& pluginManager = plugins::PluginManager::instance();
    auto filters = pluginManager.filters();

    if (filters.isEmpty()) {
        qWarning() << "No filters available";
        return false;
    }

    bool success = true;
    for (auto filter : filters) {
        QString outputPath = generateOutputPath(imagePath, filter->id());
        qInfo() << "Applying filter:" << filter->id();
        qInfo() << "Saving to:" << outputPath;

        filter->randomizeParameters();
        
        if (!applyAndSaveFilter(filter, inputImage, outputPath)) {
            qWarning() << "Failed to process or save image for filter:" << filter->id();
            success = false;
        }
    }

    return success;
}

QString TestUtil::generateOutputPath(const QString& inputPath, const QString& filterId)
{
    QFileInfo fileInfo(inputPath);
    QString newName = QString("%1-%2.jpg").arg(fileInfo.baseName(), filterId);
    return QDir(fileInfo.absolutePath()).filePath(newName);
}

bool TestUtil::applyAndSaveFilter(ImageFilter* filter, const QImage& input, const QString& outputPath)
{
    QImage output = input;
    if (!filter->process(output)) {
        return false;
    }
    return output.save(outputPath, "JPG");
}

} // namespace imgproc
} // namespace ibp 