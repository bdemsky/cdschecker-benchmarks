#ifndef _CLIFFC_HASHTABLE_H
#define _CLIFFC_HASHTABLE_H

#include <iostream>
#include <atomic>
#include <memory>
#include <assert.h>

using namespace std;

/**
	This header file declares and defines a simplified version of Cliff Click's
	NonblockingHashMap. It contains all the necessary structrues and main
	functions. In simplified_cliffc_hashtable.cc file, it has the definition for
	the static fields.
*/

template<typename TypeK, typename TypeV, class Hash, class KeyEqualsTo, class ValEqualsTo>
class cliffc_hashtable;

/**
	Corresponding the the Object[] array in Cliff Click's Java implementation.
	It keeps the first two slots for CHM (Hashtable control unit) and the hash
	records (an array of hash used for fast negative key-equality check).
*/
struct kvs_data {
	int _size;
	atomic<void*> *_data;
	
	kvs_data(int sz) {
		_size = sz;
		int real_size = sizeof(atomic<void*>) * 2 + 2;
		_data = new atomic<void*>[real_size];
		// The control block should be initialized in resize()
		// Init the hash record array
		int *hashes = new int[_size];
		int i;
		for (i = 0; i < _size; i++) {
			hashes[i] = 0;
		}
		
		// Initialize the array
		_data[0].store(NULL, memory_order_relaxed);
		_data[1].store(hashes, memory_order_relaxed);
		// Init the data to Null slot
		for (i = 2; i < real_size; i++) {
			_data[i].store(NULL, memory_order_relaxed);
		}
	}

	~kvs_data() {
		int *hashes = (int*) _data[1].load(memory_order_relaxed);
		delete hashes;
		delete[] _data;
	}
};

struct slot {
	bool _prime;
	void *_ptr;

	slot(bool prime, void *ptr) {
		_prime = prime;
		_ptr = ptr;
	}

};


/**
	TypeK and TypeV should define their own copy constructor.
*/
template<typename TypeK, typename TypeV, class Hash, class KeyEqualsTo, class ValEqualsTo>
class cliffc_hashtable {
	/**
		# The synchronization we have for the hashtable gives us the property of
		# serializability, so we should have a sequential hashtable when we check the
		# correctness. The key thing is to identify all the commit point.
	
		@Begin
		@Global_define:

			spec_hashtable<TypeK, TypeV*> __map;
			spec_hashtable<TypeK, Tag> __id_map;
			Tag __tag;

			static bool _val_equals(TypeV *ptr1, TypeV *ptr2) {
				// ...
			}
			
			# Update the tag for the current key slot if the corresponding tag
			# is NULL, otherwise just return that tag. It will update the next
			# available tag too if it requires a new tag for that key slot.
			static Tag _getKeyTag(TypeK &key) {
				if (__id_map.get(key) == NULL) {
					Tag cur_tag = tag.current();
					__id_map.put(key, cur_tag);
					__tag.next();
					return cur_tag;
				} else {
					return __id_map.get(key);
				}
			}
		
		@Interface_cluster:
			Read_interface = {
				Get,
				PutIfAbsent,
				RemoveAny,
				RemoveIfMatch,
				ReplaceAny,
				ReplaceIfMatch
			}
			
			Write_interface = {
				Put,
				PutIfAbsent(COND_PutIfAbsentSucc),
				RemoveAny,
				RemoveIfMatch(COND_RemoveIfMatchSucc),
				ReplaceAny,
				ReplaceIfMatch(COND_ReplaceIfMatchSucc)
			}
		@Happens_before:
			Write_interface -> Read_interface
		@End
	*/

friend class CHM;
	/**
		The control structure for the hashtable
	*/
	private:
	class CHM {
		friend class cliffc_hashtable;
		private:
		atomic<kvs_data*> _newkvs;
		
		// Size of active K,V pairs
		atomic_int _size;
	
		// Count of used slots
		atomic_int _slots;
		
		// The next part of the table to copy
		atomic_int _copy_idx;
		
		// Work-done reporting
		atomic_int _copy_done;
	
		public:
		CHM(int size) {
			_size.store(size, memory_order_relaxed);
			_slots.store(0, memory_order_relaxed);
	
			_copy_idx.store(0, memory_order_relaxed);
			_copy_done.store(0, memory_order_relaxed);
		}
	
		~CHM() {}
		
		private:
			
		// Heuristic to decide if the table is too full
		bool table_full(int reprobe_cnt, int len) {
			return
				reprobe_cnt >= REPROBE_LIMIT &&
				_slots.load(memory_order_relaxed) >= reprobe_limit(len);
		}
	
		kvs_data* resize(cliffc_hashtable *topmap, kvs_data *kvs) {
			kvs_data *newkvs = _newkvs.load(memory_order_acquire);
			if (newkvs != NULL)
				return newkvs;
	
			// No copy in-progress, start one; Only double the table size
			int oldlen = kvs->_size;
			int sz = _size.load(memory_order_relaxed);
			int newsz = sz;
			
			// Just follow Cliff Click's heuristic to decide the new size
			if (sz >= (oldlen >> 2)) { // If we are 25% full
				newsz = oldlen << 1; // Double size
				if (sz >= (oldlen >> 1))
					newsz = oldlen << 2; // Double double size
			}
	
			// We do not record the record timestamp
			if (newsz <= oldlen) newsz = oldlen << 1;
			// Do not shrink ever
			if (newsz < oldlen) newsz = oldlen;
	
			// Last check cause the 'new' below is expensive
			newkvs = _newkvs.load(memory_order_acquire);
			if (newkvs != NULL) return newkvs;
	
			newkvs = new kvs_data(newsz);
			void *chm = (void*) new CHM(sz);
			newkvs->_data[0].store(chm, memory_order_relaxed);
	
			kvs_data *cur_newkvs; 
			// Another check after the slow allocation
			if ((cur_newkvs = _newkvs.load(memory_order_acquire)) != NULL)
				return cur_newkvs;
			// CAS the _newkvs to the allocated table
			kvs_data *desired = (kvs_data*) NULL;
			kvs_data *expected = (kvs_data*) newkvs; 
			if (!_newkvs.compare_exchange_strong(desired, expected, memory_order_release,
					memory_order_release)) {
				// Should clean the allocated area
				delete newkvs;
				newkvs = _newkvs.load(memory_order_acquire);
			}
			return newkvs;
		}
	
		void help_copy_impl(cliffc_hashtable *topmap, kvs_data *oldkvs,
			bool copy_all) {
			assert (get_chm(oldkvs) == this);
			kvs_data *newkvs = _newkvs.load(memory_order_acquire);
			int oldlen = oldkvs->_size;
			int min_copy_work = oldlen > 1024 ? 1024 : oldlen;
		
			// Just follow Cliff Click's code here
			int panic_start = -1;
			int copyidx;
			while (_copy_done.load(memory_order_acquire) < oldlen) {
				copyidx = _copy_idx.load(memory_order_acquire);
				if (panic_start == -1) { // No painc
					copyidx = _copy_idx.load(memory_order_acquire);
					while (copyidx < (oldlen << 1) &&
						!_copy_idx.compare_exchange_strong(copyidx, copyidx +
							min_copy_work, memory_order_release, memory_order_release))
						copyidx = _copy_idx.load(memory_order_relaxed);
					if (!(copyidx < (oldlen << 1)))
						panic_start = copyidx;
				}
	
				// Now copy the chunk of work we claimed
				int workdone = 0;
				for (int i = 0; i < min_copy_work; i++)
					if (copy_slot(topmap, (copyidx + i) & (oldlen - 1), oldkvs,
						newkvs))
						workdone++;
				if (workdone > 0)
					copy_check_and_promote(topmap, oldkvs, workdone);
	
				copyidx += min_copy_work;
				if (!copy_all && panic_start == -1)
					return; // We are done with the work we claim
			}
			copy_check_and_promote(topmap, oldkvs, 0); // See if we can promote
		}
	
		kvs_data* copy_slot_and_check(cliffc_hashtable *topmap, kvs_data
			*oldkvs, int idx, void *should_help) {
			kvs_data *newkvs = _newkvs.load(memory_order_acquire);
			// We're only here cause the caller saw a Prime
			if (copy_slot(topmap, idx, oldkvs,
				_newkvs.load(memory_order_relaxed)))
				copy_check_and_promote(topmap, oldkvs, 1); // Record the slot copied
			return (should_help == NULL) ? newkvs : topmap->help_copy(newkvs);
		}
	
		void copy_check_and_promote(cliffc_hashtable *topmap, kvs_data*
			oldkvs, int workdone) {
			int oldlen = oldkvs->_size;
			int copyDone = _copy_done.load(memory_order_relaxed);
			if (workdone > 0) {
				while (true) {
					copyDone = _copy_done.load(memory_order_relaxed);
					if (_copy_done.compare_exchange_weak(copyDone, copyDone +
						workdone, memory_order_relaxed, memory_order_relaxed))
						break;
				}
			}
	
			// Promote the new table to the current table
			if (copyDone + workdone == oldlen &&
				topmap->_kvs.load(memory_order_acquire) == oldkvs)
				topmap->_kvs.compare_exchange_strong(oldkvs,
					_newkvs.load(memory_order_relaxed), memory_order_release,
					memory_order_release);
		}
	
		bool copy_slot(cliffc_hashtable *topmap, int idx, kvs_data *oldkvs,
			kvs_data *newkvs) {
			slot *key_slot;
			while ((key_slot = key(oldkvs, idx)) == NULL)
				CAS_key(oldkvs, idx, NULL, TOMBSTONE);
	
			// First CAS old to Prime
			slot *oldval = val(oldkvs, idx);
			while (!is_prime(oldval)) {
				slot *box = (oldval == NULL || oldval == TOMBSTONE)
					? TOMBPRIME : new slot(true, oldval->_ptr);
				if (CAS_val(oldkvs, idx, oldval, box)) {
					if (box == TOMBPRIME)
						return 1; // Copy done
					// Otherwise we CAS'd the box
					oldval = box; // Record updated oldval
					break;
				}
				oldval = val(oldkvs, idx); // Else re-try
			}
	
			if (oldval == TOMBPRIME) return false; // Copy already completed here
	
			slot *old_unboxed = new slot(false, oldval->_ptr);
			int copied_into_new = (putIfMatch(topmap, newkvs, key_slot, old_unboxed,
				NULL) == NULL);
	
			// Old value is exposed in the new table
			while (!CAS_val(oldkvs, idx, oldval, TOMBPRIME))
				oldval = val(oldkvs, idx);
	
			return copied_into_new;
		}
	};

	

	private:
	static const int Default_Init_Size = 8; // Intial table size

	static slot* const MATCH_ANY;
	static slot* const NO_MATCH_OLD;

	static slot* const TOMBPRIME;
	static slot* const TOMBSTONE;

	static const int REPROBE_LIMIT = 10; // Forces a table-resize

	static Hash hashFunc;
	static KeyEqualsTo keyEqualsTo;
	static ValEqualsTo valEqualsTo;

	atomic<kvs_data*> _kvs;

	public:
	cliffc_hashtable() {
		// Should initialize the CHM for the construction of the table
		// For other CHM in kvs_data, they should be initialzed in resize()
		// because the size is determined dynamically
		kvs_data *kvs = new kvs_data(Default_Init_Size);
		void *chm = (void*) new CHM(0);
		kvs->_data[0].store(chm, memory_order_relaxed);
		_kvs.store(kvs, memory_order_release);
	}

	cliffc_hashtable(int init_size) {
		// Should initialize the CHM for the construction of the table
		// For other CHM in kvs_data, they should be initialzed in resize()
		// because the size is determined dynamically
		kvs_data *kvs = new kvs_data(init_size);
		void *chm = (void*) new CHM(0);
		kvs->_data[0].store(chm, memory_order_relaxed);
		_kvs.store(kvs, memory_order_release);
	}

	/**
		@Begin
		@Interface: Get
		@Commit_point_set: Read_Val_Point1 | Read_Val_Point2 | Read_Val_Point3
		@ID: _getKeyTag(key)
		@Action:
			@DefineVar: TypeV *_Old_Val = __map.get(key)
		@Post_check:
			_equals_val(_Old_Val, __RET__)
		@End
	*/
	TypeV* get(TypeK& key) {
		void *key_ptr = (void*) new TypeK(key);
		slot *key_slot = new slot(false, key_ptr);
		int fullhash = hash(key_slot);
		slot *V = get_impl(this, _kvs.load(memory_order_relaxed), key_slot, fullhash);
		if (V == NULL) return NULL;
		assert (!is_prime(V));
		return (TypeV*) V->_ptr;
	}

	/**
		@Begin
		@Interface: Put
		@Commit_point_set: Write_Val_Point
		@ID: _getKeyTag(key)
		@Action:
			# Remember this old value at checking point
			@DefineVar: TypeV *_Old_Val = __map.get(key)
			@Code: __map.put(key, &val);
		@Post_check:
			_equals_val(__RET__, _Old_Val)
		@End
	*/
	TypeV* put(TypeK& key, TypeV& val) {
		return putIfMatch(key, val, NO_MATCH_OLD);
	}

	/**
		@Begin
		@Interface: PutIfAbsent
		@Commit_point_set:
			Write_Val_Point | PutIfAbsent_Fail_Point
		@Condition: __map.get(key) == NULL
		@HB_condition:
			COND_PutIfAbsentSucc :: __RET__ == NULL
		@ID: _getKeyTag(key)
		@Action:
			@DefineVar: TypeV *_Old_Val = __map.get(key)
			@Code:
			if (COND_SAT)
				__map.put(key, &value);
		@Post_check:
			COND_SAT ? __RET__ == NULL : _equals_val(_Old_Val, __RET__) 
		@End
	*/
	TypeV* putIfAbsent(TypeK& key, TypeV& value) {
		return putIfMatch(key, val, TOMBSTONE);
	}

	/**
		@Begin
		@Interface: RemoveAny
		@Commit_point_set: Write_Val_Point
		@ID: _getKeyTag(key)
		@Action:
			@DefineVar: TypeV *_Old_Val = __map.get(key)
			@Code: __map.put(key, NULL);
		@Post_check:
			_equals_val(__RET__, _Old_Val)
		@End
	*/
	TypeV* remove(TypeK& key) {
		return putIfMatch(key, TOMBSTONE, NO_MATCH_OLD);
	}

	/**
		@Begin
		@Interface: RemoveIfMatch
		@Commit_point_set:
			Write_Val_Point | RemoveIfMatch_Fail_Point
		@Condition:
			_equals_val(__map.get(key), &val)
		@HB_condition:
			COND_RemoveIfMatchSucc :: __RET__ == true
		@ID: _getKeyTag(key)
		@Action:
			@Code:
			if (COND_SAT)
				__map.put(key, NULL);
		@Post_check:
			COND_SAT ? __RET__ : !__RET__
		@End
	*/
	bool remove(TypeK& key, TypeV& val) {
		slot *val_slot = val == NULL ? NULL : new slot(false, val);
		return putIfMatch(key, TOMBSTONE, val) == val;

	}

	/**
		@Begin
		@Interface: ReplaceAny
		@Commit_point_set:
			Write_Val_Point
		@ID: _getKeyTag(key)
		@Action:
			@DefineVar: TypeV *_Old_Val = __map.get(key)
		@Post_check:
			_equals_val(__RET__, _Old_Val)
		@End
	*/
	TypeV* replace(TypeK& key, TypeV& val) {
		return putIfMatch(key, val, MATCH_ANY);
	}

	/**
		@Begin
		@Interface: ReplaceIfMatch
		@Commit_point_set:
			Write_Val_Point | ReplaceIfMatch_Fail_Point
		@Condition:
			_equals_val(__map.get(key), &oldval)
		@HB_condition:
			COND_ReplaceIfMatchSucc :: __RET__ == true
		@ID: _getKeyTag(key)
		@Action:
			@Code:
			if (COND_SAT)
				__map.put(key, &newval);
		@Post_check:
			COND_SAT ? __RET__ : !__RET__
		@End
	*/
	bool replace(TypeK& key, TypeV& oldval, TypeV& newval) {
		return putIfMatch(key, newval, oldval) == oldval;
	}

	private:
	static CHM* get_chm(kvs_data* kvs) {
		return (CHM*) kvs->_data[0].load(memory_order_relaxed);
	}

	static int* get_hashes(kvs_data *kvs) {
		return (int *) kvs->_data[1].load(memory_order_relaxed);
	}
	
	// Preserve happens-before semantics on newly inserted keys
	static inline slot* key(kvs_data *kvs, int idx) {
		assert (idx >= 0 && idx < kvs->_size);
		// Corresponding to the volatile read in get_impl() and putIfMatch in
		// Cliff Click's Java implementation
		return (slot*) kvs->_data[idx * 2 + 2].load(memory_order_acquire);
	}

	/**
		The atomic operation in val() function is a "potential" commit point,
		which means in some case it is a real commit point while it is not for
		some other cases. This so happens because the val() function is such a
		fundamental function that many internal operation will call. Our
		strategy is that we label any potential commit points and check if they
		really are the commit points later.
	*/
	// Preserve happens-before semantics on newly inserted values
	static inline slot* val(kvs_data *kvs, int idx) {
		assert (idx >= 0 && idx < kvs->_size);
		// Corresponding to the volatile read in get_impl() and putIfMatch in
		// Cliff Click's Java implementation
		slot *res = (slot*) kvs->_data[idx * 2 + 3].load(memory_order_acquire);
		/**
			@Begin
			# This is a complicated potential commit point since many many functions are
			# calling val().
			@Potential_commit_point_define: true
			@Label: Read_Val_Point
			@End
		*/
		return res;


	}

	static int hash(slot *key_slot) {
		assert(key_slot != NULL && key_slot->_ptr != NULL);
		TypeK* key = (TypeK*) key_slot->_ptr;
		int h = hashFunc(*key);
		// Spread bits according to Cliff Click's code
		h += (h << 15) ^ 0xffffcd7d;
		h ^= (h >> 10);
		h += (h << 3);
		h ^= (h >> 6);
		h += (h << 2) + (h << 14);
		return h ^ (h >> 16);
	}
	
	// Heuristic to decide if reprobed too many times. 
	// Be careful here: Running over the limit on a 'get' acts as a 'miss'; on a
	// put it triggers a table resize. Several places MUST have exact agreement.
	static int reprobe_limit(int len) {
		return REPROBE_LIMIT + (len >> 2);
	}
	
	static inline bool is_prime(slot *val) {
		return (val != NULL) && val->_prime;
	}

	// Check for key equality. Try direct pointer comparison first (fast
	// negative teset) and then the full 'equals' call
	static bool keyeq(slot *K, slot *key_slot, int *hashes, int hash,
		int fullhash) {
		// Caller should've checked this.
		assert (K != NULL);
		TypeK* key_ptr = (TypeK*) key_slot->_ptr;
		return
			K == key_slot ||
				((hashes[hash] == 0 || hashes[hash] == fullhash) &&
				K != TOMBSTONE &&
				keyEqualsTo(*key_ptr, *((TypeK*) K->_ptr)));
	}

	static bool valeq(slot *val_slot1, slot *val_slot2) {
		assert (val_slot1 != NULL);
		TypeK* ptr1 = (TypeV*) val_slot1->_ptr;
		if (val_slot2 == NULL || ptr1 == NULL) return false;
		return valEqualsTo(*ptr1, *((TypeV*) val_slot2->_ptr));
	}
	
	// Together with key() preserve the happens-before relationship on newly
	// inserted keys
	static inline bool CAS_key(kvs_data *kvs, int idx, void *expected, void *desired) {
		return kvs->_data[2 * idx + 2].compare_exchange_strong(expected,
			desired, memory_order_release, memory_order_release);
	}

	/**
		Same as the val() function, we only label the CAS operation as the
		potential commit point.
	*/
	// Together with val() preserve the happens-before relationship on newly
	// inserted values
	static inline bool CAS_val(kvs_data *kvs, int idx, void *expected, void
		*desired) {
		bool res =  kvs->_data[2 * idx + 3].compare_exchange_strong(expected,
			desired, memory_order_release, memory_order_release);
		/**
			# If it is a successful put instead of a copy or any other internal
			# operantions, expected != NULL
			@Begin
			@Potential_commit_point_define: __ATOMIC_RET__ == true
			@Label: Write_Val_Point
			@End
		*/
		return res;
	}

	slot* get_impl(cliffc_hashtable *topmap, kvs_data *kvs, slot* key_slot, int
		fullhash) {
		int len = kvs->_size;
		CHM *chm = get_chm(kvs);
		int *hashes = get_hashes(kvs);

		int idx = fullhash & (len - 1);
		int reprobe_cnt = 0;
		while (true) {
			slot *K = key(kvs, idx);
			slot *V = val(kvs, idx);
			/**
				@Begin
				@Commit_point_define: V == NULL
				@Potential_commit_point_label: Read_Val_Point
				@Label: Get_Success_Point_1
				@End
			*/

			if (V == NULL) return NULL; // A miss
			
			if (keyeq(K, key_slot, hashes, idx, fullhash)) {
				// Key hit! Check if table-resize in progress
				if (!is_prime(V)) {
					/**
						@Begin
						@Commit_point_define: true
						@Potential_commit_point_label: Read_Val_Point
						@Label: Get_Success_Point_2
						@End
					*/
					return (V == TOMBSTONE) ? NULL : V; // Return this value
				}
				// Otherwise, finish the copy & retry in the new table
				return get_impl(topmap, chm->copy_slot_and_check(topmap, kvs,
					idx, key_slot), key_slot, fullhash);
			}

			if (++reprobe_cnt >= REPROBE_LIMIT ||
				key_slot == TOMBSTONE) {
				// Retry in new table
				// Atomic read (acquire) can be here 
				kvs_data *newkvs = chm->_newkvs.load(memory_order_acquire);
				/**
					@Begin
					@Commit_point_define_check: newkvs == NULL
					@Label: Get_Success_Point_3
					@End
				*/
				return newkvs == NULL ? NULL : get_impl(topmap,
					topmap->help_copy(newkvs), key_slot, fullhash);
			}

			idx = (idx + 1) & (len - 1); // Reprobe by 1
		}
	}

	// A wrapper of the essential function putIfMatch()
	TypeV* putIfMatch(TypeK& key, TypeV& value, slot *old_val) {
		// TODO: Should throw an exception rather return NULL
		if (old_val == NULL) {
			return NULL;
		}
		void *key_ptr = (void*) new TypeK(key);
		slot *key_slot = new slot(false, key_ptr);

		void *val_ptr = (void*) new TypeV(value);
		slot *value_slot = new slot(false, val_ptr);
		slot *res = putIfMatch(this, _kvs.load(memory_order_relaxed), key_slot,
			value_slot, old_val);
		// Only when copy_slot() call putIfMatch() will it return NULL
		assert (res != NULL); 
		assert (!is_prime(res));
		return res == TOMBSTONE ? NULL : (TypeV*) res->_ptr;
	}

	/**
		Put, Remove, PutIfAbsent, etc will call this function. Return the old
		value. If the returned value is equals to the expVal (or expVal is
		NO_MATCH_OLD), then this function puts the val_slot to the table 'kvs'.
		Only copy_slot will pass a NULL expVal, and putIfMatch only returns a
		NULL if passed a NULL expVal.
	*/
	static slot* putIfMatch(cliffc_hashtable *topmap, kvs_data *kvs, slot
		*key_slot, slot *val_slot, slot *expVal) {
		assert (val_slot != NULL);
		assert (!is_prime(val_slot));
		assert (!is_prime(expVal));

		int fullhash = hash(key_slot);
		int len = kvs->_size;
		CHM *chm = get_chm(kvs);
		int *hashes = get_hashes(kvs);
		int idx = fullhash & (len - 1);

		// Claim a key slot
		int reprobe_cnt = 0;
		slot *K;
		slot *V;
		kvs_data *newkvs;
		
		while (true) { // Spin till we get a key slot
			K = key(kvs, idx);
			V = val(kvs, idx);
			if (K == NULL) { // Get a free slot
				if (val_slot == TOMBSTONE) return val_slot;
				// Claim the null key-slot
				if (CAS_key(kvs, idx, NULL, key_slot)) {
					chm->_slots.fetch_add(1, memory_order_relaxed); // Inc key-slots-used count
					hashes[idx] = fullhash; // Memorize full hash
					break;
				}
				K = key(kvs, idx); // CAS failed, get updated value
				assert (K != NULL);
			}

			// Key slot not null, there exists a Key here
			if (keyeq(K, key_slot, hashes, idx, fullhash))
				break; // Got it
			
			// Notice that the logic here should be consistent with that of get.
			// The first predicate means too many reprobes means nothing in the
			// old table.
			if (++reprobe_cnt >= reprobe_limit(len) ||
				K == TOMBSTONE) { // Found a Tombstone key, no more keys
				newkvs = chm->resize(topmap, kvs);
				// Help along an existing copy
				if (expVal != NULL) topmap->help_copy(newkvs);
				return putIfMatch(topmap, newkvs, key_slot, val_slot, expVal);
			}

			idx = (idx + 1) & (len - 1); // Reprobe
		} // End of spinning till we get a Key slot

		if (val_slot == V) return V; // Fast cutout for no-change
	
		// Here it tries to resize cause it doesn't want other threads to stop
		// its progress (eagerly try to resize soon)
		newkvs = chm->_newkvs.load(memory_order_acquire);
		if (newkvs == NULL &&
			((V == NULL && chm->table_full(reprobe_cnt, len)) || is_prime(V)))
			newkvs = chm->resize(topmap, kvs); // Force the copy to start
		
		// Finish the copy and then put it in the new table
		if (newkvs != NULL)
			return putIfMatch(topmap, chm->copy_slot_and_check(topmap, kvs, idx,
				expVal), key_slot, val_slot, expVal);
		
		// Decided to update the existing table
		while (true) {
			assert (!is_prime(V));

			if (expVal != NO_MATCH_OLD &&
				V != expVal &&
				(expVal != MATCH_ANY || V == TOMBSTONE || V == NULL) &&
				!(V == NULL && expVal == TOMBSTONE) &&
				(expVal == NULL || !valeq(expVal, V))) {
				/**
					@Begin
					@Commit_point_define: expVal == TOMBSTONE
					@Potential_commit_point_label: Read_Val_Point
					@Label: PutIfAbsent_Fail_Point
						# This is a check for the PutIfAbsent() when the value
						# is not absent
					@End
				*/
				/**
					@Begin
					@Commit_point_define: expVal != NULL && val_slot == TOMBSTONE
					@Potential_commit_point_label: Read_Val_Point
					@Label: RemoveIfMatch_Fail_Point
					@End
				*/
				/**
					@Begin
					@Commit_point_define: !valeq(expVal, V)
					@Potential_commit_point_label: Read_Val_Point
					@Label: ReplaceIfMatch_Fail_Point
					@End
				*/
				return V; // Do not update!
			}

			if (CAS_val(kvs, idx, V, val_slot)) {
				/**
					@Begin
					# The only point where a successful put happens
					@Commit_point_define: true
					@Potential_commit_point_label: Write_Val_Point
					@Label: Write_Success_Point
					@End
				*/
				if (expVal != NULL) { // Not called by a table-copy
					// CAS succeeded, should adjust size
					// Both normal put's and table-copy calls putIfMatch, but
					// table-copy does not increase the number of live K/V pairs
					if ((V == NULL || V == TOMBSTONE) &&
						val_slot != TOMBSTONE)
						chm->_size.fetch_add(1, memory_order_relaxed);
					if (!(V == NULL || V == TOMBSTONE) &&
						val_slot == TOMBSTONE)
						chm->_size.fetch_add(-1, memory_order_relaxed);
				}
				return (V == NULL && expVal != NULL) ? TOMBSTONE : V;
			}
			// Else CAS failed
			V = val(kvs, idx);
			if (is_prime(V))
				return putIfMatch(topmap, chm->copy_slot_and_check(topmap, kvs,
					idx, expVal), key_slot, val_slot, expVal);
		}
	}

	// Help along an existing table-resize. This is a fast cut-out wrapper.
	kvs_data* help_copy(kvs_data *helper) {
		kvs_data *topkvs = _kvs.load(memory_order_acquire);
		CHM *topchm = get_chm(topkvs);
		// No cpy in progress
		if (topchm->_newkvs.load(memory_order_acquire) == NULL) return helper;
		topchm->help_copy_impl(this, topkvs, false);
		return helper;
	}
};

#endif
