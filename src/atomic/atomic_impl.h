#include <atomic>
#include <mutex>


// Any trivially copayble type can be atomic
// - Continuous chunk of memory
// - Copying object means copying all bits (memcpy)
// - No virtual functions, noexcept constructor

namespace case_atomic
{

void operation()
{
	std::atomic<int> x;

	// Explicit reads and writes
	int y = x.load(); // int y = x;
	x.store(y);       // x = y;

	// Atomic exchange
	int z = x.exchange(y); // z = x;
                           // x = y;

	/* Compare and swap (conditional exchange)
	if (x == y)
	{
		x = z;
		return true;
	}
	else
	{
		y = x;
		return false;
	} */
	bool success = x.compare_exchange_strong(y, z);

	// load old value and assign new
	z = x.fetch_add(y);
	z = x.fetch_sub(y);
	z = x.fetch_or(y);
	z = x.fetch_xor(y);
}

struct LockFree
{
	std::atomic<int> m_x{ 0 };

	void runner()
	{
		int x0 = m_x.load();
		while (!m_x.compare_exchange_strong(x0, x0 + 1))
		{
			// m_x = x0 + 1;
			// x0 is unique value among all threads now
			// do the lockfree impl with x0 within this block
		}
	}
};

// it's not always lock-free due to alignment
namespace lockfree
{
	long x;
	struct A { long x; };                 // lock-free
	struct B { long x; long y; };         // lock-free
	struct C { long x; int y; };          // lock-free
	struct D { int x; int y; int z; };    // not lock-free under x86 (12 bytes)
	struct E { long x; long y; long z; }; // not lock-free under x86 (>16 bytes)

	void check()
	{
		bool isALockFree = std::atomic<A>{}.is_lock_free();
		bool isBLockFree = std::atomic<B>{}.is_lock_free();
		bool isCLockFree = std::atomic<C>{}.is_lock_free();
		bool isCLockFree = std::atomic<D>{}.is_lock_free();
		bool isCLockFree = std::atomic<E>{}.is_lock_free();
	}
}

namespace compare_and_swap
{
	struct Lock;
	struct TimedLock;

	template<typename T>
	bool compare_exchange_strong(T& old_v, T new_v)
	{
		// Optimization (read is faster)
		T tmp = value; // current value of the atomic
		if (tmp != old_v)
		{
			old_v = tmp;
			return false;
		}

		Lock l; // Get exclusive access
		tmp = value; // value could have changed
		if (tmp != old_v)
		{
			old_v = tmp;
			return false;
		}
		value = new_v;
		return true;
	}
	template<typename T>
	bool compare_exchange_weak(T& old_v, T new_v)
	{
		// Optimization (read is faster)
		T tmp = value; // current value of the atomic
		if (tmp != old_v)
		{
			old_v = tmp;
			return false;
		}

		// It is expensive to get exclusive lock
		// so it may fail with certain hardware
		// Do the timed lock and return false if it fails
		TimedLock l; // Get exclusive access
		if (!l.locked()) 
			return false; // old_v is correct

		tmp = value; // value could have changed
		if (tmp != old_v)
		{
			old_v = tmp;
			return false;
		}
		value = new_v;
		return true;
	}
}

namespace atomic_list
{
	struct Node
	{
		int value;
		Node* next;
	};

	std::atomic<Node*> head;
	void push_front(int x)
	{
		Node* new_node = new Node;
		new_node->value = x;
		Node* old_head = head;
		do
		{
			new_node->next = old_head;
		} while (!head.compare_exchange_strong(old_head, new_node));
	}
}

namespace memory_barrier
{
	// std::memory_order_relaxed : any order can be possible
	// std::memory_order_aquire  : nothing after barrier can go before barrier
	// std::memory_order_release : nothing before barrier can go after barrier
	// std::memory_order_seq_cst : strongest order

	// Step
	// Thread 1 write atomic var x with release barrier
	// - All write is done before visible
	// Thread 2 reads atomic var x with aquire barrier
	// - All reads are done after the barrier
	// All memory writes hppened in thread 1 before the barrier
	// become visible in thread 2 after the barrier

	// Concept
	// Thead 1 prepares data (write) then release it by updating atomic variable x
	// Thread 2 aquires atomic variable x and the data is guaranteed to be visible
}
}
