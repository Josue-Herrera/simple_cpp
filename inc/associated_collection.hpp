

#include <vector> 
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include "container_algo.hpp"

// Maps types to void
template<class...>
using void_t = void;

// Primary Template catches any type
// without a nested ::key_type memeber
template<class, class = void>
constexpr bool is_associatable = false;

//  Catches V with a nested ::key_type
template<class T>
constexpr bool is_associatable < T, void_t<decltype( T::key )> > = true;

template<
         class Element,
         class Foreign_key  = decltype( Element::key ),
         class Key          = decltype( Element::key ),
         class Contributors = Foreign_key
        >
class associated_collection {

    static_assert(is_associatable<Element>, "Element must contain a key member field.");

    template<class... Ts>
    using set = boost::container::flat_set<Ts...>;

    template<class... Ts>
    using map = boost::container::flat_multimap<Ts...>;

public: 
    using key_type                = Key;
    using foreign_key_type        = Foreign_key;
    using value_type              = Element;
    using reference               = Element&;
    using const_reference         = Element const&;

    using collection_type         = set<Element>;
    using key_collection_type     = set<key_type>;
    using foreign_collection_type = set<std::pair<foreign_key_type, key_type>>;
    using inverse_foreign_type    = set<std::pair<key_type, foreign_key_type>>;

    using association_type        = map<key_type, foreign_key_type>;
    using contributors_type       = map<key_type, Contributors>;

    using size_type               = typename collection_type::size_type;
    using iterator                = typename collection_type::iterator;
    using const_iterator          = typename collection_type::const_iterator;

    using aiterator               = typename association_type::iterator;
    using const_aiterator         = typename association_type::const_iterator;
    using avalue_type             = typename association_type::value_type;
    using areference              = avalue_type&;
    using aconst_reference        = avalue_type const&;


private:

    collection_type         collection_;
    association_type        associations_;
    contributors_type       contributors_;

    key_collection_type     keys_;
    foreign_collection_type foreign_keys_;
   
public:

    contributors_type       const& contributors() const noexcept { return contributors_; }
    association_type        const& associations() const noexcept { return associations_; }
    key_collection_type     const& keys()         const noexcept { return keys_; }
    foreign_collection_type const& foreign_keys() const noexcept { return foreign_keys_; }
    key_collection_type     &      borrow_keys()        noexcept { return keys_; }
    size_type                      size()         const noexcept { return collection_.size(); }
    const_iterator                 begin()        const noexcept { return collection_.cbegin(); }    
    const_iterator                 end()          const noexcept { return collection_.cend();   }
    iterator                       begin()              noexcept { return std::begin(collection_);     }    
    iterator                       end()                noexcept { return std::end(collection_);       }

    void reserve(std::size_t new_capacity) noexcept {
        collection_.reserve(new_capacity);
        associations_.reserve(new_capacity);
        contributors_.reserve(new_capacity);
        keys_.reserve(new_capacity);
        foreign_keys_.reserve(new_capacity);
    }

    template<class... Args>
    void emplace(Args&&... values) noexcept {
        collection_.emplace(std::forward<Args>(values)...);
    }

    template<class... Args>
    void emplace_association(Args&&... values) noexcept {
        auto location = associations_.emplace(std::forward<Args>(values)...);
        auto range    = associations_.equal_range(location->first);
        std::sort(range.first, range.second);
    }
    
    auto erase(key_type const& k) noexcept -> foreign_collection_type& {
       return erase_if([&](const_reference e) { return e == k; });
    }

    auto compare_associations(key_type const& a, key_type const& b) noexcept -> bool {
        const auto a_range = associations_.equal_range(a);
        const auto b_range = associations_.equal_range(b);
        return calgo::equal_values(a_range, b_range);
    }

    auto erase(inverse_foreign_type const& set_of_assocations) noexcept -> foreign_collection_type& {
        calgo::erase_if(associations_, [&](avalue_type value){
            if (calgo::contains(set_of_assocations, value)) {
                if (calgo::equals_one(associations_.count(value.first))) {
                    calgo::erase_value(collection_, value.first);
                    contributors_.erase(value.first);
                }
                return true;
            }
            return false;
        });

        calgo::sort(collection_, [&](const_reference a, const_reference b){
            if(compare_associations(a.key, b.key)) return true;
            else return calgo::value(a.key) < calgo::value(b.key);
        });
        
        calgo::erase_duplicates_if(collection_, [&](const_reference a, const_reference b){
            bool equal = compare_associations(a.key, b.key);
            if (equal) {
                auto range = inverse_assocations(b.key);
                associations_.erase(range.first, range.second);
                contributors_.erase(b.key);
            }
            return equal;
        });
        return foreign_keys_;
    }

    template<class Foreign, class UnaryOperation>
    auto visit (Foreign const& foreign_key, UnaryOperation operation) noexcept -> void {
        static_assert(std::is_same_v<Foreign, foreign_key_type>, "Visited Key must be the foreign key.\n");
        calgo::set_intersection(collection_, associations_, operation, calgo::overload{
            [&](reference a, areference b){ 
                if (calgo::value(b) == foreign_key and calgo::key(b) == a){  
                    return false; 
                } else {
                    return a < calgo::key(b);
                }
            },
            [&](areference b, reference a){ 
                if (calgo::value(b) == foreign_key and calgo::key(b) == a){  
                    return false; 
                } else {
                    return not (a < calgo::key(b));
                }
            }
        });
    }

private:

    auto inverse_assocations(key_type key) noexcept {
        auto range = associations_.equal_range(key);
        calgo::transform (
            calgo::iterable(range), 
            calgo::back_inserter(foreign_keys_), 
            [](auto r){ return calgo::reverse(r); }
        );
        return range;
    }

    template<class Predicate>
    auto erase_if (Predicate predicate) noexcept -> foreign_collection_type& {
       calgo::erase_if(collection_,
        [&](const_reference element) {
            if (predicate(element)) {
                auto range = inverse_assocations(element.key);
                associations_.erase(range.first, range.second);
                contributors_.erase(element.key);
                return true;
            }
            return false;
        });
        return foreign_keys_;
    }
};

template<class AC1, class AC2>
void emplace_associations(AC1& ac1, AC2& ac2, typename AC1::key_type const& k1, typename AC2::key_type const& k2){
    ac1.emplace_association(k1,k2);
    ac2.emplace_association(k2,k1);
}

