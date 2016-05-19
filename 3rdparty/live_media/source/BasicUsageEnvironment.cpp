/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2015 Live Networks, Inc.  All rights reserved.
// Basic Usage Environment: for a simple, non-scripted, console application
// Implementation

#include "BasicUsageEnvironment.hh"
#include <stdio.h>

//by archientist
#include <dk_log4cplus_logger.h>

////////// BasicUsageEnvironment //////////

#if defined(__WIN32__) || defined(_WIN32)
extern "C" int initializeWinsockIfNecessary();
#endif

BasicUsageEnvironment::BasicUsageEnvironment(TaskScheduler& taskScheduler)
: BasicUsageEnvironment0(taskScheduler) {
#if defined(__WIN32__) || defined(_WIN32)
  if (!initializeWinsockIfNecessary()) {
    setResultErrMsg("Failed to initialize 'winsock': ");
    reportBackgroundError();
    internalError();
  }
#endif
}

BasicUsageEnvironment::~BasicUsageEnvironment() {
}

BasicUsageEnvironment*
BasicUsageEnvironment::createNew(TaskScheduler& taskScheduler) {
  return new BasicUsageEnvironment(taskScheduler);
}

int BasicUsageEnvironment::getErrno() const {
#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_WCE)
  return WSAGetLastError();
#else
  return errno;
#endif
}

UsageEnvironment& BasicUsageEnvironment::operator<<(char const* str) {
	if (str == NULL) str = "(NULL)"; // sanity check
#if defined(DEBUG)
	debuggerking::log4cplus_logger::make_debug_log("live555", "%s", str);
#else
	fprintf(stderr, "%s", str);
#endif
	return *this;
}

UsageEnvironment& BasicUsageEnvironment::operator<<(int i) {
#if defined(DEBUG)
	debuggerking::log4cplus_logger::make_debug_log("live555", "%d", i);
#else
	fprintf(stderr, "%d", i);
#endif
	return *this;
}

UsageEnvironment& BasicUsageEnvironment::operator<<(unsigned u) {
#if defined(DEBUG)
	debuggerking::log4cplus_logger::make_debug_log("live555", "%u", u);
#else
	fprintf(stderr, "%u", u);
#endif
	return *this;
}

UsageEnvironment& BasicUsageEnvironment::operator<<(double d) {
#if defined(DEBUG)
	debuggerking::log4cplus_logger::make_debug_log("live555", "%f", d);
#else
	fprintf(stderr, "%f", d);
#endif
	return *this;
}

UsageEnvironment& BasicUsageEnvironment::operator<<(void* p) {
#if defined(DEBUG)
	debuggerking::log4cplus_logger::make_debug_log("live555", "%p", p);
#else
	fprintf(stderr, "%p", p);
#endif
	return *this;
}
