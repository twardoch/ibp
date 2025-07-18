// this_file: tests/misc/test_configuration.cpp

#include "../test_utils.h"
#include <gtest/gtest.h>

namespace ibp {
namespace test {

class ConfigurationTest : public IBPTestBase {
protected:
    void SetUp() override {
        IBPTestBase::SetUp();
        configFile = TestUtils::createTempFile("", ".ini");
    }
    
    QString configFile;
};

// Mock configuration manager for testing
class MockConfigurationManager {
public:
    MockConfigurationManager(const QString& filename) 
        : filename(filename) {}
    
    void setValue(const QString& key, const QVariant& value) {
        settings[key] = value;
    }
    
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const {
        return settings.value(key, defaultValue);
    }
    
    bool contains(const QString& key) const {
        return settings.contains(key);
    }
    
    void remove(const QString& key) {
        settings.remove(key);
    }
    
    void clear() {
        settings.clear();
    }
    
    QStringList keys() const {
        return settings.keys();
    }
    
    bool save() {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }
        
        QTextStream out(&file);
        for (auto it = settings.constBegin(); it != settings.constEnd(); ++it) {
            out << it.key() << "=" << it.value().toString() << "\n";
        }
        
        return true;
    }
    
    bool load() {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return false;
        }
        
        settings.clear();
        QTextStream in(&file);
        
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split("=", Qt::SkipEmptyParts);
            if (parts.size() == 2) {
                settings[parts[0]] = parts[1];
            }
        }
        
        return true;
    }
    
private:
    QString filename;
    QMap<QString, QVariant> settings;
};

TEST_F(ConfigurationTest, BasicSetAndGet) {
    MockConfigurationManager config(configFile);
    
    config.setValue("test_key", "test_value");
    EXPECT_EQ(config.getValue("test_key").toString(), "test_value");
    
    config.setValue("test_int", 42);
    EXPECT_EQ(config.getValue("test_int").toInt(), 42);
    
    config.setValue("test_bool", true);
    EXPECT_EQ(config.getValue("test_bool").toBool(), true);
}

TEST_F(ConfigurationTest, DefaultValues) {
    MockConfigurationManager config(configFile);
    
    EXPECT_EQ(config.getValue("nonexistent", "default").toString(), "default");
    EXPECT_EQ(config.getValue("nonexistent", 123).toInt(), 123);
    EXPECT_EQ(config.getValue("nonexistent", false).toBool(), false);
}

TEST_F(ConfigurationTest, ContainsAndRemove) {
    MockConfigurationManager config(configFile);
    
    EXPECT_FALSE(config.contains("test_key"));
    
    config.setValue("test_key", "value");
    EXPECT_TRUE(config.contains("test_key"));
    
    config.remove("test_key");
    EXPECT_FALSE(config.contains("test_key"));
}

TEST_F(ConfigurationTest, SaveAndLoad) {
    MockConfigurationManager config(configFile);
    
    config.setValue("key1", "value1");
    config.setValue("key2", 42);
    config.setValue("key3", true);
    
    EXPECT_TRUE(config.save());
    
    MockConfigurationManager config2(configFile);
    EXPECT_TRUE(config2.load());
    
    EXPECT_EQ(config2.getValue("key1").toString(), "value1");
    EXPECT_EQ(config2.getValue("key2").toInt(), 42);
    EXPECT_EQ(config2.getValue("key3").toBool(), true);
}

TEST_F(ConfigurationTest, ClearConfiguration) {
    MockConfigurationManager config(configFile);
    
    config.setValue("key1", "value1");
    config.setValue("key2", "value2");
    
    EXPECT_EQ(config.keys().size(), 2);
    
    config.clear();
    EXPECT_EQ(config.keys().size(), 0);
    EXPECT_FALSE(config.contains("key1"));
    EXPECT_FALSE(config.contains("key2"));
}

TEST_F(ConfigurationTest, GetAllKeys) {
    MockConfigurationManager config(configFile);
    
    config.setValue("a", "1");
    config.setValue("b", "2");
    config.setValue("c", "3");
    
    QStringList keys = config.keys();
    EXPECT_EQ(keys.size(), 3);
    EXPECT_TRUE(keys.contains("a"));
    EXPECT_TRUE(keys.contains("b"));
    EXPECT_TRUE(keys.contains("c"));
}

} // namespace test
} // namespace ibp