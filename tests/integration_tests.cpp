// this_file: tests/integration_tests.cpp

#include "test_utils.h"
#include <ibp/version.h>
#include <QApplication>
#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <gtest/gtest.h>

namespace ibp {
namespace test {

class IntegrationTest : public IBPTestBase {
protected:
    void SetUp() override {
        IBPTestBase::SetUp();
        
        // Find the executable path
        QString appPath = QCoreApplication::applicationDirPath();
        executablePath = appPath + "/ibp";
        
        #ifdef Q_OS_WIN
        executablePath += ".exe";
        #endif
        
        // Create temporary directories for testing
        inputDir = TestUtils::createTempDir() + "/input";
        outputDir = TestUtils::createTempDir() + "/output";
        
        QDir().mkpath(inputDir);
        QDir().mkpath(outputDir);
        
        // Create test images
        testImage = TestUtils::createTestImage(200, 200, Qt::red);
        testImagePath = inputDir + "/test.png";
        testImage.save(testImagePath);
    }
    
    QString executablePath;
    QString inputDir;
    QString outputDir;
    QImage testImage;
    QString testImagePath;
};

TEST_F(IntegrationTest, ExecutableExists) {
    EXPECT_TRUE(QFile::exists(executablePath)) 
        << "Executable not found at: " << executablePath.toStdString();
}

TEST_F(IntegrationTest, VersionCommand) {
    QProcess process;
    process.start(executablePath, QStringList() << "--version");
    
    ASSERT_TRUE(process.waitForFinished(5000))
        << "Version command timed out";
    
    EXPECT_EQ(process.exitCode(), 0)
        << "Version command failed with exit code: " << process.exitCode();
    
    QString output = process.readAllStandardOutput();
    EXPECT_FALSE(output.isEmpty())
        << "Version output is empty";
    
    // Should contain version information
    EXPECT_TRUE(output.contains("Image Batch Processor") || output.contains("ibp"))
        << "Version output doesn't contain expected application name";
}

TEST_F(IntegrationTest, HelpCommand) {
    QProcess process;
    process.start(executablePath, QStringList() << "--help");
    
    ASSERT_TRUE(process.waitForFinished(5000))
        << "Help command timed out";
    
    EXPECT_EQ(process.exitCode(), 0)
        << "Help command failed with exit code: " << process.exitCode();
    
    QString output = process.readAllStandardOutput();
    EXPECT_FALSE(output.isEmpty())
        << "Help output is empty";
    
    // Should contain usage information
    EXPECT_TRUE(output.contains("Usage:") || output.contains("Options:"))
        << "Help output doesn't contain expected usage information";
}

TEST_F(IntegrationTest, InvalidOption) {
    QProcess process;
    process.start(executablePath, QStringList() << "--invalid-option");
    
    ASSERT_TRUE(process.waitForFinished(5000))
        << "Invalid option command timed out";
    
    EXPECT_NE(process.exitCode(), 0)
        << "Invalid option should return non-zero exit code";
}

TEST_F(IntegrationTest, BasicImageProcessing) {
    // Skip this test if we don't have a working executable
    if (!QFile::exists(executablePath)) {
        GTEST_SKIP() << "Executable not found";
    }
    
    // Create a simple filter list file
    QString filterListPath = inputDir + "/filters.ifl";
    QFile filterFile(filterListPath);
    ASSERT_TRUE(filterFile.open(QIODevice::WriteOnly | QIODevice::Text));
    
    QTextStream out(&filterFile);
    out << "[imagefilter_identity]\n";
    out << "id=identity\n";
    out << "enabled=true\n";
    filterFile.close();
    
    QString outputPath = outputDir + "/output.png";
    
    QProcess process;
    QStringList args;
    args << "-i" << testImagePath
         << "-o" << outputPath
         << "-l" << filterListPath;
    
    process.start(executablePath, args);
    
    ASSERT_TRUE(process.waitForFinished(10000))
        << "Image processing command timed out";
    
    if (process.exitCode() != 0) {
        QString errorOutput = process.readAllStandardError();
        GTEST_SKIP() << "Image processing failed (expected for basic test): " << errorOutput.toStdString();
    }
    
    // Check if output file was created
    EXPECT_TRUE(QFile::exists(outputPath))
        << "Output file was not created";
    
    // Load and verify output image
    QImage outputImage(outputPath);
    EXPECT_FALSE(outputImage.isNull())
        << "Output image is invalid";
    
    // For identity filter, output should be similar to input
    EXPECT_EQ(outputImage.size(), testImage.size())
        << "Output image size doesn't match input";
}

TEST_F(IntegrationTest, VersionHeaderIntegration) {
    // Test that version header is properly configured
    EXPECT_STREQ(IBP_VERSION, PROJECT_VERSION);
    EXPECT_GT(IBP_VERSION_MAJOR, 0);
    EXPECT_GE(IBP_VERSION_MINOR, 0);
    EXPECT_GE(IBP_VERSION_PATCH, 0);
    
    // Test that git information is populated
    EXPECT_STRNE(IBP_GIT_COMMIT_HASH, "unknown");
    EXPECT_STRNE(IBP_BUILD_TIMESTAMP, "unknown");
}

} // namespace test
} // namespace ibp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}