// Copyright 2021 Ian Ruh
#ifndef INCLUDE_ORDERED_SET_H_
#define INCLUDE_ORDERED_SET_H_

#include <symengine/basic.h>
#include <symengine/symbol.h>
#include <symengine/expression.h>

#include <unordered_map>
#include <vector>

using SymEngine::RCP;
using SymEngine::Basic;
using SymEngine::Symbol;

namespace cppmpc {

class OrderedSet {
 private:
    // Contains the actual elmenets
    std::vector<RCP<const Symbol>> elements_vector;
    std::unordered_map<RCP<const Symbol>, size_t, SymEngine::RCPBasicHash,
                       SymEngine::RCPBasicKeyEq>
            elements_map;

 public:
    OrderedSet() {}
    // TODO(ianruh): Copy constructor
    // TODO(ianruh): Move constructor

    /**
     * @brief Append a symbol to the ordered set.
     *
     * @param el The symbol to append.
     */
    void append(RCP<const Symbol> el) { this->insert(this->size(), el); }

    /**
     * @brief Uses the append for `RCP<const Symbol>`, but makes this nicer to
     * use with expressions.
     *
     * @param exp The symbol to append.
     */
    void append(const SymEngine::Expression& exp) { this->insert(this->size(), exp); }

    /**
     * @brief Insert symbol into the ordered set at the specified index.
     *
     * @param index The index to insert the symbol at.
     * @param el The symbol to insert.
     */
    void insert(size_t index, RCP<const Symbol> el) {
        if (this->elements_map.count(el) > 0) {
            return;
        }

        if (index == this->elements_vector.size()) {
            this->elements_vector.insert(this->elements_vector.end(), el);
        } else {
            this->elements_vector.insert(this->elements_vector.begin() + index,
                                         el);
        }

        this->elements_map.insert({el, index});

        for (size_t i = index; i < this->elements_vector.size(); i++) {
            this->elements_map.at(this->elements_vector.at(i)) = i;
        }
    }

    /**
     * @brief Convenience function to insert a symbol expression into the
     * ordered set.
     *
     * @param index The index to insert the symbol at.
     * @param exp The symbol to insert.
     */
    void insert(size_t index, const SymEngine::Expression& exp) {
        RCP<const Basic> basic = exp.get_basic();

        if(SymEngine::is_a<SymEngine::Symbol>(*basic)) {
            this->insert(index, SymEngine::rcp_dynamic_cast<const SymEngine::Symbol>(basic));
        } else {
            throw std::runtime_error("Only symbols can be in ordered set.");
        }
    }

    // We need to remove the element from the vector, and then
    // update the indexes of every element after in the map.
    void remove(size_t index) {
        this->elements_map.erase(this->elements_vector.at(index));
        this->elements_vector.erase(this->elements_vector.begin() + index);
        for (size_t i = index; i < this->elements_vector.size(); i++) {
            this->elements_map.at(this->elements_vector.at(i)) = i;
        }
    }

    /**
     * @brief Get the symbol at the given index.
     */
    RCP<const Symbol> at(size_t index) const {
        return this->elements_vector.at(index);
    }

    /**
     * @brief The number of symbols in the set.
     */
    size_t size() const { return this->elements_vector.size(); }

    /**
     * @brief Whether the set contains the given symbol. Is O(1).
     *
     * @param el The element to test.
     * @return True if in the set, false if not.
     */
    bool contains(const RCP<const Symbol>& el) const {
        return this->elements_map.count(el) > 0;
    }

    // Determine if the set other is a subset of this.
    bool isSubset(const OrderedSet& other) const {
        for (size_t i = 0; i < other.size(); i++) {
            if (!(this->elements_map.count(other.at(i)) > 0)) {
                return false;
            }
        }
        return true;
    }

    void unionWith(const OrderedSet& other) {
        for (size_t i = 0; i < other.size(); i++) {
            this->append(other.at(i));
        }
    }

    // Utility method used in testing to verify that the data
    // structures are self consistent with each other.
    bool isConsistent() const {
        bool goodSoFar = true;

        goodSoFar = goodSoFar &&
                    (this->elements_vector.size() == this->elements_map.size());

        for (size_t i = 0; i < this->elements_vector.size(); i++) {
            auto el = this->elements_vector.at(i);
            goodSoFar = goodSoFar && this->elements_map.at(el) == i;
        }

        return goodSoFar;
    }
};

}  // namespace cppmpc

#endif  // INCLUDE_ORDERED_SET_H_
