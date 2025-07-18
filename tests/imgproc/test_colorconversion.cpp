// this_file: tests/imgproc/test_colorconversion.cpp

#include "../test_utils.h"
#include <gtest/gtest.h>

namespace ibp {
namespace test {

class ColorConversionTest : public ImageProcessingTest {
protected:
    void SetUp() override {
        ImageProcessingTest::SetUp();
    }
};

// Mock color conversion functions for testing
class MockColorConversion {
public:
    static QColor rgbToHsv(const QColor& rgb) {
        return rgb.toHsv();
    }
    
    static QColor hsvToRgb(const QColor& hsv) {
        return hsv.toRgb();
    }
    
    static QColor rgbToHsl(const QColor& rgb) {
        return rgb.toHsl();
    }
    
    static QColor hslToRgb(const QColor& hsl) {
        return hsl.toRgb();
    }
    
    static int rgbToGray(const QColor& rgb) {
        return qGray(rgb.rgb());
    }
    
    static QImage convertToGrayscale(const QImage& image) {
        return image.convertToFormat(QImage::Format_Grayscale8);
    }
};

TEST_F(ColorConversionTest, RgbToHsvConversion) {
    QColor red(255, 0, 0);
    QColor redHsv = MockColorConversion::rgbToHsv(red);
    
    EXPECT_EQ(redHsv.hue(), 0);
    EXPECT_EQ(redHsv.saturation(), 255);
    EXPECT_EQ(redHsv.value(), 255);
}

TEST_F(ColorConversionTest, HsvToRgbConversion) {
    QColor hsv = QColor::fromHsv(0, 255, 255); // Pure red in HSV
    QColor rgb = MockColorConversion::hsvToRgb(hsv);
    
    EXPECT_EQ(rgb.red(), 255);
    EXPECT_EQ(rgb.green(), 0);
    EXPECT_EQ(rgb.blue(), 0);
}

TEST_F(ColorConversionTest, RgbToHslConversion) {
    QColor red(255, 0, 0);
    QColor redHsl = MockColorConversion::rgbToHsl(red);
    
    EXPECT_EQ(redHsl.hue(), 0);
    EXPECT_EQ(redHsl.saturation(), 255);
    EXPECT_EQ(redHsl.lightness(), 127); // 50% lightness
}

TEST_F(ColorConversionTest, GrayscaleConversion) {
    QColor red(255, 0, 0);
    QColor green(0, 255, 0);
    QColor blue(0, 0, 255);
    
    int redGray = MockColorConversion::rgbToGray(red);
    int greenGray = MockColorConversion::rgbToGray(green);
    int blueGray = MockColorConversion::rgbToGray(blue);
    
    // Green should be brightest in grayscale
    EXPECT_GT(greenGray, redGray);
    EXPECT_GT(greenGray, blueGray);
    
    // All should be in valid range
    EXPECT_GE(redGray, 0);
    EXPECT_LE(redGray, 255);
    EXPECT_GE(greenGray, 0);
    EXPECT_LE(greenGray, 255);
    EXPECT_GE(blueGray, 0);
    EXPECT_LE(blueGray, 255);
}

TEST_F(ColorConversionTest, ImageGrayscaleConversion) {
    QImage colorImage = TestUtils::createTestImage(10, 10, Qt::red);
    QImage grayImage = MockColorConversion::convertToGrayscale(colorImage);
    
    EXPECT_EQ(grayImage.format(), QImage::Format_Grayscale8);
    EXPECT_EQ(grayImage.size(), colorImage.size());
}

TEST_F(ColorConversionTest, ColorSpaceRoundTrip) {
    QColor original(123, 45, 67);
    
    // RGB -> HSV -> RGB
    QColor hsv = MockColorConversion::rgbToHsv(original);
    QColor backToRgb = MockColorConversion::hsvToRgb(hsv);
    
    // Should be approximately the same (allowing for rounding errors)
    EXPECT_NEAR(original.red(), backToRgb.red(), 1);
    EXPECT_NEAR(original.green(), backToRgb.green(), 1);
    EXPECT_NEAR(original.blue(), backToRgb.blue(), 1);
    
    // RGB -> HSL -> RGB
    QColor hsl = MockColorConversion::rgbToHsl(original);
    QColor backToRgb2 = MockColorConversion::hslToRgb(hsl);
    
    EXPECT_NEAR(original.red(), backToRgb2.red(), 1);
    EXPECT_NEAR(original.green(), backToRgb2.green(), 1);
    EXPECT_NEAR(original.blue(), backToRgb2.blue(), 1);
}

} // namespace test
} // namespace ibp