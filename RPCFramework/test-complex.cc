#include "rpcxx.h"
#include "test-rpc-common.h"
#include "gtest/gtest.h"
#include <sstream>
#include <map>

namespace {

class ComplexService : public rpc::Service<ComplexService> {
  bool initialized = false;
  std::map<std::string, std::string> m;

 public:
  ComplexService() {
    Export(&ComplexService::InitializeSomeRandomThing);
    Export(&ComplexService::CheckInitialized);
    Export(&ComplexService::Guess);
    Export(&ComplexService::Repeat);
    Export(&ComplexService::TestSign);
    Export(&ComplexService::Put);
    Export(&ComplexService::Get);
  }

  void InitializeSomeRandomThing() {
    initialized = true;
  }

  bool CheckInitialized() {
    return initialized;
  }

  std::string Guess(unsigned int x) {
    if (x == 0xc0defefe) {
      return std::string("WIN");
    }
    return std::string("");
  }

  std::string Repeat(std::string s, int times) {
    std::stringstream ss;
    for (int i = 0; i < times; i++) ss << s;
    return ss.str();
  }

  unsigned long TestSign(int x, unsigned int y) {
    unsigned long r = (x >> 1);
    r <<= 32;
    r |= y >> 1;
    return r;
  }

  void Put(std::string key, std::string value) {
    m[key] = value;
  }

  std::string Get(std::string key) {
    auto it = m.find(key);
    if (it == m.end()) return "";
    return it->second;
  }

};

class ComplexServiceTest : public testing::Test, public ServiceTestUtil  {
 protected:
  static constexpr int kInstanceId = 100;
  ComplexService *client_service;
  ComplexService *server_service;
 public:
  void SetUp() override {
    SetUpServer();
    server_service = new ComplexService();
    srv->AddService(server_service, kInstanceId);

    SetUpClient();
    client_service = new ComplexService();
    client_service->set_instance_id(kInstanceId);
  }
  void TearDown() override {
    TearDownClient();
    TearDownServer();
    delete client_service;
    // DO NOT DELETE server_service, it is done during TearDownServer
  }
};

TEST_F(ComplexServiceTest, TestInitialize)
{
  auto res = client->Call(client_service, 
                          &ComplexService::InitializeSomeRandomThing);
  client->Flush();
  delete res;
  
  auto result = client->Call(client_service, &ComplexService::CheckInitialized);
  client->Flush();
  EXPECT_EQ(client->has_error(), false);
  EXPECT_EQ(result->data(), true);
  EXPECT_EQ(server_service->CheckInitialized(), true);

  delete result;
}

TEST_F(ComplexServiceTest, TestString)
{
  auto r1 = client->Call(client_service, &ComplexService::Guess, 0xc0defefe);
  auto r2 = client->Call(client_service, &ComplexService::Repeat, 
                         std::string("WIN"), 10);
  client->Flush();
  EXPECT_EQ(r1->data(), "WIN");
  EXPECT_EQ(r2->data(), "WINWINWINWINWINWINWINWINWINWIN");
  delete r1;
  delete r2;
}

TEST_F(ComplexServiceTest, TestIntegerSigns)
{
  auto r = client->Call(client_service, &ComplexService::TestSign, -1, (unsigned int) -1);
  client->Flush();
  EXPECT_EQ(r->data(), 0xffffffff7fffffff);
  delete r;
}

TEST_F(ComplexServiceTest, TestKVStore)
{
  auto r1 = client->Call(client_service, &ComplexService::Get, std::string("K"));
  auto r2 = client->Call(client_service, &ComplexService::Put, std::string("K"), std::string("Wall"));
  auto r3 = client->Call(client_service, &ComplexService::Get, std::string("K"));
  auto r4 = client->Call(client_service, &ComplexService::Get, std::string("Nothing"));

  client->Flush();

  EXPECT_EQ(r1->data(), "");
  EXPECT_EQ(r3->data(), "Wall");
  EXPECT_EQ(r4->data(), "");

  delete r1;
  delete r2;
  delete r3;
  delete r4;
}

}
