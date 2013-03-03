//  (C) Copyright Beman Dawes 1999-2003. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/utility for documentation.

#ifndef NONCOPYABLE_H_3OKWH64X
#define NONCOPYABLE_H_3OKWH64X

class noncopyable {
protected:
	constexpr noncopyable() {}
	~noncopyable() {}
	
private:
	noncopyable(const noncopyable&);
	const noncopyable& operator=(const noncopyable&);
};

#endif /* end of include guard: NONCOPYABLE_H_3OKWH64X */
