/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NFD_DAEMON_FACE_UDP_CHANNEL_HPP
#define NFD_DAEMON_FACE_UDP_CHANNEL_HPP

#include "channel.hpp"
#include "udp-protocol.hpp"

#include <array>

namespace nfd {
namespace face {

/**
 * \brief Class implementing UDP-based channel to create faces
 */
class UdpChannel : public Channel
{
public:
  /**
   * \brief Create UDP channel for the local endpoint
   *
   * To enable creation of faces upon incoming connections,
   * one needs to explicitly call UdpChannel::listen method.
   * The created socket is bound to the localEndpoint.
   * reuse_address option is set
   *
   * \throw UdpChannel::Error if bind on the socket fails
   */
  UdpChannel(const udp::Endpoint& localEndpoint,
             const time::seconds& timeout);

  bool
  isListening() const override
  {
    return m_socket.is_open();
  }

  size_t
  size() const override
  {
    return m_channelFaces.size();
  }

  /**
   * \brief Create a face by establishing connection to remote endpoint
   *
   * \throw UdpChannel::Error if bind or connect on the socket fail
   */
  void
  connect(const udp::Endpoint& remoteEndpoint,
          ndn::nfd::FacePersistency persistency,
          const FaceCreatedCallback& onFaceCreated,
          const FaceCreationFailedCallback& onConnectFailed);

  /**
   * \brief Enable listening on the local endpoint, accept connections,
   *        and create faces when remote host makes a connection
   *
   * \param onFaceCreated Callback to notify successful creation of a face
   * \param onFaceCreationFailed Callback to notify errors
   *
   * Once a face is created, if it doesn't send/receive anything for
   * a period of time equal to timeout, it will be destroyed
   */
  void
  listen(const FaceCreatedCallback& onFaceCreated,
         const FaceCreationFailedCallback& onFaceCreationFailed);

private:
  void
  waitForNewPeer(const FaceCreatedCallback& onFaceCreated,
                 const FaceCreationFailedCallback& onReceiveFailed);

  /**
   * \brief The channel has received a new packet from a remote
   *        endpoint that is not associated with any UdpFace yet
   */
  void
  handleNewPeer(const boost::system::error_code& error,
                size_t nBytesReceived,
                const FaceCreatedCallback& onFaceCreated,
                const FaceCreationFailedCallback& onReceiveFailed);

  std::pair<bool, shared_ptr<Face>>
  createFace(const udp::Endpoint& remoteEndpoint,
             ndn::nfd::FacePersistency persistency);

private:
  const udp::Endpoint m_localEndpoint;
  udp::Endpoint m_remoteEndpoint; ///< The latest peer that started communicating with us
  boost::asio::ip::udp::socket m_socket; ///< Socket used to "accept" new peers
  std::map<udp::Endpoint, shared_ptr<Face>> m_channelFaces;
  std::array<uint8_t, ndn::MAX_NDN_PACKET_SIZE> m_receiveBuffer;
  time::seconds m_idleFaceTimeout; ///< Timeout for automatic closure of idle on-demand faces
};

} // namespace face
} // namespace nfd

#endif // NFD_DAEMON_FACE_UDP_CHANNEL_HPP
