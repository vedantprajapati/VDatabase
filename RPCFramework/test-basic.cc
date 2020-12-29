#include <gtest/gtest.h>
#include "rpcxx.h"

namespace {

class BaseTest {
public:
    virtual bool Encode(uint8_t * buf, uint32_t * len) = 0;
    virtual bool Decode(uint8_t * buf, uint32_t * len, bool * ok) = 0;
    virtual bool IsEqual() const = 0;
    virtual std::string Debug() const = 0;
};

template<typename T>
class DerivedTest : BaseTest {
    T in;
    T out;
    
public:
    DerivedTest(T && in) : in(in) {}
    
    virtual bool Encode(uint8_t * buf, uint32_t * len) override {
        return rpc::Protocol<T>::Encode(buf, len, in);
    }
    
    virtual bool Decode(uint8_t * buf, uint32_t * len, bool * ok) override { 
        return rpc::Protocol<T>::Decode(buf, len, ok, out);
    }
    
    virtual bool IsEqual() const override { return in == out; }
    
    virtual std::string Debug() const override {
        std::stringstream ss;
        ss << "in = " << in << ", out = " << out;
        return ss.str();
    }
};

#define MAX_LEN 256

class BasicTest : public testing::Test {
protected:
    uint32_t in_len;
    uint32_t out_len;
    uint8_t * in_buf;
    uint8_t * out_buf;        
       
    void SetUp() override {
        in_len = 0;
        out_len = 0;
        in_buf = nullptr;
        out_buf = nullptr; 
    }

    void TearDown() override {
        delete [] in_buf;
        delete [] out_buf;
    }
};

#define BASIC_TEST(TEST_NAME, TEST_TYPE, TEST_VALUE) \
TEST_F(BasicTest, TEST_NAME) { \
    auto tname = #TEST_TYPE; \
    auto test = DerivedTest<TEST_TYPE>(TEST_VALUE); \
    bool ret = false; \
    bool ok = true; \
    uint32_t out_size = 0, in_size = 0, temp; \
    \
    ASSERT_FALSE(test.Encode(in_buf, &out_len)) << \
        "Encode<" << tname << "> returned true on nullptr buffer"; \
    \
    for (out_len = 1; out_len <= MAX_LEN; out_len *= 2) { \
        in_buf = new uint8_t[out_len];    \
        out_size = out_len; \
        if ((ret = test.Encode(in_buf, &out_size))) { \
            break; \
        } \
        else { \
            delete [] in_buf; \
            in_buf = nullptr; \
        } \
    } \
    \
    EXPECT_LE(out_size, out_len) << "Encode<" << tname << "> set *out_len to " \
        << out_size << ", which is greater than " << out_len << " bytes."; \
    ASSERT_TRUE(ret) << "Decode<" << tname << "> failed on " \
        << "buffer size of " << MAX_LEN << " bytes."; \
    temp = out_size; \
    EXPECT_TRUE(test.Encode(in_buf, &temp)) << "Encode<" << tname << \
        "> failed on buffer size of " << out_size << " bytes."; \
    EXPECT_EQ(temp, out_size); \
    \
    delete [] in_buf; \
    temp = out_len * 2; \
    in_buf = new uint8_t[temp]; \
    EXPECT_TRUE(test.Encode(in_buf, &temp)) << "Encode<" << tname << \
        "> failed on buffer size of " << out_len * 2 << " bytes."; \
    EXPECT_EQ(temp, out_size) << "Encode<" << tname << "> set *out_len to " \
        << temp << " on buffer size of " << out_len * 2 << " bytes."; \
    \
    ASSERT_FALSE(test.Decode(out_buf, &in_len, &ok)) \
        << "Decode<" << tname << "> returned true on nullptr buffer"; \
    for (in_len = 1; in_len < out_len; in_len *= 2) { \
        out_buf = new uint8_t[in_len]; \
        memcpy(out_buf, in_buf, in_len); \
        in_size = in_len; \
        ret = test.Decode(out_buf, &in_size, &ok); \
        EXPECT_FALSE(ret) << "Encode<" << tname << "> required at least " \
            << out_len << " bytes, but Decode<" << tname << "> returns " \
            << "true on " << in_len << " bytes!"; \
        delete [] out_buf; \
    } \
    \
    out_buf = new uint8_t[in_len]; \
    memcpy(out_buf, in_buf, in_len); \
    in_size = in_len; \
    ret = test.Decode(out_buf, &in_size, &ok); \
    ASSERT_TRUE(ret) << "Decode<" << tname << "> returns false on " \
        << in_len << " bytes, but Encode<" << tname << "> succeeded on " \
        << out_len << " bytes!"; \
    EXPECT_LE(in_size, in_len) << "Decode<" << tname << "> set *in_len to " \
        << in_size << ", which is greater than " << in_len << " bytes."; \
    temp = in_size; \
    EXPECT_TRUE(test.Decode(out_buf, &temp, &ok)) << "Decode<" << tname << \
        "> failed on buffer size of " << in_size << " bytes."; \
    EXPECT_EQ(temp, in_size); \
    EXPECT_EQ(ok, true); \
    EXPECT_EQ(out_size, in_size) << "Mismatch on amount encoded vs. amount " \
        << "decoded"; \
    ASSERT_TRUE(test.IsEqual()) << test.Debug(); \
    \
    delete [] out_buf; \
    temp = in_len * 2; \
    out_buf = new uint8_t[temp]; \
    ASSERT_EQ(temp, out_len * 2); \
    memcpy(out_buf, in_buf, temp); \
    EXPECT_TRUE(test.Decode(out_buf, &temp, &ok)) << "Decode<" << tname << \
        "> failed on buffer size of " << in_len * 2 << " bytes."; \
    EXPECT_EQ(temp, in_size) << "Decode<" << tname << "> set *in_len to " \
        << temp << " on buffer size of " << in_len * 2 << " bytes."; \
    \
    EXPECT_EQ(ok, true); \
    EXPECT_EQ(out_size, in_size) << "Mismatch on amount encoded vs. amount " \
        << "decoded"; \
    ASSERT_TRUE(test.IsEqual()) << test.Debug(); \
}

BASIC_TEST(BoolTest, bool, true)
BASIC_TEST(FloatTest, float, -123.45678)
BASIC_TEST(DoubleTest, double, 69e-5)
BASIC_TEST(CharTest, char, 'f')
BASIC_TEST(UCharTest, unsigned char, 'k')
BASIC_TEST(ShortTest, short, -1)
BASIC_TEST(UShortTest, unsigned short, 2)
BASIC_TEST(IntTest, int, 1 << 31)
BASIC_TEST(UIntTest, unsigned int, 1 << 31)
BASIC_TEST(LongTest, long, -1)
BASIC_TEST(ULongTest, long, 100)
BASIC_TEST(LongLongTest, long long, ~(1LL << 63))
BASIC_TEST(ULongLongTest, unsigned long long, 0xdeadbeef12345678)
BASIC_TEST(StringTest, std::string, 
    "The quick brown fox jumps over the lazy dog.")
BASIC_TEST(EmptyStringTest, std::string, "")
}

