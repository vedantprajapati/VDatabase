// -*- c++ -*-

#ifndef RPC_H
#define RPC_H

#include <cstdint>
#include <cstddef>
#include <array>
#include <algorithm>
#include <cstring>
#include <arpa/inet.h> // for htonl
#include <atomic>

namespace rpc {

class BaseService;

// Member Functions are 16B according to Itantium ABI.
struct MemberFunctionPtr {
  void *fp;
  ptrdiff_t this_diff;

  bool operator==(const MemberFunctionPtr &rhs) {
    return fp == rhs.fp && this_diff == rhs.this_diff;
  }

  template <typename MemberFunction>
  static MemberFunctionPtr From(MemberFunction f) {
    static_assert(sizeof(MemberFunction) == sizeof(MemberFunctionPtr),
                  "Member Function Pointer isn't 16 bytes?");
    MemberFunctionPtr ptr;
    memcpy(&ptr, &f, sizeof(MemberFunctionPtr));
    return ptr;
  }

  template <typename MemberFunction>
  MemberFunction To() {
    static_assert(sizeof(MemberFunction) == sizeof(MemberFunctionPtr),
                  "Member Function Pointer isn't 16 bytes?");
    MemberFunction f;
    memcpy(&f, this, sizeof(MemberFunctionPtr));
    return f;
  }
};

class BaseProcedure {
  friend class Connection;
  friend class BaseService;
 protected:
  MemberFunctionPtr func_ptr;
  BaseService *instance;
  virtual ~BaseProcedure() {}

  virtual bool DecodeAndExecute(uint8_t *in_bytes, uint32_t *in_len,
                                uint8_t *out_bytes, uint32_t *out_len,
                                bool *ok) = 0;
};

class BaseParams {
  friend class BaseClient;
 protected:
  void *func_ptr = nullptr;
  virtual bool Encode(uint8_t *out_bytes, uint32_t *out_len) const = 0;
 public:
  virtual ~BaseParams() {}
};

class BaseResult {
  friend class BaseClient;
 protected:
  bool ready = false;
  bool error = false;
  virtual bool HandleResponse(uint8_t *in_bytes, uint32_t *in_len, bool *ok) = 0;
  virtual ~BaseResult() {}
 public:
  bool is_ready() const { return ready; }
  bool has_error() const { return error; }
};

class BaseService {
  friend class Server;
  friend class Connection;
 public:
  virtual ~BaseService();

  static constexpr size_t kMaxRequestSize = 4096;
  static constexpr size_t kMaxResponseSize = 128;
  static constexpr size_t kMaxPipelineRequests = 8;

  void set_instance_id(int id) { ins_id = id; }
  int instance_id() const { return ins_id; }

  int LookupExportFunction(MemberFunctionPtr func_ptr);

 protected:
  static constexpr size_t kMaxDesc = 128;

  void ExportRaw(MemberFunctionPtr func_ptr, BaseProcedure *proc);
  
 private:
  std::array<BaseProcedure *, kMaxDesc> proc_entries;
  size_t nr_entries = 0;
  int ins_id;
};

class BaseClient {
  uint8_t *buf;
  size_t bufsz;
  std::array<BaseResult*, BaseService::kMaxPipelineRequests> pending;
  size_t nr_pending;
  int fd;
  bool error;
  unsigned int xid;
  bool log_enabled;
 public:
  BaseClient();
  ~BaseClient();
  BaseClient(const BaseClient &rhs) = delete;

  bool Connect(const char *addr, unsigned int port);
  bool Send(int instance_id, int func_id, BaseParams *params, BaseResult *result);
  void Flush();

  bool has_error() const { return error; }
  void set_log_enabled(bool enabled) { log_enabled = enabled; }
 private:
  bool ParseBuffer(uint8_t *inbytes, uint32_t *in_len, int nr_replied, bool *ok);
};

class Connection;

class Server {
  static constexpr size_t kMaxServices = 128;
  friend class Connection;

  int epoll_fd = 0;
  int sock;
  uint64_t last_event_ts = 0;
  Connection *lru = nullptr, *mru = nullptr;
  std::array<BaseService *, kMaxServices> svc_table;
  size_t nr_svc = 0;
  std::atomic_bool should_stop;
  bool log_enabled = true;
 public:
  Server();
  ~Server();

  bool AddService(BaseService *svc, int instance_id);
  bool Listen(const char *addr, unsigned short port);
  void MainLoop();
  void SignalStop();

  void set_log_enabled(bool enabled) { log_enabled = enabled; }
 private:
  void OnNewConnection();
  bool OnConnectionEvent(Connection *conn, uint32_t event_mask);
  void CheckTimeout();
  bool CloseConnection(Connection *conn);
  bool ReadConnectionBuffer(Connection *conn);
  bool WriteConnectionBuffer(Connection *conn);
  static uint32_t ConnectionPollMask(Connection *conn);

  BaseService *LookupService(int instance_id, int func_id);
};

}

#endif
