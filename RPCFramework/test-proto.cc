#include <gtest/gtest.h>
#include "rpcxx.h"

namespace {

class ProtocolTest : public testing::Test {
protected:
    uint8_t *buf;
    uint32_t len;
    bool ok;
  
    void SetUp() override {
        buf = new uint8_t[256];
        len = 256;
        ok = true;
    }

    void TearDown() override {
        delete [] buf;
    }
};

TEST_F(ProtocolTest, UnalignedTest)
{
  double x = 3.14159265358979323846;
  double y = 0.0;
  auto p = buf + 1;

  EXPECT_EQ(true, rpc::Protocol<double>::Encode(p, &len, x));
  EXPECT_EQ(true, rpc::Protocol<double>::Decode(p, &len, &ok, y));

  ASSERT_EQ(ok, true);
  ASSERT_EQ(y, x);
}

TEST_F(ProtocolTest, LongTest)
{
    auto k = std::array<long, 3>({std::numeric_limits<long>::min(), -1,
        std::numeric_limits<long>::max()});
    
    for (long x : k) {
        long y = 0xdeadbeefc011fefe;
        len = 256;
        ok = true;
        EXPECT_EQ(true, rpc::Protocol<long>::Encode(buf, &len, x));
        EXPECT_EQ(true, rpc::Protocol<long>::Decode(buf, &len, &ok, y));

        ASSERT_EQ(ok, true);
        ASSERT_EQ(y, x);
    }
}

TEST_F(ProtocolTest, CharTest)
{
    char arr[] = "1234abcd+-*/";
    char x = 'x';
    char & y = arr[4];

    EXPECT_EQ(true, rpc::Protocol<char>::Encode(buf, &len, x));
    EXPECT_EQ(true, rpc::Protocol<char>::Decode(buf, &len, &ok, y));

    ASSERT_EQ(ok, true);
    ASSERT_EQ(y, x);
    ASSERT_EQ(0, memcmp(arr, "1234xbcd+-*/", sizeof(arr))) << "Decode<char> "
        "overwrote more than 1 byte of data!"; 
}

TEST_F(ProtocolTest, ShortTest)
{
    short arr[] = { -4, -2, 0, 2, 4, 6 };
    short ans[] = { -4, -2, 7, 2, 4, 6 };
    short x = ans[2];
    short & y = arr[2];

    EXPECT_EQ(true, rpc::Protocol<short>::Encode(buf, &len, x));
    EXPECT_EQ(true, rpc::Protocol<short>::Decode(buf, &len, &ok, y));

    ASSERT_EQ(ok, true);
    ASSERT_EQ(y, x);
    ASSERT_EQ(0, memcmp(arr, ans, sizeof(arr))) << "Decode<short> "
        "overwrote more than 2 bytes of data!"; 
}

TEST_F(ProtocolTest, UIntTest)
{
    unsigned int arr[] = { 2, 3, 5, 7, 11, 13 };
    unsigned int ans[] = { 2, 3, 9, 7, 11, 13 };
    unsigned int x = ans[2];
    unsigned int & y = arr[2];

    EXPECT_EQ(true, rpc::Protocol<unsigned int>::Encode(buf, &len, x));
    EXPECT_EQ(true, rpc::Protocol<unsigned int>::Decode(buf, &len, &ok, y));

    ASSERT_EQ(ok, true);
    ASSERT_EQ(y, x);
    ASSERT_EQ(0, memcmp(arr, ans, sizeof(arr))) << "Decode<unsigned int> "
        "overwrote more than 4 bytes of data!"; 
}

TEST_F(ProtocolTest, StringTest)
{
    std::string x = "Four Seasons Landscaping is next to Fantasy Island!";
    std::string y = "";

    EXPECT_EQ(true, rpc::Protocol<std::string>::Encode(buf, &len, x));
    EXPECT_EQ(true, rpc::Protocol<std::string>::Decode(buf, &len, &ok, y));

    ASSERT_EQ(ok, true);
    ASSERT_EQ(x, y);

    len--;
    EXPECT_EQ(false, rpc::Protocol<std::string>::Decode(buf, &len, &ok, y));
    ASSERT_EQ(ok, true);
}

TEST_F(ProtocolTest, EmptyStringTest)
{
    EXPECT_EQ(true, rpc::Protocol<std::string>::Encode(buf, &len, 
        std::string("")));

    std::string x("BIGWIN");
    EXPECT_EQ(true, rpc::Protocol<std::string>::Decode(buf, &len, &ok, x));
    EXPECT_EQ(x, "");
}

#define UPDATE_BUFFER() \
    ASSERT_GE(remain, len) << "Encode wrote more than buffer size of " \
        << remain << " bytes!"; \
    ptr += len; \
    remain -= len; \
    len = remain

TEST_F(ProtocolTest, BigTestA)
{
    auto remain = len;
    auto ptr = buf;
    
    EXPECT_EQ(true, rpc::Protocol<std::string>::Encode(ptr, &len, 
        std::string("hello")));
    UPDATE_BUFFER();
  
    const float planck = 6.62607015e-34;
    EXPECT_EQ(true, rpc::Protocol<float>::Encode(ptr, &len, planck));
    UPDATE_BUFFER();
    
    EXPECT_EQ(true, rpc::Protocol<short>::Encode(ptr, &len, 8080));
    UPDATE_BUFFER();
    
    const unsigned long ulv = std::numeric_limits<unsigned long>::max();
    EXPECT_EQ(true, rpc::Protocol<unsigned long>::Encode(ptr, &len, ulv));
    UPDATE_BUFFER();
    
    auto total = remain;
    ptr = buf;
    remain = len = 256;
    
    std::string ss;
    EXPECT_EQ(true, rpc::Protocol<std::string>::Decode(ptr, &len, &ok, ss));
    ASSERT_EQ(ok, true);
    ASSERT_EQ(ss, "hello");
    UPDATE_BUFFER();
    
    float f;
    EXPECT_EQ(true, rpc::Protocol<float>::Decode(ptr, &len, &ok, f));
    ASSERT_EQ(ok, true);
    ASSERT_EQ(planck, f);
    UPDATE_BUFFER();
    
    short s;
    EXPECT_EQ(true, rpc::Protocol<short>::Decode(ptr, &len, &ok, s));
    ASSERT_EQ(ok, true);
    ASSERT_EQ(s, 8080);
    UPDATE_BUFFER();
    
    unsigned long ul;
    EXPECT_EQ(true, rpc::Protocol<unsigned long>::Decode(ptr, &len, &ok, ul));
    ASSERT_EQ(ok, true);
    ASSERT_EQ(ul, ulv);
    UPDATE_BUFFER();
    
    ASSERT_EQ(total, remain) << "amount encoded does not equal to "
        "amount decoded";
}

TEST_F(ProtocolTest, BigTestB)
{
    auto remain = len;
    auto ptr = buf;
    
    EXPECT_EQ(true, rpc::Protocol<unsigned char>::Encode(ptr, &len, 'c'));
    UPDATE_BUFFER();
  
    EXPECT_EQ(true, rpc::Protocol<unsigned short>::Encode(ptr, &len, 666));
    UPDATE_BUFFER();
    
    EXPECT_EQ(true, rpc::Protocol<bool>::Encode(ptr, &len, false));
    UPDATE_BUFFER();
    
    const double dv = std::numeric_limits<double>::max();
    EXPECT_EQ(true, rpc::Protocol<double>::Encode(ptr, &len, dv));
    UPDATE_BUFFER();
    
    auto total = remain;
    ptr = buf;
    remain = len = 256;
    
    unsigned char uc;
    EXPECT_EQ(true, rpc::Protocol<unsigned char>::Decode(ptr, &len, &ok, uc));
    ASSERT_EQ(ok, true);
    ASSERT_EQ(uc, 'c');
    UPDATE_BUFFER();
    
    unsigned short us;
    EXPECT_EQ(true, rpc::Protocol<unsigned short>::Decode(ptr, &len, &ok, us));
    ASSERT_EQ(ok, true);
    ASSERT_EQ(666, us);
    UPDATE_BUFFER();
    
    bool b;
    EXPECT_EQ(true, rpc::Protocol<bool>::Decode(ptr, &len, &ok, b));
    ASSERT_EQ(ok, true);
    ASSERT_EQ(b, false);
    UPDATE_BUFFER();
    
    double d;
    EXPECT_EQ(true, rpc::Protocol<double>::Decode(ptr, &len, &ok, d));
    ASSERT_EQ(ok, true);
    ASSERT_EQ(d, dv);
    UPDATE_BUFFER();
    
    ASSERT_EQ(total, remain) << "amount encoded does not equal to "
        "amount decoded";
}

}
