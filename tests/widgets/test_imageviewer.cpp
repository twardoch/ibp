// this_file: tests/widgets/test_imageviewer.cpp

#include "../test_utils.h"
#include <gtest/gtest.h>

namespace ibp {
namespace test {

class ImageViewerTest : public ImageProcessingTest {
protected:
    void SetUp() override {
        ImageProcessingTest::SetUp();
    }
};

// Mock ImageViewer widget for testing
class MockImageViewer : public QWidget {
    Q_OBJECT
    
public:
    MockImageViewer(QWidget* parent = nullptr) 
        : QWidget(parent), zoomFactor(1.0), fitToWindow(false) {}
    
    void setImage(const QImage& image) {
        currentImage = image;
        emit imageChanged();
    }
    
    QImage getImage() const {
        return currentImage;
    }
    
    void setZoomFactor(double factor) {
        if (factor > 0 && factor != zoomFactor) {
            zoomFactor = factor;
            fitToWindow = false;
            emit zoomChanged(zoomFactor);
        }
    }
    
    double getZoomFactor() const {
        return zoomFactor;
    }
    
    void zoomIn() {
        setZoomFactor(zoomFactor * 1.25);
    }
    
    void zoomOut() {
        setZoomFactor(zoomFactor / 1.25);
    }
    
    void zoomToFit() {
        fitToWindow = true;
        zoomFactor = 1.0; // Placeholder
        emit zoomChanged(zoomFactor);
    }
    
    void actualSize() {
        setZoomFactor(1.0);
    }
    
    bool isFitToWindow() const {
        return fitToWindow;
    }
    
    QSize getImageSize() const {
        return currentImage.size();
    }
    
    QSize getViewportSize() const {
        return size();
    }
    
    QPoint getImagePosition() const {
        return imagePosition;
    }
    
    void setImagePosition(const QPoint& pos) {
        imagePosition = pos;
    }
    
    void centerImage() {
        if (!currentImage.isNull()) {
            QSize imageSize = currentImage.size();
            QSize viewportSize = size();
            
            int x = (viewportSize.width() - imageSize.width()) / 2;
            int y = (viewportSize.height() - imageSize.height()) / 2;
            
            setImagePosition(QPoint(x, y));
        }
    }
    
signals:
    void imageChanged();
    void zoomChanged(double factor);
    void positionChanged(const QPoint& position);
    
private:
    QImage currentImage;
    double zoomFactor;
    bool fitToWindow;
    QPoint imagePosition;
};

TEST_F(ImageViewerTest, BasicImageOperations) {
    MockImageViewer viewer;
    
    EXPECT_TRUE(viewer.getImage().isNull());
    
    viewer.setImage(testImage);
    EXPECT_FALSE(viewer.getImage().isNull());
    EXPECT_EQ(viewer.getImageSize(), testImage.size());
}

TEST_F(ImageViewerTest, ZoomOperations) {
    MockImageViewer viewer;
    
    EXPECT_DOUBLE_EQ(viewer.getZoomFactor(), 1.0);
    
    viewer.setZoomFactor(2.0);
    EXPECT_DOUBLE_EQ(viewer.getZoomFactor(), 2.0);
    
    viewer.zoomIn();
    EXPECT_DOUBLE_EQ(viewer.getZoomFactor(), 2.5); // 2.0 * 1.25
    
    viewer.zoomOut();
    EXPECT_DOUBLE_EQ(viewer.getZoomFactor(), 2.0); // 2.5 / 1.25
    
    viewer.actualSize();
    EXPECT_DOUBLE_EQ(viewer.getZoomFactor(), 1.0);
}

TEST_F(ImageViewerTest, ZoomToFit) {
    MockImageViewer viewer;
    
    EXPECT_FALSE(viewer.isFitToWindow());
    
    viewer.zoomToFit();
    EXPECT_TRUE(viewer.isFitToWindow());
    
    viewer.setZoomFactor(2.0);
    EXPECT_FALSE(viewer.isFitToWindow());
}

TEST_F(ImageViewerTest, ImagePositioning) {
    MockImageViewer viewer;
    
    QPoint testPos(50, 100);
    viewer.setImagePosition(testPos);
    EXPECT_EQ(viewer.getImagePosition(), testPos);
    
    // Test centering
    viewer.resize(400, 300);
    viewer.setImage(testImage); // 100x100 image
    viewer.centerImage();
    
    QPoint expectedCenter(150, 100); // (400-100)/2, (300-100)/2
    EXPECT_EQ(viewer.getImagePosition(), expectedCenter);
}

TEST_F(ImageViewerTest, InvalidZoomFactors) {
    MockImageViewer viewer;
    
    double originalZoom = viewer.getZoomFactor();
    
    // Test negative zoom
    viewer.setZoomFactor(-1.0);
    EXPECT_DOUBLE_EQ(viewer.getZoomFactor(), originalZoom);
    
    // Test zero zoom
    viewer.setZoomFactor(0.0);
    EXPECT_DOUBLE_EQ(viewer.getZoomFactor(), originalZoom);
}

TEST_F(ImageViewerTest, ImageChangeSignal) {
    MockImageViewer viewer;
    
    bool signalEmitted = false;
    QObject::connect(&viewer, &MockImageViewer::imageChanged, 
                    [&]() { signalEmitted = true; });
    
    viewer.setImage(testImage);
    EXPECT_TRUE(signalEmitted);
}

TEST_F(ImageViewerTest, ZoomChangeSignal) {
    MockImageViewer viewer;
    
    bool signalEmitted = false;
    double signalZoom = 0.0;
    
    QObject::connect(&viewer, &MockImageViewer::zoomChanged, 
                    [&](double zoom) {
                        signalEmitted = true;
                        signalZoom = zoom;
                    });
    
    viewer.setZoomFactor(1.5);
    
    EXPECT_TRUE(signalEmitted);
    EXPECT_DOUBLE_EQ(signalZoom, 1.5);
}

TEST_F(ImageViewerTest, NoSignalOnSameZoom) {
    MockImageViewer viewer;
    
    viewer.setZoomFactor(2.0);
    
    int signalCount = 0;
    QObject::connect(&viewer, &MockImageViewer::zoomChanged, 
                    [&](double) { signalCount++; });
    
    viewer.setZoomFactor(2.0); // Same zoom
    
    EXPECT_EQ(signalCount, 0);
}

TEST_F(ImageViewerTest, ViewportOperations) {
    MockImageViewer viewer;
    
    viewer.resize(800, 600);
    EXPECT_EQ(viewer.getViewportSize(), QSize(800, 600));
    
    viewer.setImage(testImage);
    EXPECT_EQ(viewer.getImageSize(), testImage.size());
    
    // Test that viewport size is independent of image size
    EXPECT_NE(viewer.getViewportSize(), viewer.getImageSize());
}

} // namespace test
} // namespace ibp

#include "test_imageviewer.moc"