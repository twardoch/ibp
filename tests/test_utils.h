// this_file: tests/test_utils.h

#ifndef IBP_TEST_UTILS_H
#define IBP_TEST_UTILS_H

#include <gtest/gtest.h>
#include <QApplication>
#include <QImage>
#include <QTemporaryFile>
#include <QDir>
#include <string>
#include <memory>

namespace ibp {
namespace test {

/**
 * @brief Test utilities for Image Batch Processor
 */
class TestUtils {
public:
    /**
     * @brief Initialize Qt application for testing
     * @return QApplication instance
     */
    static std::unique_ptr<QApplication> initializeQtApp();
    
    /**
     * @brief Create a test image with specified dimensions and color
     * @param width Image width
     * @param height Image height
     * @param color Fill color (default: white)
     * @return QImage instance
     */
    static QImage createTestImage(int width = 100, int height = 100, QColor color = Qt::white);
    
    /**
     * @brief Create a temporary file with specified content
     * @param content File content
     * @param suffix File suffix (e.g., ".txt", ".jpg")
     * @return Temporary file path
     */
    static QString createTempFile(const QByteArray& content, const QString& suffix = ".tmp");
    
    /**
     * @brief Create a temporary directory
     * @return Temporary directory path
     */
    static QString createTempDir();
    
    /**
     * @brief Compare two QImages for equality
     * @param img1 First image
     * @param img2 Second image
     * @param tolerance Pixel tolerance (0-255)
     * @return True if images are equal within tolerance
     */
    static bool compareImages(const QImage& img1, const QImage& img2, int tolerance = 0);
    
    /**
     * @brief Get test data directory path
     * @return Path to test data directory
     */
    static QString getTestDataDir();
    
    /**
     * @brief Load test image from resources
     * @param filename Image filename in test resources
     * @return QImage instance
     */
    static QImage loadTestImage(const QString& filename);
    
    /**
     * @brief Save image to temporary file
     * @param image Image to save
     * @param format Image format (e.g., "PNG", "JPEG")
     * @return Temporary file path
     */
    static QString saveImageToTempFile(const QImage& image, const QString& format = "PNG");
    
    /**
     * @brief Generate random string
     * @param length String length
     * @return Random string
     */
    static QString randomString(int length = 10);
    
    /**
     * @brief Check if two floating point values are approximately equal
     * @param a First value
     * @param b Second value
     * @param epsilon Tolerance
     * @return True if values are approximately equal
     */
    static bool approximatelyEqual(double a, double b, double epsilon = 1e-9);
    
    /**
     * @brief Cleanup temporary files and directories
     */
    static void cleanup();

private:
    static QList<QString> tempFiles;
    static QList<QString> tempDirs;
};

/**
 * @brief Base class for IBP tests with common setup
 */
class IBPTestBase : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    
    static std::unique_ptr<QApplication> app;
};

/**
 * @brief Test fixture for image processing tests
 */
class ImageProcessingTest : public IBPTestBase {
protected:
    void SetUp() override;
    void TearDown() override;
    
    QImage testImage;
    QString tempDir;
};

/**
 * @brief Test fixture for plugin tests
 */
class PluginTest : public IBPTestBase {
protected:
    void SetUp() override;
    void TearDown() override;
    
    QString pluginDir;
};

} // namespace test
} // namespace ibp

// Custom Google Test matchers
MATCHER_P(ImageEquals, expected, "Image equality matcher") {
    return ibp::test::TestUtils::compareImages(arg, expected);
}

MATCHER_P2(ImageEqualsWithTolerance, expected, tolerance, "Image equality matcher with tolerance") {
    return ibp::test::TestUtils::compareImages(arg, expected, tolerance);
}

#endif // IBP_TEST_UTILS_H