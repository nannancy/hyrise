#pragma once

#include "storage/vector_compression/base_vector_decompressor.hpp"

#include "types.hpp"

namespace opossum {

template <typename UnsignedIntType>
class BitMagicDecompressor : public BaseVectorDecompressor {
 public:
  explicit BitMagicDecompressor(const bm::sparse_vector<uint32_t, bm::bvector<>>& data) : _data{data} {}
  ~BitMagicDecompressor() final = default;

  uint32_t get(size_t i) final { return _data[i]; }
  size_t size() const final { return _data.size(); }

 private:
  const bm::sparse_vector<uint32_t, bm::bvector<>>& _data;
};

}  // namespace opossum
