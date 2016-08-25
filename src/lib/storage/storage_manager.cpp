#include "storage_manager.hpp"

#include <memory>
#include <string>
#include <utility>

namespace opossum {

storage_manager &storage_manager::get() {
  static storage_manager instance;
  return instance;
}

void storage_manager::add_table(const std::string &name, std::shared_ptr<table> tp) {
  _tables.insert(std::make_pair(name, std::move(tp)));
}

std::shared_ptr<table> storage_manager::get_table(const std::string &name) const { return _tables.at(name); }

void storage_manager::print(std::ostream &out) const {
  out << "==================" << std::endl;
  out << "===== Tables =====" << std::endl << std::endl;

  auto cnt = 0;
  for (auto const &tab : _tables) {
    out << "==== table >> " << tab.first << " <<";
    out << " (" << tab.second->col_count() << " columns, " << tab.second->row_count() << " rows in "
        << tab.second->chunk_count() << " chunks)";
    out << std::endl << std::endl;
    tab.second->print();
    cnt++;
  }
}
}  // namespace opossum
