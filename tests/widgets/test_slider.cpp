// this_file: tests/widgets/test_slider.cpp

#include "../test_utils.h"
#include <gtest/gtest.h>

namespace ibp {
namespace test {

class SliderTest : public IBPTestBase {
protected:
    void SetUp() override {
        IBPTestBase::SetUp();
    }
};

// Mock custom slider widget for testing
class MockCustomSlider : public QWidget {
    Q_OBJECT
    
public:
    MockCustomSlider(QWidget* parent = nullptr) 
        : QWidget(parent), minValue(0), maxValue(100), currentValue(50) {}
    
    void setRange(int min, int max) {
        minValue = min;
        maxValue = max;
        
        // Clamp current value to new range
        int clampedValue = std::max(min, std::min(max, currentValue));
        if (clampedValue != currentValue) {
            setValue(clampedValue);
        }
    }
    
    void setValue(int value) {
        int clampedValue = std::max(minValue, std::min(maxValue, value));
        if (clampedValue != currentValue) {
            currentValue = clampedValue;
            emit valueChanged(currentValue);
        }
    }
    
    int getValue() const {
        return currentValue;
    }
    
    int getMinimum() const {
        return minValue;
    }
    
    int getMaximum() const {
        return maxValue;
    }
    
    void setStep(int step) {
        stepSize = step;
    }
    
    int getStep() const {
        return stepSize;
    }
    
    void stepUp() {
        setValue(currentValue + stepSize);
    }
    
    void stepDown() {
        setValue(currentValue - stepSize);
    }
    
signals:
    void valueChanged(int value);
    
private:
    int minValue;
    int maxValue;
    int currentValue;
    int stepSize = 1;
};

TEST_F(SliderTest, BasicValueOperations) {
    MockCustomSlider slider;
    
    EXPECT_EQ(slider.getValue(), 50);
    EXPECT_EQ(slider.getMinimum(), 0);
    EXPECT_EQ(slider.getMaximum(), 100);
    
    slider.setValue(75);
    EXPECT_EQ(slider.getValue(), 75);
}

TEST_F(SliderTest, RangeOperations) {
    MockCustomSlider slider;
    
    slider.setRange(10, 90);
    EXPECT_EQ(slider.getMinimum(), 10);
    EXPECT_EQ(slider.getMaximum(), 90);
    
    // Value should be clamped to new range
    EXPECT_EQ(slider.getValue(), 50); // Should still be within range
    
    slider.setValue(5); // Below minimum
    EXPECT_EQ(slider.getValue(), 10);
    
    slider.setValue(95); // Above maximum
    EXPECT_EQ(slider.getValue(), 90);
}

TEST_F(SliderTest, SteppingOperations) {
    MockCustomSlider slider;
    
    slider.setRange(0, 100);
    slider.setValue(50);
    slider.setStep(5);
    
    EXPECT_EQ(slider.getStep(), 5);
    
    slider.stepUp();
    EXPECT_EQ(slider.getValue(), 55);
    
    slider.stepDown();
    EXPECT_EQ(slider.getValue(), 50);
    
    slider.stepDown();
    EXPECT_EQ(slider.getValue(), 45);
}

TEST_F(SliderTest, BoundaryConditions) {
    MockCustomSlider slider;
    
    slider.setRange(0, 10);
    
    // Test at minimum
    slider.setValue(0);
    slider.stepDown();
    EXPECT_EQ(slider.getValue(), 0); // Should not go below minimum
    
    // Test at maximum
    slider.setValue(10);
    slider.stepUp();
    EXPECT_EQ(slider.getValue(), 10); // Should not go above maximum
}

TEST_F(SliderTest, ValueChangeSignal) {
    MockCustomSlider slider;
    
    bool signalEmitted = false;
    int signalValue = 0;
    
    QObject::connect(&slider, &MockCustomSlider::valueChanged, 
                    [&](int value) {
                        signalEmitted = true;
                        signalValue = value;
                    });
    
    slider.setValue(25);
    
    EXPECT_TRUE(signalEmitted);
    EXPECT_EQ(signalValue, 25);
}

TEST_F(SliderTest, NoSignalOnSameValue) {
    MockCustomSlider slider;
    
    slider.setValue(30);
    
    int signalCount = 0;
    QObject::connect(&slider, &MockCustomSlider::valueChanged, 
                    [&](int) { signalCount++; });
    
    slider.setValue(30); // Same value
    
    EXPECT_EQ(signalCount, 0);
}

TEST_F(SliderTest, RangeChangeWithValueClamping) {
    MockCustomSlider slider;
    
    slider.setValue(80);
    
    bool signalEmitted = false;
    int signalValue = 0;
    
    QObject::connect(&slider, &MockCustomSlider::valueChanged, 
                    [&](int value) {
                        signalEmitted = true;
                        signalValue = value;
                    });
    
    // Change range to force value clamping
    slider.setRange(0, 50);
    
    EXPECT_TRUE(signalEmitted);
    EXPECT_EQ(signalValue, 50); // Value should be clamped to new maximum
    EXPECT_EQ(slider.getValue(), 50);
}

TEST_F(SliderTest, LargeStepSize) {
    MockCustomSlider slider;
    
    slider.setRange(0, 100);
    slider.setValue(50);
    slider.setStep(30);
    
    slider.stepUp();
    EXPECT_EQ(slider.getValue(), 80);
    
    slider.stepUp();
    EXPECT_EQ(slider.getValue(), 100); // Clamped to maximum
    
    slider.stepDown();
    EXPECT_EQ(slider.getValue(), 70);
}

} // namespace test
} // namespace ibp

#include "test_slider.moc"