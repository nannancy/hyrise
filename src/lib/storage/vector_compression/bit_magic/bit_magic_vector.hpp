#pragma once

#include <memory>

#include <bm.h> // not bm64
#include <bmsparsevec.h>

#include "storage/vector_compression/base_compressed_vector.hpp"
#include "types.hpp"

namespace opossum {

namespace hana = boost::hana;

// TODO doc
class BitMagicVector : public CompressedVector<BitMagicVector> {

 public:
  explicit BitMagicVector(bm::sparse_vector<uint32_t, bm::bvector<>> data) : _data{std::move(data)} {}
  ~BitMagicVector() = default;

 public:
  size_t on_size() const { return _data.size(); }
  size_t on_data_size() const { return 0; }  // TODO

  auto on_create_base_decompressor() const { return std::unique_ptr<BaseVectorDecompressor>{on_create_decompressor()}; }

  auto on_create_decompressor() const {
    return std::make_unique<BitMagicDecompressor>(_data);
  }

  auto on_begin() const { return _data.begin(); }

  auto on_end() const { return _data.end(); }

  std::unique_ptr<const BaseCompressedVector> on_copy_using_allocator(const PolymorphicAllocator<size_t>& alloc) const {
    auto data_copy = pmr_vector{_data, alloc};
    return std::make_unique<BitMagicVector>(std::move(data_copy));
  }

 private:
  const bm::sparse_vector<uint32_t, bm::bvector<>> _data;
};

}  // namespace opossum
