#ifndef _OBJECT_TYPE_H_
#define _OBJECT_TYPE_H_

#include "intrusive_ptr.hpp"

#include <limits>
#include <string>
#include <cstring>
#include "utility.hpp"

enum OBJECT_TYPE {
	OBJECT_UNINITIALIZED,
	OBJECT_STAR,
	OBJECT_BODY,
	OBJECT_NEBULA,
	OBJECT_CONSTELLATION,
};

//! Intrusive pointer used to manage StelObject with smart pointers
class ObjectBase;
typedef IntrusivePtr<ObjectBase> ObjectBaseP;

#endif
