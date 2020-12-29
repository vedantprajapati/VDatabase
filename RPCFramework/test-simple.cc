#include "test-rpc-common.h"
#include "gtest/gtest.h"
#include <sys/time.h>

namespace {

class SimpleService : public rpc::Service<SimpleService> {
 public:
  SimpleService() {
    Export(&SimpleService::DoHash);
  }

  int DoHash(int x) {
    uint64_t l = x;
    l *= 2654435761;
    return l % 2147483647;
  }
};

class SimpleServiceTest : public testing::Test, public ServiceTestUtil {
 protected:
  static constexpr int kInstanceId = 123;
  SimpleService *client_service;
 public:
  void SetUp() override {
    SetUpServer();
    srv->AddService(new SimpleService(), kInstanceId);

    SetUpClient();
    client_service = new SimpleService();
    client_service->set_instance_id(kInstanceId);
  }

  void TearDown() override {
    TearDownClient();
    TearDownServer();
    delete client_service;
  }
};

TEST_F(SimpleServiceTest, TestOneCall)
{
  auto result = client->Call(client_service, &SimpleService::DoHash, 1998);
  client->Flush();

  ASSERT_EQ(result->data(), 1425526035);
  ASSERT_EQ(result->has_error(), false);
  delete result;
}

TEST_F(SimpleServiceTest, TestPipelineCalls)
{
  auto result = client->Call(client_service, &SimpleService::DoHash, 1998);
  ASSERT_NE(result, nullptr);
  std::vector<decltype(result)> more_results;
  more_results.push_back(result);

  for (size_t i = 0; i < rpc::BaseService::kMaxPipelineRequests; i++) {
    auto r = client->Call(client_service, &SimpleService::DoHash, 1998);
    if (i < rpc::BaseService::kMaxPipelineRequests - 1) {
      EXPECT_NE(r, nullptr);
      more_results.push_back(r);
    } else {
      EXPECT_EQ(r, nullptr);
    }
  }
  client->Flush();
  for (auto r: more_results) {
    EXPECT_EQ(r->data(), 1425526035);
    EXPECT_EQ(r->has_error(), false);
    delete r;
  }
}

TEST_F(SimpleServiceTest, TestConcurrentCalls)
{
  srv->set_log_enabled(false);

  client->set_log_enabled(false);
  auto result = client->Call(client_service, &SimpleService::DoHash, 1998);
  ASSERT_NE(result, nullptr);
  client->Flush();
  ASSERT_EQ(result->has_error(), false);

  delete result;

  std::array<rpc::Client*, 500> all_clients;
  std::vector<decltype(result)> all_results;

  for (auto &cl: all_clients) {
    cl = new rpc::Client();
    bool ok = cl->Connect("127.0.0.1", 3888);
    ASSERT_EQ(ok, true);
    cl->set_log_enabled(false);

    for (size_t i = 0; i < rpc::BaseService::kMaxPipelineRequests; i++)
      all_results.push_back(cl->Call(client_service, &SimpleService::DoHash, 1998));
  }

  struct timeval start;
  gettimeofday(&start, nullptr);

  for (auto cl: all_clients) {
    cl->Flush();
  }

  struct timeval end;
  gettimeofday(&end, nullptr);

  auto total_req = all_clients.max_size() * rpc::BaseService::kMaxPipelineRequests;
  auto duration = 1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000;

  printf("%lu requests done in %lu ms, thru %lu req/s\n",
         total_req, duration, 1000 * total_req / duration);

  for (auto r: all_results) {
    EXPECT_EQ(r->data(), 1425526035);
    EXPECT_EQ(r->has_error(), false);
    delete r;
  }

  for (auto cl: all_clients) {
    delete cl;
  }
}

}
