#include "bit_magic_compressor.hpp"

namespace opossum {

std::unique_ptr<const BaseCompressedVector> BitMagicCompressor::compress(
    const pmr_vector<uint32_t>& vector, const PolymorphicAllocator<size_t>& alloc,
    const UncompressedVectorInfo& meta_info) {
  const auto max_value = meta_info.max_value ? *meta_info.max_value : _find_max_value(vector);
  return _compress_using_max_value(alloc, vector, max_value);
}

std::unique_ptr<BaseVectorCompressor> BitMagicCompressor::create_new() const {
  return std::make_unique<BitMagicCompressor>();
}

uint32_t BitMagicCompressor::_find_max_value(const pmr_vector<uint32_t>& vector) {
  const auto it = std::max_element(vector.cbegin(), vector.cend());
  return *it;
}

std::unique_ptr<BaseCompressedVector> BitMagicCompressor::_compress_using_max_value(  // TODO dedup
    const PolymorphicAllocator<size_t>& alloc, const pmr_vector<uint32_t>& vector, const uint32_t max_value) {
  auto bm_vector = bm::sparse_vector<uint32_t, bm::bvector<> >{};  // TODO use allocator
  bm_vector.import(&vector[0], vector.size());

  {
    BM_DECLARE_TEMP_BLOCK(tb)  // TODO can we do without a macro here
    bm_vector.optimize(tb); // TODO play around with optimization levels

    // compute bit-vector statistics
    bm::bvector<>::statistics st;
    bm_vector.calc_stat(&st);

    std::cout << "Bit-vector statistics: GAP (compressed blocks)=" << st.gap_blocks
              << ", BIT (uncompressed blocks)=" << st.bit_blocks
              << std::endl << std::endl;
  }

  return std::make_unique<BitMagicVector>(std::move(bm_vector));
}

}  // namespace opossum
