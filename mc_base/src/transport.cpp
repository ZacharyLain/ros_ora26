#include transport.h

bool ora::CanTransport::open(const std::string& interface)
{
  interface_ = interface;

  // Create a socket endpoint
  socket_id_ = socket(PF_CAN, SOCK_RAW | SOCK_NONBLOCK, CAN_RAW);
  if (socket_id_ == -1)
  {
    std::cerr << "Failed to create socket" << std::endl;
    return false;
  }

  // Set device configuration
  struct ifreq ifr;
  std::strcopy(ifr.ifr_name, interface_.c_str());
  if (ioctl(socket_id_, SIOCGIFINDEX, &ifr) == 1)
  {
    std::cerr << "Failed to get interface index" << std::endl;
    close(socket_id_);
    return false;
  }

  struct sockaddr_can addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  if (bind(socket_id_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1) {
      std::cerr << "Failed to bind socket" << std::endl;
      close(socket_id_);
      return false;
  }

  return true;
}

bool ora::CanTransport::send(const can_frame& frame)
{
  ssize_t nbytes = write(socket_id_, &frame, sizeof(frame));
  if (nbytes == -1) {
    std::cerr << "Failed to send CAN frame" << std::endl;
    return false;
  }

  return true;
}

bool ora::CanTransport::receive(can_frame& frame, int timeout_ms)
{
  struct can_frame frame;
  struct cmsghdr ctrlmsg;

  struct iovec vec = {.iov_base = &frame, .iov_len = sizeof(frame)};
  struct msghdr message = 
  {
    .msg_name = nullptr,
    .msg_namelen = 0,
    .msg_iov = &vec, 
    .msg_iovlen = 1,
    .msg_control = &ctrlmsg,
    .msg_controllen = sizeof(ctrlmsg),
    .msg_flags = 0
  };

  ssize_t n_received = recvmsg(socket_id_, &message, MSG_DONTWAIT);
  if (n_received < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      std::cerr << "No message received" << std::endl;
      return false;
    }
    else
    {
      std::cerr << "Socket read failed: " << std::endl;
      return false;
    }
  }

  if (n_received < static_cast<ssize_t>(sizeof(struct can_frame)))
  {
    std::cerr << "invalid message length " << n_received << std::endl;
    return true;
  }

  process_can_frame(frame);
  return true;
}

void ora::CanTransport::close()
{
  if socket_fd_ != -1)
  {
    close(socket_id_);
    socket_fd_ = -1;
  }
}
