// this_file: tests/imgproc/test_imagehistogram.cpp

#include "../test_utils.h"
#include <gtest/gtest.h>

namespace ibp {
namespace test {

class ImageHistogramTest : public ImageProcessingTest {
protected:
    void SetUp() override {
        ImageProcessingTest::SetUp();
    }
};

// Mock ImageHistogram implementation for testing
class MockImageHistogram {
public:
    MockImageHistogram() : bins(256, 0) {}
    
    void calculate(const QImage& image) {
        std::fill(bins.begin(), bins.end(), 0);
        
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                QRgb pixel = image.pixel(x, y);
                int gray = qGray(pixel);
                bins[gray]++;
            }
        }
    }
    
    int getBin(int index) const {
        if (index >= 0 && index < bins.size()) {
            return bins[index];
        }
        return 0;
    }
    
    int getTotalPixels() const {
        int total = 0;
        for (int bin : bins) {
            total += bin;
        }
        return total;
    }
    
    int getMaxBin() const {
        return *std::max_element(bins.begin(), bins.end());
    }
    
    int getMinBin() const {
        return *std::min_element(bins.begin(), bins.end());
    }
    
    double getMean() const {
        double sum = 0;
        int totalPixels = getTotalPixels();
        
        if (totalPixels == 0) return 0;
        
        for (int i = 0; i < bins.size(); ++i) {
            sum += i * bins[i];
        }
        
        return sum / totalPixels;
    }
    
private:
    std::vector<int> bins;
};

TEST_F(ImageHistogramTest, BasicHistogramCalculation) {
    MockImageHistogram histogram;
    
    // Create a simple white image
    QImage whiteImage = TestUtils::createTestImage(10, 10, Qt::white);
    histogram.calculate(whiteImage);
    
    // All pixels should be in the white bin (255)
    EXPECT_EQ(histogram.getBin(255), 100); // 10x10 = 100 pixels
    EXPECT_EQ(histogram.getBin(0), 0);     // No black pixels
    EXPECT_EQ(histogram.getTotalPixels(), 100);
}

TEST_F(ImageHistogramTest, BlackImageHistogram) {
    MockImageHistogram histogram;
    
    QImage blackImage = TestUtils::createTestImage(5, 5, Qt::black);
    histogram.calculate(blackImage);
    
    // All pixels should be in the black bin (0)
    EXPECT_EQ(histogram.getBin(0), 25);   // 5x5 = 25 pixels
    EXPECT_EQ(histogram.getBin(255), 0);  // No white pixels
    EXPECT_EQ(histogram.getTotalPixels(), 25);
}

TEST_F(ImageHistogramTest, EmptyImageHistogram) {
    MockImageHistogram histogram;
    
    QImage emptyImage;
    histogram.calculate(emptyImage);
    
    EXPECT_EQ(histogram.getTotalPixels(), 0);
    EXPECT_EQ(histogram.getMaxBin(), 0);
    EXPECT_EQ(histogram.getMinBin(), 0);
}

TEST_F(ImageHistogramTest, HistogramStatistics) {
    MockImageHistogram histogram;
    
    QImage grayImage = TestUtils::createTestImage(10, 10, QColor(128, 128, 128));
    histogram.calculate(grayImage);
    
    // All pixels should be at gray level 128
    EXPECT_EQ(histogram.getBin(128), 100);
    EXPECT_EQ(histogram.getTotalPixels(), 100);
    EXPECT_EQ(histogram.getMaxBin(), 100);
    
    // Mean should be close to 128
    EXPECT_NEAR(histogram.getMean(), 128.0, 0.1);
}

} // namespace test
} // namespace ibp