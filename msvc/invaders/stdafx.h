// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <Windows.h>

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
using std::ends;

#include <exception>

#include <algorithm>
using std::pair;

#include <boost/noncopyable.hpp>
using boost::noncopyable;

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include <boost/scoped_ptr.hpp>
using boost::scoped_ptr;

#include <boost/scoped_array.hpp>
using boost::scoped_array;

#include <boost/static_assert.hpp>
#define compile_assert BOOST_STATIC_ASSERT

#include <boost/thread/thread.hpp>
using boost::thread;

#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

#include <boost/function.hpp>
using boost::function;

#include <boost/array.hpp>
using boost::array;

#include <boost/format.hpp>
using boost::wformat;

#include <vector>
using std::vector;

#include <list>
using std::list;

#include <string>
using std::wstring;
using std::string;

#include "utils.h"
#include "timers.h"
#include "Basic.h"