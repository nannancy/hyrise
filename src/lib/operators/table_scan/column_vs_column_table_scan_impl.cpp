#include "column_vs_column_table_scan_impl.hpp"

#include <memory>
#include <string>
#include <type_traits>

#include "resolve_type.hpp"
#include "storage/chunk.hpp"
#include "storage/create_iterable_from_segment.hpp"
#include "storage/reference_segment/reference_segment_iterable.hpp"
#include "storage/segment_iterables/any_segment_iterable.hpp"
#include "storage/segment_iterate.hpp"
#include "storage/table.hpp"
#include "type_comparison.hpp"
#include "utils/assert.hpp"

namespace opossum {

ColumnVsColumnTableScanImpl::ColumnVsColumnTableScanImpl(const std::shared_ptr<const Table>& in_table,
                                                         const ColumnID left_column_id,
                                                         const PredicateCondition& predicate_condition,
                                                         const ColumnID right_column_id)
    : _in_table(in_table),
      _left_column_id(left_column_id),
      _predicate_condition(predicate_condition),
      _right_column_id{right_column_id} {}

std::string ColumnVsColumnTableScanImpl::description() const { return "ColumnVsColumn"; }

std::shared_ptr<PosList> ColumnVsColumnTableScanImpl::scan_chunk(ChunkID chunk_id) const {
  return nullptr;
}

template <EraseTypes erase_comparator_type, typename LeftIterable, typename RightIterable>
std::shared_ptr<PosList> __attribute__((noinline))
ColumnVsColumnTableScanImpl::_typed_scan_chunk_with_iterables(ChunkID chunk_id, const LeftIterable& left_iterable,
                                                              const RightIterable& right_iterable) const {
  auto matches_out = std::shared_ptr<PosList>{};

  left_iterable.with_iterators([&](auto left_it, const auto left_end) {
    right_iterable.with_iterators([&](auto right_it, const auto right_end) {
      matches_out =
          _typed_scan_chunk_with_iterators<erase_comparator_type>(chunk_id, left_it, left_end, right_it, right_end);
    });
  });

  return matches_out;
}

template <EraseTypes erase_comparator_type, typename LeftIterator, typename RightIterator>
std::shared_ptr<PosList> __attribute__((noinline))
ColumnVsColumnTableScanImpl::_typed_scan_chunk_with_iterators(ChunkID chunk_id, LeftIterator& left_it,
                                                              const LeftIterator& left_end, RightIterator& right_it,
                                                              const RightIterator& right_end) const {
  const auto chunk = _in_table->get_chunk(chunk_id);

  auto matches_out = std::make_shared<PosList>();

  using LeftType = typename LeftIterator::ValueType;
  using RightType = typename RightIterator::ValueType;

  // C++ cannot compare strings and non-strings out of the box:
  if constexpr (std::is_same_v<LeftType, pmr_string> == std::is_same_v<RightType, pmr_string>) {
    bool condition_was_flipped = false;
    auto maybe_flipped_condition = _predicate_condition;
    if (maybe_flipped_condition == PredicateCondition::GreaterThan ||
        maybe_flipped_condition == PredicateCondition::GreaterThanEquals) {
      maybe_flipped_condition = flip_predicate_condition(maybe_flipped_condition);
      condition_was_flipped = true;
    }

    auto conditionally_erase_comparator_type = [](auto comparator, const auto& it1, const auto& it2) {
      if constexpr (erase_comparator_type == EraseTypes::OnlyInDebugBuild) {
        return comparator;
      } else {
        return std::function<bool(const AbstractSegmentPosition<std::decay_t<decltype(it1->value())>>&,
                                  const AbstractSegmentPosition<std::decay_t<decltype(it2->value())>>&)>{comparator};
      }
    };

    with_comparator_light(maybe_flipped_condition, [&](auto predicate_comparator) {
      const auto comparator = [predicate_comparator](const auto& left, const auto& right) {
        return predicate_comparator(left.value(), right.value());
      };

      if (condition_was_flipped) {
        const auto erased_comparator = conditionally_erase_comparator_type(comparator, right_it, left_it);
        AbstractTableScanImpl::_scan_with_iterators<true>(erased_comparator, right_it, right_end, chunk_id,
                                                          *matches_out, left_it);
      } else {
        const auto erased_comparator = conditionally_erase_comparator_type(comparator, left_it, right_it);
        AbstractTableScanImpl::_scan_with_iterators<true>(erased_comparator, left_it, left_end, chunk_id, *matches_out,
                                                          right_it);
      }
    });
  } else {
    Fail("Trying to compare strings and non-strings");
  }

  return matches_out;
}

}  // namespace opossum
