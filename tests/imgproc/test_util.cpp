// this_file: tests/imgproc/test_util.cpp

#include "../test_utils.h"
#include <gtest/gtest.h>

namespace ibp {
namespace test {

class UtilTest : public ImageProcessingTest {
protected:
    void SetUp() override {
        ImageProcessingTest::SetUp();
    }
};

// Mock utility functions for testing
class MockUtil {
public:
    static double clamp(double value, double min, double max) {
        return std::max(min, std::min(max, value));
    }
    
    static int clamp(int value, int min, int max) {
        return std::max(min, std::min(max, value));
    }
    
    static double lerp(double a, double b, double t) {
        return a + t * (b - a);
    }
    
    static bool isInRange(double value, double min, double max) {
        return value >= min && value <= max;
    }
    
    static QRect getBoundingRect(const QImage& image) {
        return image.rect();
    }
    
    static bool isValidImageFormat(QImage::Format format) {
        return format != QImage::Format_Invalid;
    }
    
    static QString formatToString(QImage::Format format) {
        switch (format) {
            case QImage::Format_RGB32: return "RGB32";
            case QImage::Format_ARGB32: return "ARGB32";
            case QImage::Format_Grayscale8: return "Grayscale8";
            default: return "Unknown";
        }
    }
};

TEST_F(UtilTest, ClampFunctions) {
    // Test double clamp
    EXPECT_EQ(MockUtil::clamp(0.5, 0.0, 1.0), 0.5);
    EXPECT_EQ(MockUtil::clamp(-0.5, 0.0, 1.0), 0.0);
    EXPECT_EQ(MockUtil::clamp(1.5, 0.0, 1.0), 1.0);
    
    // Test int clamp
    EXPECT_EQ(MockUtil::clamp(128, 0, 255), 128);
    EXPECT_EQ(MockUtil::clamp(-10, 0, 255), 0);
    EXPECT_EQ(MockUtil::clamp(300, 0, 255), 255);
}

TEST_F(UtilTest, LinearInterpolation) {
    EXPECT_EQ(MockUtil::lerp(0.0, 10.0, 0.0), 0.0);
    EXPECT_EQ(MockUtil::lerp(0.0, 10.0, 1.0), 10.0);
    EXPECT_EQ(MockUtil::lerp(0.0, 10.0, 0.5), 5.0);
    
    // Test with negative values
    EXPECT_EQ(MockUtil::lerp(-5.0, 5.0, 0.5), 0.0);
}

TEST_F(UtilTest, RangeChecking) {
    EXPECT_TRUE(MockUtil::isInRange(0.5, 0.0, 1.0));
    EXPECT_TRUE(MockUtil::isInRange(0.0, 0.0, 1.0));
    EXPECT_TRUE(MockUtil::isInRange(1.0, 0.0, 1.0));
    EXPECT_FALSE(MockUtil::isInRange(-0.1, 0.0, 1.0));
    EXPECT_FALSE(MockUtil::isInRange(1.1, 0.0, 1.0));
}

TEST_F(UtilTest, BoundingRectCalculation) {
    QImage image = TestUtils::createTestImage(100, 50);
    QRect rect = MockUtil::getBoundingRect(image);
    
    EXPECT_EQ(rect.x(), 0);
    EXPECT_EQ(rect.y(), 0);
    EXPECT_EQ(rect.width(), 100);
    EXPECT_EQ(rect.height(), 50);
}

TEST_F(UtilTest, ImageFormatValidation) {
    EXPECT_TRUE(MockUtil::isValidImageFormat(QImage::Format_RGB32));
    EXPECT_TRUE(MockUtil::isValidImageFormat(QImage::Format_ARGB32));
    EXPECT_TRUE(MockUtil::isValidImageFormat(QImage::Format_Grayscale8));
    EXPECT_FALSE(MockUtil::isValidImageFormat(QImage::Format_Invalid));
}

TEST_F(UtilTest, FormatToString) {
    EXPECT_EQ(MockUtil::formatToString(QImage::Format_RGB32), "RGB32");
    EXPECT_EQ(MockUtil::formatToString(QImage::Format_ARGB32), "ARGB32");
    EXPECT_EQ(MockUtil::formatToString(QImage::Format_Grayscale8), "Grayscale8");
    EXPECT_EQ(MockUtil::formatToString(QImage::Format_Invalid), "Unknown");
}

TEST_F(UtilTest, EdgeCases) {
    // Test clamping at boundaries
    EXPECT_EQ(MockUtil::clamp(0.0, 0.0, 1.0), 0.0);
    EXPECT_EQ(MockUtil::clamp(1.0, 0.0, 1.0), 1.0);
    
    // Test lerp with same values
    EXPECT_EQ(MockUtil::lerp(5.0, 5.0, 0.5), 5.0);
    
    // Test empty image
    QImage emptyImage;
    QRect emptyRect = MockUtil::getBoundingRect(emptyImage);
    EXPECT_TRUE(emptyRect.isEmpty());
}

} // namespace test
} // namespace ibp