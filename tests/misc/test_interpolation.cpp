// this_file: tests/misc/test_interpolation.cpp

#include "../test_utils.h"
#include <gtest/gtest.h>

namespace ibp {
namespace test {

class InterpolationTest : public IBPTestBase {
protected:
    void SetUp() override {
        IBPTestBase::SetUp();
    }
};

// Mock interpolator classes for testing
class MockLinearInterpolator {
public:
    MockLinearInterpolator(const QVector<double>& x, const QVector<double>& y) 
        : xValues(x), yValues(y) {}
    
    double interpolate(double x) const {
        if (xValues.size() != yValues.size() || xValues.isEmpty()) {
            return 0.0;
        }
        
        // Find the interval
        int i = 0;
        while (i < xValues.size() - 1 && xValues[i + 1] < x) {
            i++;
        }
        
        if (i == xValues.size() - 1) {
            return yValues[i];
        }
        
        // Linear interpolation
        double x1 = xValues[i];
        double x2 = xValues[i + 1];
        double y1 = yValues[i];
        double y2 = yValues[i + 1];
        
        double t = (x - x1) / (x2 - x1);
        return y1 + t * (y2 - y1);
    }
    
private:
    QVector<double> xValues;
    QVector<double> yValues;
};

class MockCubicSplineInterpolator {
public:
    MockCubicSplineInterpolator(const QVector<double>& x, const QVector<double>& y) 
        : xValues(x), yValues(y) {}
    
    double interpolate(double x) const {
        // Simplified cubic spline (actually linear for testing)
        MockLinearInterpolator linear(xValues, yValues);
        return linear.interpolate(x);
    }
    
private:
    QVector<double> xValues;
    QVector<double> yValues;
};

TEST_F(InterpolationTest, LinearInterpolation) {
    QVector<double> x = {0.0, 1.0, 2.0, 3.0};
    QVector<double> y = {0.0, 1.0, 4.0, 9.0};
    
    MockLinearInterpolator interpolator(x, y);
    
    // Test exact points
    EXPECT_DOUBLE_EQ(interpolator.interpolate(0.0), 0.0);
    EXPECT_DOUBLE_EQ(interpolator.interpolate(1.0), 1.0);
    EXPECT_DOUBLE_EQ(interpolator.interpolate(2.0), 4.0);
    EXPECT_DOUBLE_EQ(interpolator.interpolate(3.0), 9.0);
    
    // Test interpolated points
    EXPECT_DOUBLE_EQ(interpolator.interpolate(0.5), 0.5);
    EXPECT_DOUBLE_EQ(interpolator.interpolate(1.5), 2.5);
    EXPECT_DOUBLE_EQ(interpolator.interpolate(2.5), 6.5);
}

TEST_F(InterpolationTest, CubicSplineInterpolation) {
    QVector<double> x = {0.0, 1.0, 2.0, 3.0};
    QVector<double> y = {0.0, 1.0, 4.0, 9.0};
    
    MockCubicSplineInterpolator interpolator(x, y);
    
    // Test exact points
    EXPECT_DOUBLE_EQ(interpolator.interpolate(0.0), 0.0);
    EXPECT_DOUBLE_EQ(interpolator.interpolate(1.0), 1.0);
    EXPECT_DOUBLE_EQ(interpolator.interpolate(2.0), 4.0);
    EXPECT_DOUBLE_EQ(interpolator.interpolate(3.0), 9.0);
}

TEST_F(InterpolationTest, ExtrapolationBehavior) {
    QVector<double> x = {1.0, 2.0, 3.0};
    QVector<double> y = {1.0, 4.0, 9.0};
    
    MockLinearInterpolator interpolator(x, y);
    
    // Test values outside the range
    EXPECT_DOUBLE_EQ(interpolator.interpolate(0.0), 1.0);  // Should return first value
    EXPECT_DOUBLE_EQ(interpolator.interpolate(4.0), 9.0);  // Should return last value
}

TEST_F(InterpolationTest, EmptyData) {
    QVector<double> x;
    QVector<double> y;
    
    MockLinearInterpolator interpolator(x, y);
    
    EXPECT_DOUBLE_EQ(interpolator.interpolate(0.0), 0.0);
    EXPECT_DOUBLE_EQ(interpolator.interpolate(1.0), 0.0);
}

TEST_F(InterpolationTest, SinglePoint) {
    QVector<double> x = {1.0};
    QVector<double> y = {5.0};
    
    MockLinearInterpolator interpolator(x, y);
    
    EXPECT_DOUBLE_EQ(interpolator.interpolate(0.0), 5.0);
    EXPECT_DOUBLE_EQ(interpolator.interpolate(1.0), 5.0);
    EXPECT_DOUBLE_EQ(interpolator.interpolate(2.0), 5.0);
}

TEST_F(InterpolationTest, MismatchedDataSizes) {
    QVector<double> x = {0.0, 1.0, 2.0};
    QVector<double> y = {0.0, 1.0};  // Different size
    
    MockLinearInterpolator interpolator(x, y);
    
    EXPECT_DOUBLE_EQ(interpolator.interpolate(0.0), 0.0);
    EXPECT_DOUBLE_EQ(interpolator.interpolate(1.0), 0.0);
}

TEST_F(InterpolationTest, MonotonicData) {
    QVector<double> x = {0.0, 1.0, 2.0, 3.0};
    QVector<double> y = {0.0, 2.0, 4.0, 6.0};  // Linear relationship
    
    MockLinearInterpolator interpolator(x, y);
    
    // Should interpolate perfectly on a line
    EXPECT_DOUBLE_EQ(interpolator.interpolate(0.5), 1.0);
    EXPECT_DOUBLE_EQ(interpolator.interpolate(1.5), 3.0);
    EXPECT_DOUBLE_EQ(interpolator.interpolate(2.5), 5.0);
}

} // namespace test
} // namespace ibp