#include <chrono>
#include <vector>
#include <iostream>

#include "SmallObject.h"

struct Plain {
	virtual ~Plain() = default;
	int a = 1;
	int b = 2;
};

struct Pooled : SmallObject<> {
	int a = 1;
	int b = 2;
};

using Clock = std::chrono::steady_clock;


template <class T>
class BulkAllocator{
	public:

	BulkAllocator(int count) : m_Count(count) {  }
	
	void Allocate(){
		
		objects.clear();
		objects.reserve(m_Count);

		for (int i = 0; i < m_Count; ++i) { objects.push_back(new T);}
		
	}

	void Deallocate(){
		for (int i = 0; i < m_Count; ++i) { delete objects[i]; }
		objects.clear();
	}

	~BulkAllocator() {
		Deallocate();
	}
	
	private:

	std::vector<T*> objects;
	int m_Count = 0;
};


template <class T>
double TimeMs(BulkAllocator<T>& allocator)
{
	auto t0 = Clock::now();
	allocator.Allocate();
	std::chrono::duration<double, std::milli> ms = Clock::now() - t0;
	return ms.count();
}


int main()
{
	
	const int N = 100'000;
	
	BulkAllocator<Plain> plainAllocator(N);
	BulkAllocator<Pooled> pooledAllocator(N);
	
	auto plainTimeMs = TimeMs(plainAllocator);
	auto pooledTimeMs = TimeMs(pooledAllocator);
	
	// logging time
	std::cout << "Time taken for allocation:" << std::endl;
	std::cout << "Plain: " << plainTimeMs << " ms" << std::endl;
	std::cout << "Pooled: " << pooledTimeMs << " ms" << std::endl;


	return 0;
}
