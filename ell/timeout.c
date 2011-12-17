/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2011  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

#include "timeout.h"
#include "private.h"

struct l_timeout {
	int fd;
	l_timeout_notify_cb_t callback;
	l_timeout_destroy_cb_t destroy;
	void *user_data;
};

static void timeout_destroy(void *user_data)
{
	struct l_timeout *timeout = user_data;

	close(timeout->fd);

	if (timeout->destroy)
		timeout->destroy(timeout->user_data);

	free(timeout);
}

static void timeout_callback(int fd, uint32_t events, void *user_data)
{
	struct l_timeout *timeout = user_data;
	uint64_t expired;
	ssize_t result;

	result = read(timeout->fd, &expired, sizeof(expired));
	if (result != sizeof(expired))
		return;

	if (timeout->callback)
		timeout->callback(timeout, timeout->user_data);
}

LIB_EXPORT struct l_timeout *l_timeout_create(unsigned int seconds,
			l_timeout_notify_cb_t callback,
			void *user_data, l_timeout_destroy_cb_t destroy)
{
	struct l_timeout *timeout;

	timeout = malloc(sizeof(struct l_timeout));
	if (!timeout)
		return NULL;

	memset(timeout, 0, sizeof(struct l_timeout));
	timeout->callback = callback;
	timeout->destroy = destroy;
	timeout->user_data = user_data;

	timeout->fd = timerfd_create(CLOCK_MONOTONIC,
					TFD_NONBLOCK | TFD_CLOEXEC);
	if (timeout->fd < 0) {
		free(timeout);
		return NULL;
	}

	if (seconds > 0) {
		struct itimerspec itimer;

		memset(&itimer, 0, sizeof(itimer));
		itimer.it_interval.tv_sec = 0;
		itimer.it_interval.tv_nsec = 0;
		itimer.it_value.tv_sec = seconds;
		itimer.it_value.tv_nsec = 0;

		if (timerfd_settime(timeout->fd, 0, &itimer, NULL) < 0) {
			close(timeout->fd);
			free(timeout);
			return NULL;
		}
	}

	watch_add(timeout->fd, EPOLLIN | EPOLLONESHOT, timeout_callback,
						timeout, timeout_destroy);

	return timeout;
}

LIB_EXPORT void l_timeout_modify(struct l_timeout *timeout,
					unsigned int seconds)
{
	if (!timeout)
		return;

	if (timeout->fd < 0)
		return;
}

LIB_EXPORT void l_timeout_remove(struct l_timeout *timeout)
{
	if (!timeout)
		return;

	watch_remove(timeout->fd);
}