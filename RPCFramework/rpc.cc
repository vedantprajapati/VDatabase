#include <cstdio>
#include <memory>
#include <cstring>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#include "rpc.h"

namespace rpc {

// Network stuff

// SlidingBuffer is a lot easier. It needs memory copy, but should be rare if
// most of requests are small.
//
// It has one major advantage: it presents continuous buffer space.
struct SlidingBuffer {
  uint8_t *p = nullptr;
  uint32_t start = 0;
  uint32_t end = 0;
  uint32_t size = 0;

  bool Slide(uint32_t reserve) {
    if (residual_size() <= reserve) {
      memmove(p, p + start, end - start);
      end = end - start;
      start = 0;
    }
    return residual_size() > reserve;
  }

  uint32_t residual_size() const { return size - end; }
  uint8_t *residual() { return p + end; }
  uint32_t data_size() const { return end - start; }
  uint8_t *data() { return p + start; }

};

class Connection final {
  friend class Server;
  static constexpr size_t kMaxInBuf = 2 * BaseService::kMaxRequestSize;
  static constexpr size_t kMaxOutBuf = 2 * BaseService::kMaxResponseSize;

  SlidingBuffer inbuf;
  SlidingBuffer outbuf;
  Connection *next_lru;
  Connection *next_mru;
  uint64_t last_active_sec;
  Server *srv;
  int fd;
  bool has_error;
 public:
  Connection(Server *srv, int fd);
  Connection(const Connection &rhs) = delete;
  ~Connection();

  void MarkActive();
 private:
  bool HandleRequest(uint8_t *in_bytes, uint32_t *in_len,
                     uint8_t *out_bytes, uint32_t *out_len);
  void DeleteFromLRU();
  template <typename T, typename ...Args> bool FillErrorResponse(
      uint8_t *out_bytes, uint32_t *out_len, Args... args) {
    if (*out_len < sizeof(T)) return false;
    has_error = true;
    *out_len = sizeof(T);
    new (out_bytes) T(args...);
    return true;
  }
};


static bool SetSocketNonBlocking(int sock)
{
  int fl = fcntl(sock, F_GETFL);
  if (fcntl(sock, F_SETFL, fl | O_NONBLOCK) < 0) {
    perror("fcntl");
    return false;
  }
  return true;
}

static bool SetSocketBlocking(int sock)
{
  int fl = fcntl(sock, F_GETFL);
  if (fcntl(sock, F_SETFL, fl & ~O_NONBLOCK) < 0) {
    perror("fcntl");
    return false;
  }
  return true;
}

static uint64_t GetTimestamp()
{
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return tv.tv_sec;
}

Connection::Connection(Server *srv, int fd)
    : srv(srv), fd(fd), has_error(false)
{
  inbuf.p = new uint8_t[kMaxInBuf];
  inbuf.size = kMaxInBuf;
  outbuf.p = new uint8_t[kMaxOutBuf];
  outbuf.size = kMaxOutBuf;
  next_lru = next_mru = this;

  MarkActive();
}

void Connection::DeleteFromLRU()
{
  *(next_lru ? &next_lru->next_mru : &srv->mru) = next_mru;
  *(next_mru ? &next_mru->next_lru : &srv->lru) = next_lru;
}

Connection::~Connection()
{
  DeleteFromLRU();
  close(fd);
  delete [] inbuf.p;
  delete [] outbuf.p;
}

void Connection::MarkActive()
{
  last_active_sec = GetTimestamp();

  if (next_lru != this) {
    DeleteFromLRU();
  }

  next_mru = srv->mru;
  next_lru = nullptr;

  *(next_mru ? &next_mru->next_lru : &srv->lru) = this;
  srv->mru = this;
}

struct SunRpcCallBody {
  unsigned int xid;
  unsigned int type;
  unsigned int rpcvers;
  unsigned int prog;
  unsigned int vers;
  unsigned int proc;
  unsigned int cred_null; // Authentications, we only support NULL
  unsigned int cred_length;
  unsigned int verf_null;
  unsigned int verf_length;
};

struct SunRpcReplyHeader {
  unsigned int xid;
  unsigned int type;
  unsigned int reply_stat;

  SunRpcReplyHeader(unsigned int xid) : xid(xid), type(htonl(1)), reply_stat(0) {}
};

struct SunRpcRejectAuthBody : public SunRpcReplyHeader {
  unsigned int reject_stat;
  unsigned int auth_stat;

  SunRpcRejectAuthBody(unsigned int xid, unsigned int auth_stat)
      : SunRpcReplyHeader(xid), reject_stat(htonl(1)), auth_stat(htonl(auth_stat)) {
    reply_stat = htonl(1);
  }
};

struct SunRpcAcceptHeader : public SunRpcReplyHeader {
  unsigned int verf_null;
  unsigned int verf_len; // 0, as we don't support authentication
  unsigned int accept_stat;

  SunRpcAcceptHeader(unsigned int xid, unsigned int accept_stat)
      : SunRpcReplyHeader(xid), verf_null(0), verf_len(0), accept_stat(htonl(accept_stat)) {}
};

struct SunRpcAcceptMismatch : public SunRpcAcceptHeader {
  unsigned int lo = 0, hi = 0;
  using SunRpcAcceptHeader::SunRpcAcceptHeader;
};


bool Connection::HandleRequest(uint8_t *in_bytes, uint32_t *in_len,
                               uint8_t *out_bytes, uint32_t *out_len)
{
  // Since we don't support authentications, our RpcCallBody is fixed-size.
  if (*in_len < sizeof(SunRpcCallBody))
    return false;
  SunRpcCallBody callbody;
  memcpy(&callbody, in_bytes, sizeof(SunRpcCallBody));
  if (callbody.type != 0 || callbody.cred_null != 0 || callbody.cred_length != 0
      || callbody.verf_null != 0 || callbody.verf_length != 0) {
    *in_len = 0;
    return FillErrorResponse<SunRpcRejectAuthBody>(out_bytes, out_len, callbody.xid, 2);
  }
  auto instance_id = ntohl(callbody.prog);
  auto func_id = ntohl(callbody.proc);
  auto svc = srv->LookupService(instance_id, func_id);
  if (svc == nullptr) {
    // PROG_MISMATCH
    fprintf(stderr, "Server cannot find instance %d and procedure %d\n", instance_id, func_id);
    *in_len = 0;
    return FillErrorResponse<SunRpcAcceptMismatch>(out_bytes, out_len, callbody.xid, 2);
  }
  uint32_t param_in_len = *in_len - sizeof(SunRpcCallBody);
  uint32_t param_out_len = *out_len - sizeof(SunRpcAcceptHeader);
  bool ok = true;

  auto proc = svc->proc_entries[func_id];
  if (srv->log_enabled)
    printf("Server invoking instance %d procedure %d\n", instance_id, func_id);
  bool consume = proc->DecodeAndExecute(
      in_bytes + sizeof(SunRpcCallBody), &param_in_len,
      out_bytes + sizeof(SunRpcAcceptHeader), &param_out_len,
      &ok);
  if (!ok) {
    fprintf(stderr, "Procedure::DecodeAndExecute() fail to parse arguments!\n");
    // GARBAGE_ARGS
    *in_len = 0;
    return FillErrorResponse<SunRpcAcceptHeader>(out_bytes, out_len, callbody.xid, 4);
  }
  if (!consume)
    return false;
  new (out_bytes) SunRpcAcceptHeader(callbody.xid, 0);
  *in_len = sizeof(SunRpcCallBody) + param_in_len;
  *out_len = sizeof(SunRpcAcceptHeader) + param_out_len;
  return true;
}

Server::Server()
    : should_stop(false)
{
  epoll_fd = epoll_create(128);
  if (epoll_fd == -1) {
    perror("Fail to create event poll");
    std::abort();
  }
}

Server::~Server()
{
  for (auto conn = mru, conn_next = mru; conn; conn = conn_next) {
    // (jsun): this deletes conn, must save conn->next_mru first
    conn_next = conn->next_mru;
    if (!CloseConnection(conn)) {
      fprintf(stderr, "Cannot close connection %p(%d) on server destruction!", 
        conn, conn->fd);
    }
  }
  for (size_t i = 0; i < nr_svc; i++) {
    delete svc_table[i];
  }

  close(sock);
  close(epoll_fd);
}

void Server::SignalStop()
{
  should_stop = true;
}

bool Server::AddService(BaseService *svc, int instance_id)
{
  svc->set_instance_id(instance_id);
  if (nr_svc >= kMaxServices) {
    fprintf(stderr, "Cannot support more than %lu services", kMaxServices);
    return false;
  }
  svc_table[nr_svc++] = svc;

  return true;
}

BaseService *Server::LookupService(int instance_id, int func_id)
{
  auto it = std::find_if(svc_table.begin(), svc_table.begin() + nr_svc,
                         [instance_id](BaseService *s) {
                           return s->instance_id() == instance_id;
                         });
  if (instance_id < 0 || func_id < 0)
    return nullptr;
  if (it == svc_table.begin() + nr_svc)
    return nullptr;

  auto svc = *it;
  if ((size_t) func_id < svc->nr_entries)
    return svc;
  return nullptr;
}

bool Server::Listen(const char *addr, unsigned short port)
{
  sock = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in soaddr;
  int reuseval = 1;

  if (sock < 0) {
    perror("Cannot create server socket");
    goto fail;
  }

  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseval, sizeof(int));

  soaddr.sin_family = AF_INET;
  soaddr.sin_port = htons(port);
  inet_aton(addr, &soaddr.sin_addr);

  if (bind(sock, (struct sockaddr *) &soaddr,
           sizeof(struct sockaddr_in)) < 0) {
    fprintf(stderr, "Cannot bind to %s:%d error %s", addr, port, strerror(errno));
    goto fail;
  }

  if (listen(sock, 128) < 0) {
    fprintf(stderr, "Cannot listen on %s:%d error %s", addr, port, strerror(errno));
    goto fail;
  }

  if (SetSocketNonBlocking(sock) == false) {
    goto fail;
  }

  return true;

fail:
  if (sock > 0) close(sock);
  return false;
}


void Server::CheckTimeout()
{
  auto new_ts = GetTimestamp();
  if (new_ts == last_event_ts)
    return;

  last_event_ts = new_ts;
#if 0
  puts("LRU Direction:");
  for (auto conn = lru; conn; conn = conn->next_lru) {
    printf(" Conn %p ts %lu %p-%p\n", conn, conn->last_active_sec, conn->next_lru, conn->next_mru);
  }
  puts("MRU Direction:");
  for (auto conn = mru; conn; conn = conn->next_mru) {
    printf(" Conn %p ts %lu %p-%p\n", conn, conn->last_active_sec, conn->next_mru, conn->next_lru);
  }
#endif
}

static bool IsIOError(ssize_t nbytes)
{
  return (nbytes == 0 || (nbytes < 0 && errno != EWOULDBLOCK));
}

void Server::MainLoop()
{
  struct epoll_event events[128];
  struct epoll_event event;
  int nr = 0;

  // Add the server sock into epoll
  event.data.u64 = 0;
  event.events = EPOLLIN | EPOLLERR;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event) < 0) {
    perror("Adding sock to event poll failed");
    return;
  }

  while (!should_stop.load()) {
    if ((nr = epoll_wait(epoll_fd, events, 128, 100)) < 0) {
      if (errno == EINTR) continue;
      perror("Event Poll error");
      return;
    }
    // printf("Server wakes up with %d events\n", nr);
    while (nr-- > 0) {
      auto e = &events[nr];
      if (e->data.u64 == 0) {
        OnNewConnection();
        continue;
      }
      auto conn = (Connection *) e->data.ptr;
      auto mask = ConnectionPollMask(conn);

      auto ok = OnConnectionEvent(conn, e->events);
      if (!ok) continue;

      auto new_mask = ConnectionPollMask(conn);
      if (((new_mask & EPOLLOUT) == 0 && conn->has_error)
          || new_mask == 0) {
        CloseConnection(conn);
      } else if (new_mask != mask) {
        event.data.ptr = conn;
        event.events = new_mask | EPOLLERR;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, conn->fd, &event) < 0) {
          perror("Error when changing event mask! (Rare)");
        }
      }
    }
    CheckTimeout();
  }
}

void Server::OnNewConnection()
{
  struct sockaddr_in soaddr = {0};
  socklen_t len = 0;
  int newfd = 0;

  memset(&soaddr, 0, sizeof(struct sockaddr_in));
  if ((newfd = accept(sock, (struct sockaddr *) &soaddr, &len)) < 0) {
    if (errno != EWOULDBLOCK && errno != EINPROGRESS)
      perror("accept");
    return;
  }

  if (SetSocketNonBlocking(newfd) == false) {
    goto fail;
  }

  struct epoll_event event;
  event.data.ptr = new Connection(this, newfd);
  event.events = EPOLLIN | EPOLLERR;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, newfd, &event) < 0) {
    perror("Cannot add new client fd to event poll");
    goto fail;
  }

  if (log_enabled)
    printf("Server got new connection from %s\n", inet_ntoa(soaddr.sin_addr));

  return;
fail:
  close(newfd);
  return;
}

bool Server::CloseConnection(Connection *conn)
{
  if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn->fd, nullptr) == 0) {
    if (log_enabled)
      printf("Server closes connection %p %d\n", conn, conn->fd);

    delete conn;
    return true;
  }
  return false;
}

uint32_t Server::ConnectionPollMask(Connection *conn)
{
  uint32_t mask = 0;
  if (conn->inbuf.residual_size() > 0 || conn->inbuf.start > 0) mask |= EPOLLIN;
  if (conn->outbuf.data_size() > 0) mask |= EPOLLOUT;
  return mask;
}

bool Server::OnConnectionEvent(Connection *conn, uint32_t event_mask)
{
  if ((event_mask & EPOLLERR) || (event_mask & EPOLLHUP)) {
    CloseConnection(conn);
    return false;
  }
  conn->MarkActive();

  if (event_mask & EPOLLOUT) {
    if (!WriteConnectionBuffer(conn))
      return false;
  }

  if (event_mask & EPOLLIN) {
    if (!conn->inbuf.Slide(0))
      return true;

    // Read from the network
    if (!ReadConnectionBuffer(conn))
      return false;

    // printf("haserror %d outbuf %d\n", conn->has_error, conn->outbuf.residual_size());
    // Process pipelined requests
    while (conn->outbuf.Slide(BaseService::kMaxResponseSize) && !conn->has_error) {
      uint32_t out_len = conn->outbuf.residual_size();
      uint32_t in_len = conn->inbuf.data_size();
      if (!conn->HandleRequest(conn->inbuf.data(), &in_len,
                               conn->outbuf.residual(), &out_len)) {
        break;
      }
      if (log_enabled)
        printf("Server procedure consumes %d bytes generates %d bytes\n", in_len, out_len);
      conn->outbuf.end += out_len;
      conn->inbuf.start += in_len;
      if (!WriteConnectionBuffer(conn))
        return false;
    }
  }
  return true;


}

bool Server::ReadConnectionBuffer(Connection *conn)
{
  auto nbytes = read(conn->fd, conn->inbuf.residual(), conn->inbuf.residual_size());
  if (IsIOError(nbytes) && CloseConnection(conn)) {
    return false;
  } else if (nbytes > 0) {
    conn->inbuf.end += nbytes;
  }
  return true;
}

bool Server::WriteConnectionBuffer(Connection *conn)
{
  if (conn->outbuf.data_size() == 0)
    return true;

  auto nbytes = write(conn->fd, conn->outbuf.data(), conn->outbuf.data_size());
  if (IsIOError(nbytes) && CloseConnection(conn)) {
    return false;
  } else if (nbytes > 0) {
    conn->outbuf.start += nbytes;
  }
  return true;
}

BaseClient::BaseClient()
    : bufsz(0), nr_pending(0), error(false), xid(1), log_enabled(true)
{
  fd = socket(AF_INET, SOCK_STREAM, 0);
  buf = new uint8_t[BaseService::kMaxRequestSize * BaseService::kMaxPipelineRequests];
}

BaseClient::~BaseClient()
{
  delete [] buf;
  close(fd);
}

bool BaseClient::Connect(const char *addr, unsigned int port)
{
  if (fd < 0) return false; // socket creation failed?
  struct sockaddr_in soaddr;
  memset(&soaddr, 0, sizeof(struct sockaddr_in));
  soaddr.sin_family = AF_INET;
  soaddr.sin_port = htons(port);
  inet_aton(addr, &soaddr.sin_addr);

  if (connect(fd, (const sockaddr *) &soaddr, sizeof(sockaddr_in)) < 0) {
    perror("connect");
    return false;
  }
  error = false;
  return true;
}

void BaseClient::Flush()
{
  ssize_t sent = 0;
  static constexpr size_t kInBufferSize = BaseService::kMaxPipelineRequests * BaseService::kMaxResponseSize;
  uint8_t *inbuf = new uint8_t[kInBufferSize];
  uint32_t insz = 0, instart = 0;
  struct pollfd pfd;
  int nr_replied = 0;

  pfd.fd = fd;
  pfd.events = POLLIN;

  SetSocketBlocking(fd);
  while (sent < (ssize_t) bufsz) {
    auto nbytes = write(fd, buf + sent, bufsz - sent);
    if (IsIOError(nbytes)) {
      goto fail;
    }
    sent += nbytes;
  }
  SetSocketNonBlocking(fd);

  nr_replied = 0;
  while (nr_replied < (int) nr_pending) {
    auto r = poll(&pfd, 1, 0);
    if (r < 0 && errno == EINTR)
      continue;
    if (r < 0) {
      perror("poll");
      goto fail;
    }
    if (pfd.revents & POLLERR) {
      fprintf(stderr, "poll return POLLERR on client\n");
      goto fail;
    }
    if (pfd.revents & POLLIN) {
      int nbytes = read(fd, inbuf + insz, kInBufferSize - insz);
      if (IsIOError(nbytes)) {
        goto fail;
      } else if (nbytes > 0) {
        insz += nbytes;
      }
      bool ok = true;
      while (true) {
        uint32_t len = insz - instart;
        bool result = ParseBuffer(inbuf + instart, &len, nr_replied, &ok);
        if (!ok) {
          fprintf(stderr, "Client Result::HandleResponse() parsing error\n");
          goto fail;
        }
        if (!result) {
          break;
        }
        instart += len;
        nr_replied++;
      }
      if (log_enabled)
        printf("Client receives replied requests %d/%lu\n", nr_replied, nr_pending);
    }
  }

  if (instart < insz) {
    fprintf(stderr, "Garbage buffer left unparsed %u < %u\n", instart, insz);
    goto fail;
  }
  goto finalize;

fail:
  close(fd);
  error = true;
  for (int i = nr_replied; i < (int) nr_pending; i++) {
    pending[i]->error = true;
  }
finalize:
  nr_replied = nr_pending = 0;
  bufsz = 0;
  delete [] inbuf;
}

bool BaseClient::ParseBuffer(uint8_t *buf, uint32_t *in_len, int nr_replied, bool *ok)
{
  auto reply_header = (SunRpcReplyHeader *) buf;
  if (*in_len < sizeof(SunRpcReplyHeader)) return false;

  if (reply_header->type != htonl(1) || reply_header->reply_stat != 0) {
    *ok = false;
    return false;
  }

  auto accept_header = (SunRpcAcceptHeader *) buf;
  if (*in_len < sizeof(SunRpcAcceptHeader)) return false;
  if (accept_header->accept_stat != 0) {
    fprintf(stderr, "Accept Message respond with error code %d\n",
            accept_header->accept_stat);
    *ok = false;
    return false;
  }
  uint32_t len = *in_len - sizeof(SunRpcAcceptHeader);
  if (!pending[nr_replied]->HandleResponse(buf  + sizeof(SunRpcAcceptHeader), &len, ok)) {
    return false;
  }
  if (log_enabled)
    printf("Client received a result of %lu bytes\n", sizeof(SunRpcAcceptHeader) + len);
  *in_len = sizeof(SunRpcAcceptHeader) + len;
  return true;
}

bool BaseClient::Send(int instance_id, int func_id, BaseParams *params, BaseResult *result)
{
  std::unique_ptr<BaseParams> _(params);
  if (nr_pending == BaseService::kMaxPipelineRequests)
    return false;

  uint32_t len = BaseService::kMaxRequestSize * BaseService::kMaxPipelineRequests - bufsz;
  if (len < sizeof(SunRpcCallBody))
    return false;

  len -= sizeof(SunRpcCallBody);
  bool r = params->Encode(buf + bufsz + sizeof(SunRpcCallBody), &len);
  if (!r) return false;

  if (log_enabled)
    printf("Client send call to instance %d func %d\n", instance_id, func_id);

  SunRpcCallBody call;
  memset(&call, 0, sizeof(SunRpcCallBody));
  call.xid = htonl(xid++);
  call.rpcvers = htonl(2);
  call.prog = htonl(instance_id);
  call.proc = htonl(func_id);
  memcpy(buf + bufsz, &call, sizeof(SunRpcCallBody));

  bufsz += sizeof(SunRpcCallBody) + len;
  pending[nr_pending++] = result;
  return true;
}

BaseService::~BaseService() 
{
    for (decltype(nr_entries) i = 0; i < nr_entries; i++) {
        delete proc_entries[i];
    }
}

int BaseService::LookupExportFunction(MemberFunctionPtr func_ptr) 
{
    auto it = std::find_if(
        proc_entries.begin(), proc_entries.begin() + nr_entries,
        [func_ptr](BaseProcedure *proc) {
            return proc->func_ptr == func_ptr;
        });
        
    if (it == proc_entries.begin() + nr_entries) {
        return -1;
    }
    
    return it - proc_entries.begin();
}

void BaseService::ExportRaw(MemberFunctionPtr func_ptr, BaseProcedure *proc) {
    proc->func_ptr = func_ptr;
    proc->instance = this;
    proc_entries.at(nr_entries) = proc;
    nr_entries++;
}

} // namespace

