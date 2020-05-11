// Bounded hash-map that using LRU eviction

#include <list>
#include <memory>
#include <mutex>
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

    template <typename KeyT, typename ValT>
    class BoundedHashMap {
      typedef typename std::unordered_map<KeyT, ValT>::size_type size_type;

      const size_type max_size_val;
      std::list<KeyT> mru;
      std::unordered_map<KeyT, BoundedHashMapVal<KeyT, ValT>> map;
      std::shared_ptr<std::mutex> m; // consider shared_mutex to optimise attempted reads when key is not present
                                     // shared_ptr is a ludicrous way to allow copying of BoundedHashMap's, e.g. in vector::push_back

    public:
      BoundedHashMap(std::size_t max_size) :
	max_size_val(max_size),	map(max_size), m(new std::mutex) {}

    private:
      bool remove_lru_entry_locked() {
	if(mru.empty()) {
	  return false; // already empty
	}

	const KeyT& lru_key = mru.back();
	return remove_locked(lru_key); // removed the lru element - this MUST succeed
      }

      void mru_move_to_front_locked(typename std::list<KeyT>::iterator& mru_it) {
	mru.splice(mru.begin(), mru, mru_it);
      }

      bool remove_locked(const KeyT& key) noexcept {
	auto map_it = map.find(key);
	if(map_it == map.end()) {
	  return false; // not present
	}

	mru.erase(map_it->second.mru_it);
	map.erase(map_it);

	return true; // was present
      }
      
    public:
      bool remove(const KeyT& key) noexcept {
	std::unique_lock<std::mutex> lock(*m);

	return remove_locked(key);
      }
    
      bool copy_if_present(const KeyT& key, ValT& to) noexcept {
	std::unique_lock<std::mutex> lock(*m);
	
	auto map_it = map.find(key);
	if(map_it == map.end()) {
	  return false; // not present
	}
	
	// move-to-front of mru
	mru_move_to_front_locked(map_it->second.mru_it);
	  
	// Key already present - copy the val
	to = map_it->second.val;

	return true; // found
      }

      bool put(const KeyT& key, const ValT& val) noexcept {
	std::unique_lock<std::mutex> lock(*m);
	
	auto map_it = map.find(key);
	if(map_it != map.end()) {
	  // move-to-front of mru
	  mru_move_to_front_locked(map_it->second.mru_it);
	  
	  // Key already present - just update the val
	  map_it->second.val = val;

	  return true; // already present
	}

	// Make space for the new entry
	if(max_size_val <= map.size()) {
	  remove_lru_entry_locked();
	}

	// Add the new entry
	mru.push_front(key);
	map.insert(make_pair(key, BoundedHashMapVal<KeyT, ValT>(mru.begin(), val)));

	return false; // not already present
      }

    };
    
  } // name BoundedHashMap

} // namespace Chess
