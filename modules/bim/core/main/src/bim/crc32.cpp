// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/crc32.hpp>

#include <boost/crc.hpp>

std::uint32_t bim::crc32(std::span<const char> bytes, std::uint32_t c)
{
  boost::crc_32_type crc(c);
  crc.process_bytes(bytes.data(), bytes.size());
  return crc.checksum();
}
