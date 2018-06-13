#include <memory>
#include <stdexcept>
#include <cstring>
#include <string>
#include <assert.h>
#include <Util/Bytes.h>
#include <Util/ArrayUtil.h>

using namespace lucene::core::util;

/*
 * BytesRef
 */

std::shared_ptr<char> BytesRef::DEFAULT_BYTES = std::shared_ptr<char>(new char[1](), std::default_delete<char[]>());

BytesRef::BytesRef()
  : bytes(BytesRef::DEFAULT_BYTES),
    offset(0),
    length(0),
    capacity(1) {
}

BytesRef::BytesRef(const BytesRef& source)
  : BytesRef(source.bytes.get(), source.offset, source.length, source.capacity) {
}

BytesRef::BytesRef(const char* new_bytes, const uint32_t new_offset, const uint32_t new_length, const uint32_t new_capacity) {
  offset = new_offset;
  length = new_length;
  capacity = new_capacity;

  char* new_byte_arr = arrayutil::CopyOfRange(new_bytes, offset, offset + length);
  bytes.reset(new_byte_arr);

  assert(IsValid());
}

BytesRef::BytesRef(const char* new_bytes, const uint32_t new_capacity)
  : BytesRef(new_bytes, 0, new_capacity, new_capacity) {
}

BytesRef::BytesRef(const uint32_t new_capacity)
  : bytes(std::shared_ptr<char>(new char[new_capacity], std::default_delete<char[]>())),
    offset(0),
    length(new_capacity),
    capacity(new_capacity) {
  assert(IsValid());
}

BytesRef::BytesRef(const std::string& text) {
  if(text.empty()) {
    bytes = BytesRef::DEFAULT_BYTES;
    offset = length = 0;
    capacity = 1;
  } else {
    const char* cstr = text.c_str();
    offset = 0;
    capacity = length = text.size();
    char* bytes_ptr = new char[capacity];
    std::memcpy(bytes_ptr, cstr, capacity);
    bytes.reset(bytes_ptr);
  }
}

BytesRef::~BytesRef() {
}

void BytesRef::ShallowCopyTo(BytesRef& target) {
  target.bytes = bytes;
  target.offset = 0;
  target.length = length;
  target.capacity = length;
}

int BytesRef::CompareTo(const BytesRef& other) const {
  if(IsValid() && other.IsValid()) {
    if(bytes == other.bytes && offset == other.offset && length == other.length) {
      return 0;
    }

    uint32_t my_len = length - offset;
    uint32_t his_len = other.length - other.offset;
    uint32_t len = (my_len < his_len ? my_len : his_len);
    char* my_bytes_ptr = bytes.get();
    char* his_bytes_ptr = other.bytes.get();

    for(uint32_t i = 0 ; i < len ; ++i) {
      char mine = my_bytes_ptr[i];
      char his = his_bytes_ptr[i];
      char diff = (mine - his);
      if(diff != 0) {
        return diff;
      }
    }

    // One is a prefix of another, or, they are equal.
    return (my_len - his_len);
  }

  throw std::runtime_error("This BytesRef is not valid or other BytesRef is not valid during CompareTo");
}

BytesRef& BytesRef::operator=(const BytesRef& source) {
  if(this != &source) {
    char* new_byte_arr = arrayutil::CopyOfRange(source.bytes.get(), offset, offset + length);
    bytes.reset(new_byte_arr);
    offset = source.offset;
    length = source.length;
    capacity = source.capacity;
  }

  return *this;
}

bool BytesRef::operator==(const BytesRef& other) const {
  return CompareTo(other) == 0;
}

bool BytesRef::operator!=(const BytesRef& other) const {
  return !operator==(other);
}

bool BytesRef::operator<(const BytesRef& other) const {
  return CompareTo(other) < 0;
}

bool BytesRef::operator<=(const BytesRef& other) const {
  return CompareTo(other) <= 0;
}

bool BytesRef::operator>(const BytesRef& other) const {
  return CompareTo(other) > 0;
}

bool BytesRef::operator>=(const BytesRef& other) const {
  return CompareTo(other) >= 0;
}

std::string BytesRef::UTF8ToString() {
  return std::string(bytes.get(), offset, length);
}

bool BytesRef::IsValid() const {
  // In C++ BytesRef allows null bytes
  if(!bytes.get() && (offset != 0 || length != 0) && capacity > 1) {
    throw std::runtime_error("bytes is nullptr, offset=" + std::to_string(offset) + ", length=" + std::to_string(length) + ", capacity=" + std::to_string(capacity));
  }

  if(offset > length) {
    throw std::overflow_error("Offset out of bounds: " + std::to_string(offset) + ", length=" + std::to_string(length));
  }

  return true;
}


/**
 * BytesRefBuilder
 */
BytesRefBuilder::BytesRefBuilder()
  : ref() {
}

const char* BytesRefBuilder::Bytes() const {
  return ref.bytes.get();
}

const uint32_t BytesRefBuilder::Length() const {
  return ref.length;
}

void BytesRefBuilder::SetLength(const uint32_t new_length) {
  ref.length = new_length;
}

char& BytesRefBuilder::operator[](const uint32_t idx) {
  return ref.bytes.get()[idx];
}

void BytesRefBuilder::Grow(uint32_t new_capacity) {
  std::pair<char*, uint32_t> new_bytes_pair = arrayutil::Grow(ref.bytes.get(), ref.capacity, new_capacity);
  if(new_bytes_pair.first) {
    ref.bytes.reset(new_bytes_pair.first);
    ref.capacity = new_bytes_pair.second;
  }
}

void BytesRefBuilder::Append(const char c) {
  Grow(ref.length + 1);
  char* byte_arr = ref.bytes.get();
  byte_arr[ref.length++] = c;
}

void BytesRefBuilder::Append(const char* new_bytes, const uint32_t off, const uint32_t len) {
  Grow(ref.length + len);
  std::memcpy(ref.bytes.get() + ref.length, new_bytes + off, len);
  ref.length += len;
}

void BytesRefBuilder::Append(BytesRef& ref) {
  Append(ref.bytes.get(), ref.offset, ref.length);
}

void BytesRefBuilder::Append(BytesRefBuilder& builder) {
  Append(builder.Get());
}

void BytesRefBuilder::Clear() {
  SetLength(0);
}

void BytesRefBuilder::CopyBytes(const char* new_bytes, const uint32_t off, const uint32_t len) {
  Clear();
  Append(new_bytes, off, len);
}

void BytesRefBuilder::CopyBytes(BytesRef& ref) {
  Clear();
  Append(ref);
}

void BytesRefBuilder::CopyBytes(BytesRefBuilder& builder) {
  Clear();
  Append(builder);
}

void BytesRefBuilder::CopyChars(std::string& text) {
  CopyChars(text, 0, text.size());
}

void BytesRefBuilder::CopyChars(std::string& text, const uint32_t off, const uint32_t len) {
  Grow(len);
  std::memcpy(ref.bytes.get(), text.c_str() + off, len);
  ref.length = len;
}

BytesRef& BytesRefBuilder::Get() {
  return ref;
}

BytesRef BytesRefBuilder::ToBytesRef() const {
  return BytesRef(ref);
}
