/*
 * time.h - Broadcom HNDRTE-specific POSIX replacement data type definitions
 *
 * Copyright (C) 2014, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: time.h 241182 2011-02-17 21:50:03Z $
 */

#if !defined(_TIME_H_)
#define _TIME_H_

typedef long unsigned int time_t;
typedef ulong suseconds_t;
typedef int clockid_t;

struct timeval {
	time_t		tv_sec;
	suseconds_t	tv_usec;
};

struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};

struct timespec {
	time_t tv_sec;  /*seconds*/
	ulong  tv_nsec; /*nanoseconds*/
};

struct itimerspec {
	struct timespec it_interval; /*timer period*/
	struct timespec it_value;    /*timer expiration*/
};

time_t time(time_t *t);
int gettimeofday(struct timeval *tv, struct timezone *tz);
#define CLOCK_REALTIME 0x0

extern int clock_gettime(clockid_t clock_id, struct timespec *tp);

#endif /* !defined(_TIME_H_) */
