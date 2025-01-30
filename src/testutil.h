#ifndef IBP_IMGPROC_TESTUTIL_H
#define IBP_IMGPROC_TESTUTIL_H

#include <QString>
#include <QImage>
#include "imagefilter.h"

namespace ibp {
namespace imgproc {

class TestUtil {
public:
    static bool runFilterTests(const QString& imagePath);

private:
    static QString generateOutputPath(const QString& inputPath, const QString& filterId);
    static bool applyAndSaveFilter(ImageFilter* filter, const QImage& input, const QString& outputPath);
};

} // namespace imgproc
} // namespace ibp

#endif // IBP_IMGPROC_TESTUTIL_H 