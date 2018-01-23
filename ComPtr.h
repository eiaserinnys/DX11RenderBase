#pragma once

template <typename Content>
class ComPtrT {
public:
	ComPtrT() {}

	ComPtrT(Content* ptr) : ptr(ptr) {}

	ComPtrT(ComPtrT& rhs) 
	{ 
		if (rhs.ptr != nullptr) 
		{ 
			AssignInternal(rhs); 
		} 
	}

	~ComPtrT() { Release(); }

	ComPtrT& operator = (Content* rhs)
	{
		if (ptr != rhs)
		{
			Release();
			ptr = rhs;
		}
		return *this;
	}

	ComPtrT& operator = (ComPtrT& rhs)
	{
		if (this != &rhs && ptr != rhs.ptr)
		{
			Release();
			AssignInternal(rhs);
		}
		return *this;
	}

	Content* Get() { return ptr; }

	Content** operator & () 
	{ 
		if (ptr != nullptr)
		{
			throw std::runtime_error("ComPtrT is not empty");
		}
		return &ptr; 
	}

	operator Content* () { return ptr; }

	Content* operator -> () { return ptr; }

	void Release()
	{
		if (ptr != nullptr) { ptr->Release(); ptr = nullptr; }
	}

private:
	Content* ptr = nullptr;

	void AssignInternal(ComPtrT& rhs)
	{
		ptr = rhs.ptr;
		if (ptr != nullptr) { ptr->AddRef(); }
	}
};