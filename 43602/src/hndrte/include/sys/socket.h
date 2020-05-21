/*
 * socket.h - Broadcom HNDRTE-specific POSIX replacement library definitions
 *
 * Copyright (C) 2014, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: socket.h 241182 2011-02-17 21:50:03Z $
 */

#if !defined(_SOCKET_H_)
#define _SOCKET_H_

typedef struct in_addr {
	unsigned long s_addr;
} in_addr_t;

struct iovec
{
        void *iov_base;
        size_t iov_len;
};

int recv(int s, void *buf, size_t len, int flags);

#include <sys/select.h>
#endif /* !defined(_SOCKET_H_) */
