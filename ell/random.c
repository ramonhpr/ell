/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2015  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/random.h>

#include "random.h"
#include "private.h"

static inline int getrandom(void *buffer, size_t count, unsigned flags) {
        return syscall(__NR_getrandom, buffer, count, flags);
}

/**
 * l_getrandom:
 * @buf: buffer to fill with random data
 * @len: length of random data requested
 *
 * Request a number of randomly generated bytes given by @len and put them
 * into buffer @buf.
 *
 * Returns: true if the random data could be generated, false otherwise.
 **/
LIB_EXPORT bool l_getrandom(void *buf, size_t len)
{
	int ret;

	if (len > 256)
		return false;

	ret = getrandom(buf, len, 0);
	if (ret < 0)
		return false;

	if ((size_t) ret == len)
		return true;

	return false;
}
