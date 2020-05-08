// Bounded hash-map that using LRU eviction

#include <list>
#include <unordered_map>

namespace Chess {

  namespace BoundedHashMap {

    template <typename KeyT, typename ValT>
    struct BoundedHashMapVal {
      typename std::list<KeyT>::iterator mru_it;
      ValT val;

      BoundedHashMapVal(const typename std::list<KeyT>::iterator& mru_it, const ValT& val) :
	mru_it(mru_it), val(val) {}
    };

    template <typename KeyT, typename ValT, typename HashT = std::hash<KeyT>>
    struct BoundedHashMap {
      typedef typename std::unordered_map<KeyT, ValT, HashT>::size_type size_type;

      const size_type max_size_val;
      std::list<KeyT> mru;
      std::unordered_map<KeyT, BoundedHashMapVal<KeyT, ValT>> map;

      BoundedHashMap(std::size_t max_size) :
	max_size_val(max_size),
	map(max_size) {}

      bool empty() const noexcept { return map.empty(); }
      size_type size() const noexcept { return map.size(); }
      size_type max_size() const noexcept { return max_size_val; }
      
      // Should do all of this more like STL standard but that's a lot of work

      typename std::unordered_map<KeyT, BoundedHashMapVal<KeyT, ValT>>::iterator end() noexcept {
	return map.end();
      }
      
      typename std::unordered_map<KeyT, BoundedHashMapVal<KeyT, ValT>>::iterator find(const KeyT& key) noexcept {
	return map.find(key);
      }
      
      bool remove(const KeyT& key) noexcept {
	auto map_it = map.find(key);
	if(map_it == map.end()) {
	  return false; // not present
	}

	mru.erase(map_it->second.mru_it);
	map.erase(map_it);

	return true; // was present
      }
    
      bool remove_lru_entry() {
	if(mru.empty()) {
	  return false; // already empty
	}

	const KeyT& lru_key = mru.back();
	return remove(lru_key); // removed the lru element - this MUST succeed
      }

      void mru_move_to_front(typename std::list<KeyT>::iterator& mru_it) {
	mru.splice(mru.begin(), mru, mru_it);
      }

      bool contains(const KeyT& key) const {
	return map.find(key) != map.end();
      }

      ValT& at(const KeyT& key) {
	BoundedHashMapVal<KeyT, ValT>& elem = map.at(key);
	
	// move-to-front of mru
	mru_move_to_front(elem.mru_it);
			    
	return elem.val;
      }

      bool put(const KeyT& key, const ValT& val) noexcept {
	auto map_it = map.find(key);
	if(map_it != map.end()) {
	  // move-to-front of mru
	  mru_move_to_front(map_it->second.mru_it);
	  
	  // Key already present - just update the val
	  map_it->second.val = val;

	  return true; // already present
	}

	// Make space for the new entry
	if(max_size_val <= size()) {
	  remove_lru_entry();
	}

	// Add the new entry
	mru.push_front(key);
	map.insert(make_pair(key, BoundedHashMapVal<KeyT, ValT>(mru.begin(), val)));

	return false; // not already present
      }

    };
    
  } // name BoundedHashMap

} // namespace Chess
