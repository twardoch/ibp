// this_file: tests/widgets/test_colorpicker.cpp

#include "../test_utils.h"
#include <gtest/gtest.h>

namespace ibp {
namespace test {

class ColorPickerTest : public IBPTestBase {
protected:
    void SetUp() override {
        IBPTestBase::SetUp();
    }
};

// Mock ColorPicker widget for testing
class MockColorPicker : public QWidget {
    Q_OBJECT
    
public:
    MockColorPicker(QWidget* parent = nullptr) : QWidget(parent), currentColor(Qt::white) {}
    
    QColor getColor() const {
        return currentColor;
    }
    
    void setColor(const QColor& color) {
        if (currentColor != color) {
            currentColor = color;
            emit colorChanged(color);
        }
    }
    
    void setHue(int hue) {
        QColor hsv = currentColor.toHsv();
        hsv.setHsv(hue, hsv.saturation(), hsv.value());
        setColor(hsv);
    }
    
    void setSaturation(int saturation) {
        QColor hsv = currentColor.toHsv();
        hsv.setHsv(hsv.hue(), saturation, hsv.value());
        setColor(hsv);
    }
    
    void setValue(int value) {
        QColor hsv = currentColor.toHsv();
        hsv.setHsv(hsv.hue(), hsv.saturation(), value);
        setColor(hsv);
    }
    
signals:
    void colorChanged(const QColor& color);
    
private:
    QColor currentColor;
};

TEST_F(ColorPickerTest, BasicColorOperations) {
    MockColorPicker picker;
    
    EXPECT_EQ(picker.getColor(), Qt::white);
    
    picker.setColor(Qt::red);
    EXPECT_EQ(picker.getColor(), Qt::red);
    
    picker.setColor(Qt::blue);
    EXPECT_EQ(picker.getColor(), Qt::blue);
}

TEST_F(ColorPickerTest, HSVColorOperations) {
    MockColorPicker picker;
    
    picker.setColor(Qt::red);
    QColor initialColor = picker.getColor();
    
    // Test hue change
    picker.setHue(120); // Green hue
    QColor greenHue = picker.getColor();
    EXPECT_EQ(greenHue.toHsv().hue(), 120);
    
    // Test saturation change
    picker.setSaturation(128);
    QColor changedSaturation = picker.getColor();
    EXPECT_EQ(changedSaturation.toHsv().saturation(), 128);
    
    // Test value change
    picker.setValue(128);
    QColor changedValue = picker.getColor();
    EXPECT_EQ(changedValue.toHsv().value(), 128);
}

TEST_F(ColorPickerTest, ColorChangeSignal) {
    MockColorPicker picker;
    
    bool signalEmitted = false;
    QColor signalColor;
    
    QObject::connect(&picker, &MockColorPicker::colorChanged, 
                    [&](const QColor& color) {
                        signalEmitted = true;
                        signalColor = color;
                    });
    
    picker.setColor(Qt::red);
    
    EXPECT_TRUE(signalEmitted);
    EXPECT_EQ(signalColor, Qt::red);
}

TEST_F(ColorPickerTest, NoSignalOnSameColor) {
    MockColorPicker picker;
    
    picker.setColor(Qt::red);
    
    int signalCount = 0;
    QObject::connect(&picker, &MockColorPicker::colorChanged, 
                    [&](const QColor&) { signalCount++; });
    
    picker.setColor(Qt::red); // Same color
    
    EXPECT_EQ(signalCount, 0);
}

TEST_F(ColorPickerTest, ColorComponentRanges) {
    MockColorPicker picker;
    
    picker.setColor(Qt::red);
    
    // Test valid ranges
    picker.setHue(0);
    EXPECT_EQ(picker.getColor().toHsv().hue(), 0);
    
    picker.setHue(359);
    EXPECT_EQ(picker.getColor().toHsv().hue(), 359);
    
    picker.setSaturation(0);
    EXPECT_EQ(picker.getColor().toHsv().saturation(), 0);
    
    picker.setSaturation(255);
    EXPECT_EQ(picker.getColor().toHsv().saturation(), 255);
    
    picker.setValue(0);
    EXPECT_EQ(picker.getColor().toHsv().value(), 0);
    
    picker.setValue(255);
    EXPECT_EQ(picker.getColor().toHsv().value(), 255);
}

} // namespace test
} // namespace ibp

#include "test_colorpicker.moc"