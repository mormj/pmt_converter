#pragma once

#include <pmtv/pmt.hpp>
#include <vector>
#include <cstdint>

namespace legacy_pmt {

/**
 * Serialize a pmtv::pmt into the legacy GNU Radio PMT binary format.
 * Returns a vector of bytes that can be passed to a ZMQ socket or saved to a file.
 */
std::vector<uint8_t> serialize_to_legacy(const pmtv::pmt& obj);

/**
 * Deserialize a binary blob (legacy GNU Radio PMT format) into a pmtv::pmt.
 * Throws std::runtime_error if the data is malformed or unrecognized.
 */
pmtv::pmt deserialize_from_legacy(const uint8_t* data, size_t size);

} // namespace legacy_pmt
