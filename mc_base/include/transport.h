#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>

namespace ora
{
  class CanTransport
  {
  public:
    bool open(const std::string& interface);  // "can0"
    bool send(const can_frame& frame);
    bool receive(can_frame& frame, int timeout_ms);
    void close();

  private:
    std::string interface_;
    int socket_id_ = -1;
  };
}

#endif // TRANSPORT_H