#ifndef ATOMIC_HPP
#define ATOMIC_HPP

#include <iostream>
#include <cstdint>

template <class T>
class atomic_stamped {

private:
	union __ref {
			struct { T* ptr; uint64_t stamp; } pair;
			__uint128_t val;
	};
	__ref ref;

public:

	// construct an atomic_stamped
	// with initial values for pointer and stamp
	atomic_stamped(T* ptr, uint64_t stamp)
	{
		set(ptr, stamp);
	}

	// ! Default constructor
	atomic_stamped()
	{
		set(nullptr, 0);
	}


	// compare and set
	// curr is the current pointer value
	// next is the new pointer value
	// stamp is the current stamp value
	// nstamp is the new stamp value
	bool cas(T* curr, T* next, uint64_t stamp, uint64_t nstamp)
	{
		__ref c, n;
		c.pair.ptr = curr;
		c.pair.stamp = stamp;
		n.pair.ptr = next;
		n.pair.stamp = nstamp;
		
		// std::cout << "vvvvvvvvvvvvv" << std::endl;
		// std::cout << "ref.ptr: " << ref.pair.ptr << std::endl;
		
		// std::cout << "c.ptr: " << c.pair.ptr << " c.stamp: " << c.pair.stamp << std::endl;
		
		// std::cout << "n.ptr: " << n.pair.ptr << " n.stamp: " << n.pair.stamp << std::endl;

		// std::cout << "^^^^^^^^^^^^^" << std::endl;
		
		bool res = __atomic_compare_exchange(&ref.val, &c.val, &n.val, 0, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
		return res;
	}

	// get pointer
	// returns the pointer
	// caller must pass an integer to receive the stamp

	T* get(uint64_t &stamp)
	{
		__ref u;
		//if(ref == nullptr)
			//printf("ref.val: %p\n", ref);

		__atomic_load(&ref.val, &u.val, __ATOMIC_RELAXED);
		stamp = u.pair.stamp;
		return u.pair.ptr;
	}

	// set pointer and stamp values

	void set(T* ptr, uint64_t stamp)
	{
		__ref u;
		u.pair.ptr = ptr;
		u.pair.stamp = stamp;
		__atomic_store(&ref.val, &u.val, __ATOMIC_RELAXED);
	}
};

#endif // 