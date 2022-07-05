/** Copyright (C) 2013 Robert B. Colton
***
*** This file is a part of the ENIGMA Development Environment.
***
*** ENIGMA is free software: you can redistribute it and/or modify it under the
*** terms of the GNU General Public License as published by the Free Software
*** Foundation, version 3 of the license or any later version.
***
*** This application and its source code is distributed AS-IS, WITHOUT ANY
*** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
*** FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
*** details.
***
*** You should have received a copy of the GNU General Public License along
*** with this code. If not, see <http://www.gnu.org/licenses/>
**/

#include "Universal_System/buffers_data.h"
#include "buffers.h"
#include "buffers_internal.h"

#include "Resources/AssetArray.h" // TODO: start actually using for this resource
#include "Graphics_Systems/graphics_mandatory.h"
#include "Graphics_Systems/General/GSsurface.h"
#include "Widget_Systems/widgets_mandatory.h"

#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>

using std::string;

namespace enigma {
AssetArray<BinaryBufferAsset> buffers{};

std::size_t get_free_buffer() {
  for (std::size_t i = 0; i < buffers.size(); i++) {
    if (buffers[i].get() == nullptr) {
      return i;
    }
  }
  return buffers.size();
}

std::vector<std::byte> valToBytes(variant value, enigma_user::buffer_data_t type) {
  return enigma_user::serialize_to_type(value, type);
}
}  // namespace enigma

namespace enigma_user {
std::vector<std::byte> serialize_to_type(variant &value, buffer_data_t type) {
#define BYTE(x) static_cast<std::byte>(x)
  // If a positive value is too large for `intXX_t`, it'll be chopped in half because the conversion from double
  // to signed integer will truncate the highest sign bit and replace it with its own sign bit (which will be 0).
  switch (type) {
    case buffer_u8: case buffer_s8: case buffer_bool: {
      if (value.type != ty_real) {
        DEBUG_MESSAGE("serialize_to_type: Expected numeric value to be passed in", MESSAGE_TYPE::M_FATAL_ERROR);
      }

      std::uint8_t as_int = 0;
      if (value.rval.d > std::numeric_limits<std::int8_t>::max()) {
        as_int = static_cast<std::uint8_t>(value.rval.d);
      } else {
        as_int = static_cast<std::int8_t>(value.rval.d);
      }
      return {BYTE(as_int)};
    }

    case buffer_u16: case buffer_s16: {
      if (value.type != ty_real) {
        DEBUG_MESSAGE("serialize_to_type: Expected numeric value to be passed in", MESSAGE_TYPE::M_FATAL_ERROR);
      }

      std::uint16_t as_int = 0;
      if (value.rval.d > std::numeric_limits<std::int16_t>::max()) {
        as_int = static_cast<std::uint16_t>(value.rval.d);
      } else {
        as_int = static_cast<std::int16_t>(value.rval.d);
      }
      return {BYTE((as_int >> 8) & 0xff), BYTE((as_int) & 0xff)};
    }

    case buffer_u32: case buffer_s32: {
      if (value.type != ty_real) {
        DEBUG_MESSAGE("serialize_to_type: Expected numeric value to be passed in", MESSAGE_TYPE::M_FATAL_ERROR);
      }

      std::uint32_t as_int = 0;
      if (value.rval.d > std::numeric_limits<std::int32_t>::max()) {
        as_int = static_cast<std::uint32_t>(value.rval.d);
      } else {
        as_int = static_cast<std::int32_t>(value.rval.d);
      }

      return {BYTE((as_int >> 24) & 0xff), BYTE((as_int >> 16) & 0xff),
              BYTE((as_int >> 8) & 0xff), BYTE(as_int & 0xff)};
    }

    case buffer_u64: {
      if (value.type != ty_real) {
        DEBUG_MESSAGE("serialize_to_type: Expected numeric value to be passed in", MESSAGE_TYPE::M_FATAL_ERROR);
      }

      std::uint64_t as_int = static_cast<std::int64_t>(value.rval.d);
      return {BYTE((as_int >> 56) & 0xff), BYTE((as_int >> 48) & 0xff),
              BYTE((as_int >> 40) & 0xff), BYTE((as_int >> 32) & 0xff),
              BYTE((as_int >> 24) & 0xff), BYTE((as_int >> 16) & 0xff),
              BYTE((as_int >> 8) & 0xff), BYTE(as_int & 0xff)};
    }

    case buffer_f16:
      DEBUG_MESSAGE("serialize_to_type: buffer_f16 is unimplemented!", MESSAGE_TYPE::M_FATAL_ERROR);
      return {};

    case buffer_f32: {
      static_assert(sizeof(float) == 4, "Expected `float` type to be 4 bytes wide");
      if (value.type != ty_real) {
        DEBUG_MESSAGE("serialize_to_type: Expected numeric value to be passed in", MESSAGE_TYPE::M_FATAL_ERROR);
      }

      std::uint32_t as_int = bit_cast<std::uint32_t>(static_cast<float>(value.rval.d));
      return {BYTE((as_int >> 24) & 0xff), BYTE((as_int >> 16) & 0xff),
              BYTE((as_int >> 8) & 0xff), BYTE(as_int & 0xff)};
    }

    case buffer_f64: {
      static_assert(sizeof(double) == 8, "Expected `double` type to be 8 bytes wide");
      if (value.type != ty_real) {
        DEBUG_MESSAGE("serialize_to_type: Expected numeric value to be passed in", MESSAGE_TYPE::M_FATAL_ERROR);
      }

      std::uint64_t as_int = bit_cast<std::uint64_t>(static_cast<double>(value.rval.d));
      return {BYTE((as_int >> 56) & 0xff), BYTE((as_int >> 48) & 0xff),
              BYTE((as_int >> 40) & 0xff), BYTE((as_int >> 32) & 0xff),
              BYTE((as_int >> 24) & 0xff), BYTE((as_int >> 16) & 0xff),
              BYTE((as_int >> 8) & 0xff), BYTE(as_int & 0xff)};
    }

    case buffer_string: case buffer_text: {
      std::string &val = value.sval();
      std::vector<std::byte> result;
      result.reserve(val.size() + 1);
      std::transform(val.begin(), val.end(), std::back_inserter(result),
                     [](char value) { return std::byte{static_cast<std::uint8_t>(value)}; });
      result.emplace_back(std::byte{0});
      return result;
    }

    default:
      return {};
  }
#undef BYTE
}

variant deserialize_from_type(std::vector<std::byte>::iterator first, std::vector<std::byte>::iterator last, buffer_data_t type) {
#define DOUBLE(x) static_cast<double>(x)
  switch (type) {
    case buffer_u8: case buffer_s8: case buffer_bool: {
      if (std::distance(first, last) != 1) {
        DEBUG_MESSAGE("deserialize_from_type: Expected span to be of correct size", MESSAGE_TYPE::M_FATAL_ERROR);
      }

      std::uint8_t value = static_cast<std::uint8_t>(*first++);
      assert(first == last);

      return {type == buffer_s8 ? DOUBLE(static_cast<int8_t>(value)) : value};
    }

    case buffer_u16: case buffer_s16: {
      if (std::distance(first, last) != 2) {
        DEBUG_MESSAGE("deserialize_from_type: Expected span to be of correct size", MESSAGE_TYPE::M_FATAL_ERROR);
      }

      std::uint16_t value = static_cast<std::uint16_t>(*first++);
      value = (value << 8) | static_cast<std::uint16_t>(*first++);
      assert(first == last);

      return {type == buffer_s16 ? DOUBLE(static_cast<int16_t>(value)) : value};
    }

    case buffer_u32: case buffer_s32: {
      if (std::distance(first, last) != 4) {
        DEBUG_MESSAGE("deserialize_from_type: Expected span to be of correct size", MESSAGE_TYPE::M_FATAL_ERROR);
      }

      std::uint32_t value = static_cast<std::uint32_t>(*(first++));
      value = (value << 8) | static_cast<std::uint32_t>(*(first++));
      value = (value << 8) | static_cast<std::uint32_t>(*(first++));
      value = (value << 8) | static_cast<std::uint32_t>(*(first++));
      assert(first == last);

      return {type == buffer_s32 ? DOUBLE(static_cast<int32_t>(value)) : value};
    }

    case buffer_u64: {
      if (std::distance(first, last) != 8) {
        DEBUG_MESSAGE("deserialize_from_type: Expected span to be of correct size", MESSAGE_TYPE::M_FATAL_ERROR);
      }

      std::uint64_t value = static_cast<std::uint64_t>(*first++);
      value = (value << 8) | static_cast<std::uint64_t>(*first++);
      value = (value << 8) | static_cast<std::uint64_t>(*first++);
      value = (value << 8) | static_cast<std::uint64_t>(*first++);
      value = (value << 8) | static_cast<std::uint64_t>(*first++);
      value = (value << 8) | static_cast<std::uint64_t>(*first++);
      value = (value << 8) | static_cast<std::uint64_t>(*first++);
      value = (value << 8) | static_cast<std::uint64_t>(*first++);
      assert(first == last);

      return {value};
    }

    case buffer_f16:
      assert("deserialize_from_type: buffer_f16 is unimplemented!" && false);

    case buffer_f32: {
      if (std::distance(first, last) != 4) {
        DEBUG_MESSAGE("deserialize_from_type: Expected span to be of correct size", MESSAGE_TYPE::M_FATAL_ERROR);
      }

      std::uint32_t as_int = static_cast<std::uint32_t>(*first++);
      as_int = (as_int << 8) | static_cast<std::uint32_t>(*first++);
      as_int = (as_int << 8) | static_cast<std::uint32_t>(*first++);
      as_int = (as_int << 8) | static_cast<std::uint32_t>(*first++);
      assert(first == last);

      return {bit_cast<float>(as_int)};
    }

    case buffer_f64: {
      if (std::distance(first, last) != 8) {
        DEBUG_MESSAGE("deserialize_from_type: Expected span to be of correct size", MESSAGE_TYPE::M_FATAL_ERROR);
      }

      std::uint64_t as_int = static_cast<std::uint64_t>(*first++);
      as_int = (as_int << 8) | static_cast<std::uint64_t>(*first++);
      as_int = (as_int << 8) | static_cast<std::uint64_t>(*first++);
      as_int = (as_int << 8) | static_cast<std::uint64_t>(*first++);
      as_int = (as_int << 8) | static_cast<std::uint64_t>(*first++);
      as_int = (as_int << 8) | static_cast<std::uint64_t>(*first++);
      as_int = (as_int << 8) | static_cast<std::uint64_t>(*first++);
      as_int = (as_int << 8) | static_cast<std::uint64_t>(*first++);
      assert(first == last);

      return {bit_cast<double>(as_int)};
    }

    case buffer_string: case buffer_text: {
      std::string result;
      result.reserve(std::distance(first, last));
      std::transform(first, last, std::back_inserter(result),
                     [](std::byte b) { return static_cast<char>(b); });

      return {std::move(result)};
    }

    default:
      return {};
  }
#undef DOUBLE
}

void write_to_buffer(enigma::BinaryBuffer *binbuff, const std::vector<std::byte> &data, std::size_t offset) {
  switch (binbuff->type) {
    case buffer_wrap:
      if (offset >= binbuff->GetSize()) {
        offset = offset % binbuff->GetSize();
      }

      if (data.size() + offset > binbuff->data.size()) {
        std::size_t extra = (data.size() + offset) - binbuff->data.size();
        std::copy(data.begin(), data.end() - extra, binbuff->data.begin() + offset);
        std::copy(data.end() - extra, data.end(), binbuff->data.begin());
      } else {
        std::copy(data.begin(), data.end(), binbuff->data.begin() + offset);
      }
      break;

    case buffer_grow:
      if (data.size() + offset > binbuff->data.size()) {
        binbuff->Resize(data.size() + offset);
      }
      std::copy(data.begin(), data.end(), binbuff->data.begin() + offset);
      break;

    case buffer_fixed:
    case buffer_fast: {
      std::size_t over = 0;
      if (offset >= binbuff->data.size()) {
        DEBUG_MESSAGE("write_to_buffer: Offset beyond end of fixed/fast buffer, aborting write", MESSAGE_TYPE::M_USER_ERROR);
        return;
      } else if (data.size() + offset > binbuff->data.size()) {
        DEBUG_MESSAGE("write_to_buffer: Data being read cannot fit into fixed/fast buffer, truncating", MESSAGE_TYPE::M_WARNING);
        over = (data.size() + offset) - binbuff->data.size();
      }
      std::copy(data.begin(), data.end() - over, binbuff->data.begin() + offset);
      break;
    }
  }
}

void write_to_file(std::ofstream &file, const std::vector<std::byte>::iterator bytes, std::size_t size) {
  // Hide the unsafe code inside a safe abstraction
  file.write(reinterpret_cast<const char *>(&*bytes), size);
}

std::vector<std::byte> read_from_file(std::ifstream &file, std::size_t size) {
  // Hide the unsafe code inside a safe abstraction
  std::vector<std::byte> result{};
  result.resize(size);
  file.read(reinterpret_cast<char *>(result.data()), size);
  return result;
}

void read_from_file(std::ifstream &file, std::vector<std::byte>::iterator bytes, std::size_t size) {
  // Hide the unsafe code inside a safe abstraction
  // Very easy buffer overflow bug here, caller has to ensure enough space is there
  file.read(reinterpret_cast<char *>(&*bytes), size);
}

buffer_t make_new_buffer(std::size_t size, buffer_type_t type, std::size_t alignment) {
  auto buffer = std::make_unique<enigma::BinaryBuffer>(std::vector<std::byte>(size), 0, alignment, type);
  if (std::size_t id = enigma::get_free_buffer(); id == enigma::buffers.size()) {
    return enigma::buffers.add(std::move(buffer));
  } else {
    enigma::buffers.assign(id, std::move(buffer));
    return id;
  }
}

buffer_t buffer_create(std::size_t size, buffer_type_t type, std::size_t alignment) {
  return make_new_buffer(size, type, alignment);
}

void buffer_delete(buffer_t buffer) {
  enigma::buffers.destroy(buffer);
}

bool buffer_exists(buffer_t buffer) {
  return (buffer >= 0 && static_cast<std::size_t>(buffer) < enigma::buffers.size() &&
          enigma::buffers[buffer].get() != nullptr);
}

void buffer_copy(buffer_t src_buffer, std::size_t src_offset, std::size_t size, buffer_t dest_buffer, std::size_t dest_offset) {
  GET_BUFFER(srcbuff, src_buffer);
  GET_BUFFER(dstbuff, dest_buffer);

  if (src_offset >= srcbuff->GetSize()) {
    if (srcbuff->type == buffer_wrap) {
      src_offset = src_offset % srcbuff->GetSize();
    } else {
      DEBUG_MESSAGE("buffer_copy: source offset greater than source size, aborting write", MESSAGE_TYPE::M_ERROR);
      return;
    }
  }

  if (dest_offset >= dstbuff->GetSize()) {
    if (dstbuff->type == buffer_wrap) {
      dest_offset = dest_offset % dstbuff->GetSize();
    } else {
      DEBUG_MESSAGE("buffer_copy: destination offset greater than destination size, aborting write", MESSAGE_TYPE::M_ERROR);
      return;
    }
  }

  std::size_t written_bytes = [&srcbuff, &src_offset, &size]() {
    if (srcbuff->type == buffer_wrap) {
      return size;
    } else {
      return std::min(srcbuff->GetSize(), size) - src_offset;
    }
  }();

  if (dstbuff->type == buffer_fixed || dstbuff->type == buffer_fast) {
    if (dest_offset + written_bytes > dstbuff->GetSize()) {
      DEBUG_MESSAGE("buffer_copy: bytes written out of range for fixed/fast buffer, truncating", MESSAGE_TYPE::M_WARNING);
      written_bytes = std::min(dstbuff->GetSize(), written_bytes) - dest_offset;
    }
  }

  switch (dstbuff->type) {
    case buffer_wrap: {
      if (srcbuff->type == buffer_wrap && src_offset + written_bytes >= srcbuff->GetSize()) {
        for (std::size_t i = 0; i < size; i++) {
          dstbuff->data[(i + dest_offset) % dstbuff->GetSize()] = srcbuff->data[(i + src_offset) % srcbuff->GetSize()];
        }
      } else {
        for (std::size_t i = 0; i < size; i++) {
          dstbuff->data[(i + dest_offset) % dstbuff->GetSize()] = srcbuff->data[i + src_offset];
        }
      }
      break;
    }

    case buffer_grow: {
      if (dest_offset + written_bytes >= dstbuff->GetSize()) {
        dstbuff->Resize(dest_offset + written_bytes + 1);
      }

      [[fallthrough]];
    }

    case buffer_fixed:
    case buffer_fast: {
      if (srcbuff->type == buffer_wrap && src_offset + written_bytes >= srcbuff->GetSize()) {
        for (std::size_t i = 0; i < size; i++) {
          dstbuff->data[i + dest_offset] = srcbuff->data[(i + src_offset) % srcbuff->GetSize()];
        }
      } else {
        for (std::size_t i = 0; i < size; i++) {
          dstbuff->data[i + dest_offset] = srcbuff->data[i + src_offset];
        }
      }
      break;
    }
  }
}

void buffer_save(buffer_t buffer, string filename) {
  GET_BUFFER(binbuff, buffer);
  std::ofstream myfile(filename.c_str());
  if (!myfile.is_open()) {
    DEBUG_MESSAGE("Unable to open file " + filename, MESSAGE_TYPE::M_ERROR);
    return;
  }

  write_to_file(myfile, binbuff->data.begin(), binbuff->GetSize());
  myfile.close();
}

void buffer_save_ext(buffer_t buffer, string filename, std::size_t offset, std::size_t size) {
  GET_BUFFER(binbuff, buffer);
  std::ofstream myfile(filename.c_str());

  // NOTE: There is an incompatibility with GMS here, in terms of two things:
  // - GMS only seems to write the bytes which were actually written to the buffer, and not the entirety of the buffer
  //   itself, i.e. if you wrote 2 bytes to the buffer and saved it, the resulting file would be only 2 bytes long
  // - When @c offset is greater than @c size, GMS seems to write the last written byte to the file and then stops, which
  //   I really do not understand.

  if (!myfile.is_open()) {
    DEBUG_MESSAGE("Unable to open file " + filename, MESSAGE_TYPE::M_ERROR);
    return;
  }

  if (offset >= binbuff->GetSize()) {
    if (binbuff->type == buffer_wrap) {
      offset = offset % binbuff->GetSize();
    } else {
      DEBUG_MESSAGE("buffer_save_ext: offset beyond end of buffer, aborting write", MESSAGE_TYPE::M_ERROR);
      return;
    }
  }

  if (offset + size >= binbuff->GetSize()) {
    DEBUG_MESSAGE("buffer_save_ext: offset (" + std::to_string(offset) + ") + size (" + std::to_string(size) +
                  ") greater than buffer size (" + std::to_string(binbuff->GetSize()) + "), truncating to buffer end",
                  MESSAGE_TYPE::M_WARNING);
    size = std::min(binbuff->GetSize(), size) - offset;
  }

  write_to_file(myfile, binbuff->data.begin() + offset, size);
  myfile.close();
}

buffer_t buffer_load(string filename) {
  buffer_t id = make_new_buffer(0, buffer_grow, 1);
  enigma::BinaryBuffer *buffer = enigma::buffers.get(id).get();

  std::ifstream myfile(filename.c_str());
  if (!myfile.is_open()) {
    DEBUG_MESSAGE("Unable to open file " + filename, MESSAGE_TYPE::M_ERROR);
    return -1;
  }

  auto size = std::filesystem::file_size(filename);
  buffer->data.resize(size);
  read_from_file(myfile, buffer->data.begin(), size);
  myfile.close();

  return id;
}

void buffer_load_ext(buffer_t buffer, string filename, std::size_t offset) {
  GET_BUFFER(binbuff, buffer);

  std::ifstream myfile(filename.c_str());
  if (!myfile.is_open()) {
    DEBUG_MESSAGE("Unable to open file " + filename, MESSAGE_TYPE::M_ERROR);
    return;
  }

  std::vector<std::byte> data = read_from_file(myfile, std::filesystem::file_size(filename));
  write_to_buffer(binbuff, data, offset);

  myfile.close();
}

void buffer_fill(buffer_t buffer, std::size_t offset, buffer_data_t type, variant value, std::size_t size) {
  GET_BUFFER(binbuff, buffer);

  // NOTE: There is an incompatibility with GMS here: the game completely hangs if you try to fill a buffer which
  // doesn't have enough space for @c size bytes. To avoid this issue, I clamp @c size to the buffer end if it is
  // too large instead of emulating a hang.

  std::size_t orig_off = offset;
  while (offset % binbuff->alignment != 0 && offset < binbuff->GetSize()) {
    binbuff->WriteByte(std::byte{0});
    offset++;
  }

  if (offset + size > binbuff->GetSize()) {
    DEBUG_MESSAGE("buffer_fill: size too large; clamping to buffer end (max: " + std::to_string(binbuff->GetSize()) + ", got offset: " + std::to_string(orig_off) + " size: " + std::to_string(size) + ")", MESSAGE_TYPE::M_WARNING);
    size = binbuff->GetSize() - offset;
  }
  std::vector<std::byte> bytes = enigma_user::serialize_to_type(value, type);

  std::size_t padding = 0;
  if (bytes.size() % binbuff->alignment != 0) {
    padding = binbuff->alignment - (bytes.size() % binbuff->alignment);
  }
  std::vector<std::byte> padding_bytes{};
  padding_bytes.resize(padding, std::byte{0});

  std::size_t element_size = bytes.size() + padding;
  std::size_t times = size / element_size;
  std::size_t remainder = size % element_size;

  for (std::size_t i = 0; i < times; i++) {
    std::copy(bytes.begin(), bytes.end(),
              binbuff->data.begin() + offset + i * element_size);
    std::copy(padding_bytes.begin(), padding_bytes.end(),
              binbuff->data.begin() + offset + i * element_size + bytes.size());
  }
  if (remainder != 0 && remainder >= bytes.size()) {
    std::copy(bytes.begin(), bytes.end(),
              binbuff->data.begin() + offset + times * element_size);
    if (remainder - bytes.size() >= padding_bytes.size()) {
      std::copy(bytes.begin(), bytes.end(),
                binbuff->data.begin() + offset + times * element_size + bytes.size());
    }
  }
  binbuff->Seek(0);
}
  
void *buffer_get_address(buffer_t buffer) {
  #ifdef DEBUG_MODE
  if (buffer < 0 or size_t(buffer) >= enigma::buffers.size() or enigma::buffers[buffer].get() == nullptr) {
    DEBUG_MESSAGE("Attempting to access non-existing buffer " + toString(buffer), MESSAGE_TYPE::M_USER_ERROR);
    return nullptr;
  }
  #endif
  enigma::BinaryBuffer *binbuff = enigma::buffers[buffer].get();
  return reinterpret_cast<void *>(binbuff->data.data());
}

std::size_t buffer_get_size(buffer_t buffer) {
  GET_BUFFER_R(binbuff, buffer, -1);
  return binbuff->GetSize();
}

std::size_t buffer_get_alignment(buffer_t buffer) {
  GET_BUFFER_R(binbuff, buffer, -1);
  return binbuff->alignment;
}

buffer_type_t buffer_get_type(buffer_t buffer) {
  GET_BUFFER_R(binbuff, buffer, static_cast<buffer_type_t>(-1));
  return binbuff->type;
}

void buffer_get_surface(buffer_t buffer, int surface, int mode, std::size_t offset, int modulo) {
  //GET_BUFFER(binbuff, buffer);
  //TODO: Write this function
  DEBUG_MESSAGE("Function unimplemented: buffer_get_surface", MESSAGE_TYPE::M_WARNING);
}

void buffer_set_surface(buffer_t buffer, int surface, int mode, std::size_t offset, int modulo) {
  int tex = surface_get_texture(surface);
  int wid = surface_get_width(surface);
  int hgt = surface_get_height(surface);
  if (buffer_get_size(buffer) == buffer_sizeof(buffer_u64) * wid * hgt) {
    enigma::graphics_push_texture_pixels(tex, wid, hgt, (unsigned char *)buffer_get_address(buffer));
  } else { // execution can not continue safely with wrong buffer size
    DEBUG_MESSAGE("Buffer allocated with wrong length!", MESSAGE_TYPE::M_WARNING);
  }
}

void buffer_resize(buffer_t buffer, std::size_t size) {
  GET_BUFFER(binbuff, buffer);
  binbuff->Resize(size);
}

void buffer_seek(buffer_t buffer, buffer_seek_t base, long long offset) {
  GET_BUFFER(binbuff, buffer);
  switch (base) {
    case buffer_seek_start:
      binbuff->Seek(offset);
      break;
    case buffer_seek_end:
      binbuff->Seek(binbuff->GetSize() - 1 + offset);
      break;
    case buffer_seek_relative:
      binbuff->Seek(binbuff->position + offset);
      break;
  }
}

std::size_t buffer_sizeof(buffer_data_t type) {
  switch (type) {
    case buffer_u8: case buffer_s8: case buffer_bool:
      return 1;
    case buffer_u16: case buffer_s16: case buffer_f16:
      return 2;
    case buffer_u32: case buffer_s32: case buffer_f32:
      return 4;
    case buffer_u64: case buffer_f64:
      return 8;
    case buffer_string: case buffer_text: default:
      break;
  }
  return 0;
}

std::size_t buffer_tell(buffer_t buffer) {
  GET_BUFFER_R(binbuff, buffer, -1);
  return binbuff->position;
}

variant buffer_peek(buffer_t buffer, std::size_t offset, buffer_data_t type) {
  GET_BUFFER_R(binbuff, buffer, -1);
  if (type != buffer_string && type != buffer_text) {
    //std::size_t dsize = buffer_sizeof(type) + binbuff->alignment - 1;
    //NOTE: These buffers most likely need a little more code added to take care of endianess on different architectures.
    //TODO: Fix floating point precision.
    variant value = deserialize_from_type(binbuff->data.begin() + offset, binbuff->data.begin() + offset + buffer_sizeof(type), type);
    return value;
  } else {
    std::string data;
    for (std::size_t pos = offset; pos < binbuff->GetSize() && binbuff->data[pos] != std::byte{0};) {
      data.push_back(static_cast<char>(binbuff->data[pos]));
      if (binbuff->type == buffer_wrap) {
        pos = (pos + 1) % binbuff->GetSize();
      } else {
        pos++;
      }
    }

    return {data};
  }
}

variant buffer_read(buffer_t buffer, buffer_data_t type) {
  GET_BUFFER_R(binbuff, buffer, -1);
  while (binbuff->position % binbuff->alignment != 0) {
    if (binbuff->ReadByte() != std::byte{0}) {
      DEBUG_MESSAGE("buffer_peek: internal error: buffer not padded with zeroes, probably read something incorrect", MESSAGE_TYPE::M_FATAL_ERROR);
    }
  }

  auto result = buffer_peek(buffer, binbuff->position, type);

  if (type != buffer_string && type != buffer_string) {
    binbuff->Seek(binbuff->position + buffer_sizeof(type));
  } else {
    while (binbuff->position < binbuff->GetSize() && binbuff->data[binbuff->position] != std::byte{0}) {
      binbuff->ReadByte(); // read the string, because we do not know its length
    }
    binbuff->ReadByte(); // skip the null terminator
  }
  return result;
}

void buffer_poke(buffer_t buffer, std::size_t offset, buffer_data_t type, variant value, bool resize) {
  GET_BUFFER(binbuff, buffer);

  // NOTE: there is a GMS incompatibility here; in GMS if the data cannot fit within the current size of the buffer, the
  // write is simply aborted. Here, if the buffer is not large enough, there are 2 cases handled:
  // - buffer_grow: Expand the buffer
  // - buffer_fixed/buffer_fast: Truncate the data being written

  std::vector<std::byte> data = serialize_to_type(value, type);

  if ((data.size() + offset) > binbuff->GetSize() && ((!resize && binbuff->type == buffer_grow) ||
      binbuff->type == buffer_fixed || binbuff->type == buffer_fast)) {
    DEBUG_MESSAGE("buffer_poke: Write would go off end of buffer, aborting", MESSAGE_TYPE::M_ERROR);
    return;
  }

  write_to_buffer(binbuff, data, offset);
}

void buffer_write(buffer_t buffer, buffer_data_t type, variant value) {
  GET_BUFFER(binbuff, buffer);
  while (binbuff->position % binbuff->alignment != 0) {
    binbuff->WriteByte(std::byte{0});
  }

  std::vector<std::byte> data = serialize_to_type(value, type);
  write_to_buffer(binbuff, data, binbuff->position);

  if (type != buffer_string && type != buffer_string) {
    binbuff->Seek(binbuff->position + buffer_sizeof(type));
  } else {
    while (binbuff->position < binbuff->GetSize() && binbuff->data[binbuff->position] != std::byte{0}) {
      binbuff->ReadByte();  // read the string, because we do not know its length
    }
    binbuff->ReadByte();  // skip the null terminator
  }
}

string buffer_md5(buffer_t buffer, std::size_t offset, std::size_t size) {
  //GET_BUFFER_R(binbuff, buffer, 0);
  //TODO: Write this function
  return NULL;
}

string buffer_sha1(buffer_t buffer, std::size_t offset, std::size_t size) {
  //GET_BUFFER_R(binbuff, buffer, 0);
  //TODO: Write this function
  return NULL;
}

buffer_t buffer_base64_decode(string str) {
//  enigma::BinaryBuffer* buffer = new enigma::BinaryBuffer(0);
//  buffer->type = buffer_grow;
//  buffer->alignment = 1;
//  int id = enigma::get_free_buffer();
//  enigma::buffers.insert(enigma::buffers.begin() + id, buffer);
  //TODO: Write this function
//  return id;
}

void buffer_base64_decode_ext(buffer_t buffer, string str, std::size_t offset) {
  //GET_BUFFER_R(binbuff, buffer, -1);
  //TODO: Write this function
}

string buffer_base64_encode(buffer_t buffer, std::size_t offset, std::size_t size) {
  //GET_BUFFER_R(binbuff, buffer, 0);
  //TODO: Write this function
  return NULL;
}

void game_save_buffer(buffer_t buffer) {
  //GET_BUFFER(binbuff, buffer);
  //TODO: Write this function
}

void game_load_buffer(buffer_t buffer) {
  //GET_BUFFER(binbuff, buffer);
  //TODO: Write this function
}

}  // namespace enigma_user
