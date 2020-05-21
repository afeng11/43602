/*
 * stddef.h - Broadcom HNDRTE-specific POSIX replacement library definitions
 *
 * Copyright (C) 2014, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: stddef.h 241182 2011-02-17 21:50:03Z $
 */

#if !defined(_STDDEF_H_)
#define _STDDEF_H_

typedef int wchar_t;

#define offsetof(type, field) ((size_t)(&((type *)0)->field))

#endif /* !defined(_STDDEF_H_) */
