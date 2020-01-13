#pragma once

#include <memory>

#include <bm.h> // not bm64
#include <bmsparsevec.h>

#include "storage/vector_compression/base_compressed_vector.hpp"
#include "types.hpp"

namespace opossum {

namespace hana = boost::hana;

/**
 * @brief Stores values as either uint32_t, uint16_t, or uint8_t
 *
 * This is simplest vector compression scheme. It matches the old FittedAttributeVector
 */
class BitMagicVector : public CompressedVector<BitMagicVector> {

 public:
  explicit BitMagicVector(bm::sparse_vector<uint32_t, bm::bvector<>> data) : _data{std::move(data)} {}
  ~BitMagicVector() = default;

 public:
  size_t on_size() const { return _data.size(); }
  size_t on_data_size() const { return sizeof(UnsignedIntType) * _data.size(); }

  auto on_create_base_decompressor() const { return std::unique_ptr<BaseVectorDecompressor>{on_create_decompressor()}; }

  auto on_create_decompressor() const {
    return std::make_unique<BitMagicDecompressor>(_data);
  }

  auto on_begin() const { return _data.cbegin(); }

  auto on_end() const { return _data.cend(); }

  std::unique_ptr<const BaseCompressedVector> on_copy_using_allocator(const PolymorphicAllocator<size_t>& alloc) const {
    auto data_copy = pmr_vector{_data, alloc};
    return std::make_unique<BitMagicVector>(std::move(data_copy));
  }

 private:
  const bm::sparse_vector<uint32_t, bm::bvector<>> _data;
};

}  // namespace opossum
