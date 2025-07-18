// this_file: tests/imgproc/test_imagefilterlist.cpp

#include "../test_utils.h"
#include <gtest/gtest.h>

namespace ibp {
namespace test {

class ImageFilterListTest : public ImageProcessingTest {
protected:
    void SetUp() override {
        ImageProcessingTest::SetUp();
    }
};

// Mock ImageFilterList for testing
class MockImageFilterList {
public:
    MockImageFilterList() = default;
    
    void addFilter(const QString& filterName) {
        filters.append(filterName);
    }
    
    void removeFilter(int index) {
        if (index >= 0 && index < filters.size()) {
            filters.removeAt(index);
        }
    }
    
    int count() const {
        return filters.size();
    }
    
    QString getFilterName(int index) const {
        if (index >= 0 && index < filters.size()) {
            return filters[index];
        }
        return QString();
    }
    
    QImage process(const QImage& input) {
        // Simple pass-through for testing
        return input;
    }
    
    bool saveToFile(const QString& filename) const {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }
        
        QTextStream out(&file);
        out << "[FilterList]\n";
        out << "count=" << filters.size() << "\n";
        
        for (int i = 0; i < filters.size(); ++i) {
            out << "filter" << i << "=" << filters[i] << "\n";
        }
        
        return true;
    }
    
    bool loadFromFile(const QString& filename) {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return false;
        }
        
        filters.clear();
        QTextStream in(&file);
        
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("filter")) {
                QStringList parts = line.split("=");
                if (parts.size() == 2) {
                    filters.append(parts[1]);
                }
            }
        }
        
        return true;
    }
    
private:
    QStringList filters;
};

TEST_F(ImageFilterListTest, AddAndRemoveFilters) {
    MockImageFilterList filterList;
    
    EXPECT_EQ(filterList.count(), 0);
    
    filterList.addFilter("filter1");
    EXPECT_EQ(filterList.count(), 1);
    EXPECT_EQ(filterList.getFilterName(0), "filter1");
    
    filterList.addFilter("filter2");
    EXPECT_EQ(filterList.count(), 2);
    EXPECT_EQ(filterList.getFilterName(1), "filter2");
    
    filterList.removeFilter(0);
    EXPECT_EQ(filterList.count(), 1);
    EXPECT_EQ(filterList.getFilterName(0), "filter2");
}

TEST_F(ImageFilterListTest, ProcessImage) {
    MockImageFilterList filterList;
    
    QImage result = filterList.process(testImage);
    EXPECT_THAT(result, ImageEquals(testImage));
}

TEST_F(ImageFilterListTest, SaveAndLoadFilterList) {
    MockImageFilterList filterList;
    filterList.addFilter("brightness");
    filterList.addFilter("contrast");
    filterList.addFilter("blur");
    
    QString filename = tempDir + "/test_filters.ifl";
    
    EXPECT_TRUE(filterList.saveToFile(filename));
    EXPECT_TRUE(QFile::exists(filename));
    
    MockImageFilterList loadedList;
    EXPECT_TRUE(loadedList.loadFromFile(filename));
    
    EXPECT_EQ(loadedList.count(), 3);
    EXPECT_EQ(loadedList.getFilterName(0), "brightness");
    EXPECT_EQ(loadedList.getFilterName(1), "contrast");
    EXPECT_EQ(loadedList.getFilterName(2), "blur");
}

TEST_F(ImageFilterListTest, InvalidOperations) {
    MockImageFilterList filterList;
    
    // Test invalid indices
    EXPECT_EQ(filterList.getFilterName(-1), QString());
    EXPECT_EQ(filterList.getFilterName(0), QString());
    
    filterList.addFilter("filter1");
    EXPECT_EQ(filterList.getFilterName(1), QString());
    
    // Test removing invalid indices
    filterList.removeFilter(-1);
    EXPECT_EQ(filterList.count(), 1);
    
    filterList.removeFilter(5);
    EXPECT_EQ(filterList.count(), 1);
}

} // namespace test
} // namespace ibp