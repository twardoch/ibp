// this_file: tests/misc/test_probability.cpp

#include "../test_utils.h"
#include <gtest/gtest.h>

namespace ibp {
namespace test {

class ProbabilityTest : public IBPTestBase {
protected:
    void SetUp() override {
        IBPTestBase::SetUp();
    }
};

// Mock probability mass function for testing
class MockProbabilityMassFunction {
public:
    MockProbabilityMassFunction() {
        probabilities.resize(256, 0.0);
    }
    
    void setProbability(int value, double probability) {
        if (value >= 0 && value < probabilities.size()) {
            probabilities[value] = probability;
        }
    }
    
    double getProbability(int value) const {
        if (value >= 0 && value < probabilities.size()) {
            return probabilities[value];
        }
        return 0.0;
    }
    
    void normalize() {
        double sum = 0.0;
        for (double p : probabilities) {
            sum += p;
        }
        
        if (sum > 0.0) {
            for (double& p : probabilities) {
                p /= sum;
            }
        }
    }
    
    double getSum() const {
        double sum = 0.0;
        for (double p : probabilities) {
            sum += p;
        }
        return sum;
    }
    
    double getMean() const {
        double mean = 0.0;
        for (int i = 0; i < probabilities.size(); ++i) {
            mean += i * probabilities[i];
        }
        return mean;
    }
    
    double getVariance() const {
        double mean = getMean();
        double variance = 0.0;
        
        for (int i = 0; i < probabilities.size(); ++i) {
            double diff = i - mean;
            variance += diff * diff * probabilities[i];
        }
        
        return variance;
    }
    
    double getStandardDeviation() const {
        return std::sqrt(getVariance());
    }
    
    void calculateFromHistogram(const QVector<int>& histogram) {
        int totalCount = 0;
        for (int count : histogram) {
            totalCount += count;
        }
        
        for (int i = 0; i < std::min(histogram.size(), probabilities.size()); ++i) {
            probabilities[i] = totalCount > 0 ? (double)histogram[i] / totalCount : 0.0;
        }
    }
    
private:
    QVector<double> probabilities;
};

TEST_F(ProbabilityTest, BasicProbabilityOperations) {
    MockProbabilityMassFunction pmf;
    
    pmf.setProbability(0, 0.5);
    pmf.setProbability(1, 0.3);
    pmf.setProbability(2, 0.2);
    
    EXPECT_DOUBLE_EQ(pmf.getProbability(0), 0.5);
    EXPECT_DOUBLE_EQ(pmf.getProbability(1), 0.3);
    EXPECT_DOUBLE_EQ(pmf.getProbability(2), 0.2);
    EXPECT_DOUBLE_EQ(pmf.getProbability(3), 0.0);
}

TEST_F(ProbabilityTest, Normalization) {
    MockProbabilityMassFunction pmf;
    
    pmf.setProbability(0, 2.0);
    pmf.setProbability(1, 3.0);
    pmf.setProbability(2, 5.0);
    
    EXPECT_DOUBLE_EQ(pmf.getSum(), 10.0);
    
    pmf.normalize();
    
    EXPECT_DOUBLE_EQ(pmf.getProbability(0), 0.2);
    EXPECT_DOUBLE_EQ(pmf.getProbability(1), 0.3);
    EXPECT_DOUBLE_EQ(pmf.getProbability(2), 0.5);
    EXPECT_NEAR(pmf.getSum(), 1.0, 1e-10);
}

TEST_F(ProbabilityTest, StatisticalMoments) {
    MockProbabilityMassFunction pmf;
    
    // Simple uniform distribution over 0, 1, 2
    pmf.setProbability(0, 1.0/3.0);
    pmf.setProbability(1, 1.0/3.0);
    pmf.setProbability(2, 1.0/3.0);
    
    // Mean should be 1.0
    EXPECT_NEAR(pmf.getMean(), 1.0, 1e-10);
    
    // Variance should be 2/3
    EXPECT_NEAR(pmf.getVariance(), 2.0/3.0, 1e-10);
    
    // Standard deviation should be sqrt(2/3)
    EXPECT_NEAR(pmf.getStandardDeviation(), std::sqrt(2.0/3.0), 1e-10);
}

TEST_F(ProbabilityTest, HistogramToProbability) {
    MockProbabilityMassFunction pmf;
    
    QVector<int> histogram = {10, 20, 30, 40}; // Total = 100
    pmf.calculateFromHistogram(histogram);
    
    EXPECT_DOUBLE_EQ(pmf.getProbability(0), 0.1);
    EXPECT_DOUBLE_EQ(pmf.getProbability(1), 0.2);
    EXPECT_DOUBLE_EQ(pmf.getProbability(2), 0.3);
    EXPECT_DOUBLE_EQ(pmf.getProbability(3), 0.4);
}

TEST_F(ProbabilityTest, EmptyHistogram) {
    MockProbabilityMassFunction pmf;
    
    QVector<int> histogram = {0, 0, 0, 0};
    pmf.calculateFromHistogram(histogram);
    
    EXPECT_DOUBLE_EQ(pmf.getProbability(0), 0.0);
    EXPECT_DOUBLE_EQ(pmf.getProbability(1), 0.0);
    EXPECT_DOUBLE_EQ(pmf.getProbability(2), 0.0);
    EXPECT_DOUBLE_EQ(pmf.getProbability(3), 0.0);
}

TEST_F(ProbabilityTest, SingleValueDistribution) {
    MockProbabilityMassFunction pmf;
    
    pmf.setProbability(5, 1.0);
    
    EXPECT_DOUBLE_EQ(pmf.getMean(), 5.0);
    EXPECT_DOUBLE_EQ(pmf.getVariance(), 0.0);
    EXPECT_DOUBLE_EQ(pmf.getStandardDeviation(), 0.0);
}

TEST_F(ProbabilityTest, OutOfBoundsBehavior) {
    MockProbabilityMassFunction pmf;
    
    pmf.setProbability(-1, 0.5);  // Should be ignored
    pmf.setProbability(300, 0.5);  // Should be ignored
    
    EXPECT_DOUBLE_EQ(pmf.getProbability(-1), 0.0);
    EXPECT_DOUBLE_EQ(pmf.getProbability(300), 0.0);
    EXPECT_DOUBLE_EQ(pmf.getSum(), 0.0);
}

} // namespace test
} // namespace ibp