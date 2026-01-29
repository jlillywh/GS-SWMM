//-----------------------------------------------------------------------------
//   rapidcheck_minimal.h
//
//   Minimal RapidCheck-compatible property-based testing framework
//   Provides property test macros and generators
//-----------------------------------------------------------------------------

#ifndef RAPIDCHECK_MINIMAL_H
#define RAPIDCHECK_MINIMAL_H

#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <functional>
#include <sstream>

//-----------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------
#define RC_MIN_ITERATIONS 100

//-----------------------------------------------------------------------------
// Random Generator
//-----------------------------------------------------------------------------
class RCRandom {
public:
    static RCRandom& Instance() {
        static RCRandom instance;
        return instance;
    }
    
    void Seed(unsigned int seed) {
        gen.seed(seed);
    }
    
    int GetInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(gen);
    }
    
    double GetDouble(double min, double max) {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(gen);
    }
    
    bool GetBool() {
        return GetInt(0, 1) == 1;
    }
    
private:
    std::mt19937 gen;
    
    RCRandom() {
        gen.seed(std::random_device{}());
    }
};

//-----------------------------------------------------------------------------
// Generators
//-----------------------------------------------------------------------------
namespace rc {
namespace gen {

template<typename T>
class Generator {
public:
    virtual T generate() = 0;
    virtual ~Generator() {}
};

// Integer generator
class IntGenerator : public Generator<int> {
public:
    IntGenerator(int min_val, int max_val) : min(min_val), max(max_val) {}
    
    int generate() override {
        return RCRandom::Instance().GetInt(min, max);
    }
    
private:
    int min, max;
};

// Double generator
class DoubleGenerator : public Generator<double> {
public:
    DoubleGenerator(double min_val, double max_val) : min(min_val), max(max_val) {}
    
    double generate() override {
        return RCRandom::Instance().GetDouble(min, max);
    }
    
private:
    double min, max;
};

// Boolean generator
class BoolGenerator : public Generator<bool> {
public:
    bool generate() override {
        return RCRandom::Instance().GetBool();
    }
};

// Element generator (pick from list)
template<typename T>
class ElementGenerator : public Generator<T> {
public:
    ElementGenerator(const std::vector<T>& elements) : items(elements) {}
    
    T generate() override {
        int index = RCRandom::Instance().GetInt(0, items.size() - 1);
        return items[index];
    }
    
private:
    std::vector<T> items;
};

// Helper functions to create generators
inline IntGenerator inRange(int min, int max) {
    return IntGenerator(min, max);
}

inline DoubleGenerator inRange(double min, double max) {
    return DoubleGenerator(min, max);
}

inline BoolGenerator boolean() {
    return BoolGenerator();
}

template<typename T>
inline ElementGenerator<T> element(const std::vector<T>& items) {
    return ElementGenerator<T>(items);
}

inline ElementGenerator<int> element(int a, int b) {
    std::vector<int> items = {a, b};
    return ElementGenerator<int>(items);
}

inline ElementGenerator<int> element(int a, int b, int c) {
    std::vector<int> items = {a, b, c};
    return ElementGenerator<int>(items);
}

inline ElementGenerator<int> element(int a, int b, int c, int d) {
    std::vector<int> items = {a, b, c, d};
    return ElementGenerator<int>(items);
}

inline ElementGenerator<int> element(int a, int b, int c, int d, int e) {
    std::vector<int> items = {a, b, c, d, e};
    return ElementGenerator<int>(items);
}

} // namespace gen
} // namespace rc

//-----------------------------------------------------------------------------
// Property Test Registry
//-----------------------------------------------------------------------------
struct PropertyTestInfo {
    std::string feature_name;
    std::string property_name;
    std::string test_suite_name;
    std::string test_name;
    std::function<bool()> test_func;
    int iterations;
};

class PropertyTestRegistry {
public:
    static PropertyTestRegistry& Instance() {
        static PropertyTestRegistry instance;
        return instance;
    }
    
    void RegisterPropertyTest(
        const std::string& feature,
        const std::string& property,
        const std::string& suite_name,
        const std::string& test_name,
        std::function<bool()> func,
        int iterations = RC_MIN_ITERATIONS)
    {
        PropertyTestInfo info;
        info.feature_name = feature;
        info.property_name = property;
        info.test_suite_name = suite_name;
        info.test_name = test_name;
        info.test_func = func;
        info.iterations = iterations;
        tests.push_back(info);
    }
    
    int RunAllPropertyTests() {
        int total = 0;
        int passed = 0;
        int failed = 0;
        
        std::cout << "[==========] Running " << tests.size() << " property tests." << std::endl;
        
        for (size_t i = 0; i < tests.size(); i++) {
            const PropertyTestInfo& test = tests[i];
            total++;
            
            std::cout << "[ RUN      ] " << test.test_suite_name << "." << test.test_name << std::endl;
            std::cout << "             Feature: " << test.feature_name << std::endl;
            std::cout << "             Property: " << test.property_name << std::endl;
            std::cout << "             Iterations: " << test.iterations << std::endl;
            
            bool test_passed = true;
            int failed_iteration = -1;
            
            try {
                for (int iter = 0; iter < test.iterations; iter++) {
                    if (!test.test_func()) {
                        test_passed = false;
                        failed_iteration = iter;
                        break;
                    }
                }
                
                if (test_passed) {
                    std::cout << "[       OK ] " << test.test_suite_name << "." << test.test_name 
                              << " (" << test.iterations << " iterations)" << std::endl;
                    passed++;
                } else {
                    std::cout << "[  FAILED  ] " << test.test_suite_name << "." << test.test_name 
                              << " (failed at iteration " << failed_iteration << ")" << std::endl;
                    failed++;
                }
            } catch (const std::exception& e) {
                std::cout << "[  FAILED  ] " << test.test_suite_name << "." << test.test_name << std::endl;
                std::cout << "           Exception: " << e.what() << std::endl;
                failed++;
            }
        }
        
        std::cout << "[==========] " << total << " property tests ran." << std::endl;
        std::cout << "[  PASSED  ] " << passed << " tests." << std::endl;
        
        if (failed > 0) {
            std::cout << "[  FAILED  ] " << failed << " tests." << std::endl;
        }
        
        return failed;
    }
    
private:
    std::vector<PropertyTestInfo> tests;
};

//-----------------------------------------------------------------------------
// Property Test Registration Helper
//-----------------------------------------------------------------------------
class PropertyTestRegistrar {
public:
    PropertyTestRegistrar(
        const std::string& feature,
        const std::string& property,
        const std::string& suite_name,
        const std::string& test_name,
        std::function<bool()> func,
        int iterations = RC_MIN_ITERATIONS)
    {
        PropertyTestRegistry::Instance().RegisterPropertyTest(
            feature, property, suite_name, test_name, func, iterations);
    }
};

//-----------------------------------------------------------------------------
// Assertion Macros for Property Tests
//-----------------------------------------------------------------------------
#define RC_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            return false; \
        } \
    } while (0)

//-----------------------------------------------------------------------------
// Property Test Definition Macro
//-----------------------------------------------------------------------------
#define RC_GTEST_PROP(test_suite_name, test_name, feature, property, ...) \
    bool test_suite_name##_##test_name##_PropertyBody(); \
    PropertyTestRegistrar test_suite_name##_##test_name##_prop_registrar( \
        feature, property, #test_suite_name, #test_name, \
        test_suite_name##_##test_name##_PropertyBody, RC_MIN_ITERATIONS); \
    bool test_suite_name##_##test_name##_PropertyBody()

//-----------------------------------------------------------------------------
// Main Property Test Runner
//-----------------------------------------------------------------------------
#define RUN_ALL_PROPERTY_TESTS() PropertyTestRegistry::Instance().RunAllPropertyTests()

#endif // RAPIDCHECK_MINIMAL_H
