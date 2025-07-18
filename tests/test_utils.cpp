// this_file: tests/test_utils.cpp

#include "test_utils.h"
#include <QApplication>
#include <QImage>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QRandomGenerator>
#include <QBuffer>
#include <QImageWriter>
#include <cmath>

namespace ibp {
namespace test {

// Static member definitions
QList<QString> TestUtils::tempFiles;
QList<QString> TestUtils::tempDirs;
std::unique_ptr<QApplication> IBPTestBase::app;

std::unique_ptr<QApplication> TestUtils::initializeQtApp() {
    // Check if QApplication already exists
    if (QCoreApplication::instance()) {
        return nullptr;
    }
    
    // Create minimal QApplication for testing
    static int argc = 1;
    static char appName[] = "IBPTest";
    static char* argv[] = { appName, nullptr };
    
    auto app = std::make_unique<QApplication>(argc, argv);
    app->setApplicationName("IBPTest");
    app->setOrganizationName("IBPTest");
    
    return app;
}

QImage TestUtils::createTestImage(int width, int height, QColor color) {
    QImage image(width, height, QImage::Format_RGB32);
    image.fill(color);
    return image;
}

QString TestUtils::createTempFile(const QByteArray& content, const QString& suffix) {
    QTemporaryFile* tempFile = new QTemporaryFile();
    tempFile->setFileTemplate(QDir::tempPath() + "/ibp_test_XXXXXX" + suffix);
    
    if (!tempFile->open()) {
        delete tempFile;
        return QString();
    }
    
    tempFile->write(content);
    tempFile->close();
    
    QString filePath = tempFile->fileName();
    tempFiles.append(filePath);
    
    // Keep the file around by not deleting the QTemporaryFile object
    tempFile->setAutoRemove(false);
    
    return filePath;
}

QString TestUtils::createTempDir() {
    QTemporaryDir* tempDir = new QTemporaryDir();
    if (!tempDir->isValid()) {
        delete tempDir;
        return QString();
    }
    
    QString dirPath = tempDir->path();
    tempDirs.append(dirPath);
    
    // Keep the directory around
    tempDir->setAutoRemove(false);
    
    return dirPath;
}

bool TestUtils::compareImages(const QImage& img1, const QImage& img2, int tolerance) {
    if (img1.size() != img2.size()) {
        return false;
    }
    
    if (img1.format() != img2.format()) {
        // Convert to common format for comparison
        QImage converted1 = img1.convertToFormat(QImage::Format_RGB32);
        QImage converted2 = img2.convertToFormat(QImage::Format_RGB32);
        return compareImages(converted1, converted2, tolerance);
    }
    
    for (int y = 0; y < img1.height(); ++y) {
        for (int x = 0; x < img1.width(); ++x) {
            QRgb pixel1 = img1.pixel(x, y);
            QRgb pixel2 = img2.pixel(x, y);
            
            int rDiff = abs(qRed(pixel1) - qRed(pixel2));
            int gDiff = abs(qGreen(pixel1) - qGreen(pixel2));
            int bDiff = abs(qBlue(pixel1) - qBlue(pixel2));
            
            if (rDiff > tolerance || gDiff > tolerance || bDiff > tolerance) {
                return false;
            }
        }
    }
    
    return true;
}

QString TestUtils::getTestDataDir() {
    // Look for test data directory relative to the executable
    QString testDataPath = QCoreApplication::applicationDirPath() + "/test_data";
    
    if (QDir(testDataPath).exists()) {
        return testDataPath;
    }
    
    // Try relative to source directory
    testDataPath = QString(CMAKE_SOURCE_DIR) + "/tests/test_data";
    if (QDir(testDataPath).exists()) {
        return testDataPath;
    }
    
    // Create temporary test data directory
    QString tempTestData = createTempDir() + "/test_data";
    QDir().mkpath(tempTestData);
    
    return tempTestData;
}

QImage TestUtils::loadTestImage(const QString& filename) {
    QString testDataDir = getTestDataDir();
    QString imagePath = testDataDir + "/" + filename;
    
    QImage image(imagePath);
    if (image.isNull()) {
        // Create a default test image if file doesn't exist
        image = createTestImage(100, 100, Qt::blue);
    }
    
    return image;
}

QString TestUtils::saveImageToTempFile(const QImage& image, const QString& format) {
    QTemporaryFile* tempFile = new QTemporaryFile();
    tempFile->setFileTemplate(QDir::tempPath() + "/ibp_test_image_XXXXXX." + format.toLower());
    
    if (!tempFile->open()) {
        delete tempFile;
        return QString();
    }
    
    QImageWriter writer(tempFile, format.toLatin1());
    if (!writer.write(image)) {
        delete tempFile;
        return QString();
    }
    
    tempFile->close();
    QString filePath = tempFile->fileName();
    tempFiles.append(filePath);
    tempFile->setAutoRemove(false);
    
    return filePath;
}

QString TestUtils::randomString(int length) {
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    QString result;
    
    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        result.append(chars.at(index));
    }
    
    return result;
}

bool TestUtils::approximatelyEqual(double a, double b, double epsilon) {
    return std::abs(a - b) < epsilon;
}

void TestUtils::cleanup() {
    // Remove temporary files
    for (const QString& filePath : tempFiles) {
        QFile::remove(filePath);
    }
    tempFiles.clear();
    
    // Remove temporary directories
    for (const QString& dirPath : tempDirs) {
        QDir dir(dirPath);
        if (dir.exists()) {
            dir.removeRecursively();
        }
    }
    tempDirs.clear();
}

// IBPTestBase implementation
void IBPTestBase::SetUpTestSuite() {
    app = TestUtils::initializeQtApp();
}

void IBPTestBase::TearDownTestSuite() {
    TestUtils::cleanup();
    app.reset();
}

void IBPTestBase::SetUp() {
    // Base setup for all tests
}

void IBPTestBase::TearDown() {
    // Base teardown for all tests
}

// ImageProcessingTest implementation
void ImageProcessingTest::SetUp() {
    IBPTestBase::SetUp();
    
    testImage = TestUtils::createTestImage(100, 100, Qt::white);
    tempDir = TestUtils::createTempDir();
}

void ImageProcessingTest::TearDown() {
    IBPTestBase::TearDown();
}

// PluginTest implementation
void PluginTest::SetUp() {
    IBPTestBase::SetUp();
    
    pluginDir = TestUtils::createTempDir() + "/plugins";
    QDir().mkpath(pluginDir);
}

void PluginTest::TearDown() {
    IBPTestBase::TearDown();
}

} // namespace test
} // namespace ibp