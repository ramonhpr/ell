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

#include <stdio.h>

#include <ell/ell.h>
#include <ell/dbus.h>

static void do_log(int priority, const char *format, va_list ap)
{
	vprintf(format, ap);
}

static void do_debug(const char *str, void *user_data)
{
	const char *prefix = user_data;

	l_info("%s%s", prefix, str);
}

static void method_return(struct l_dbus_message *message, void *user_data)
{
	l_main_quit();
}

static void ready_callback(void *user_data)
{
	struct l_dbus *dbus = user_data;
	struct l_dbus_message *msg;

	msg = l_dbus_message_new_method_call("org.freedesktop.DBus",
					"/org/freedesktop/DBus",
					"org.freedesktop.DBus.Introspectable",
					"Introspect");

	l_dbus_send(dbus, msg, method_return, NULL, NULL);

	l_dbus_message_unref(msg);
}

static void disconnect_callback(void *user_data)
{
	l_main_quit();
}

int main(int argc, char *argv[])
{
	struct l_dbus *dbus;

	l_log_set_handler(do_log);

	dbus = l_dbus_new(L_DBUS_SYSTEM_BUS);

	l_dbus_set_debug(dbus, do_debug, "[DBUS] ", NULL);

	l_dbus_set_ready_handler(dbus, ready_callback, dbus, NULL);
	l_dbus_set_disconnect_handler(dbus, disconnect_callback, NULL, NULL);

	l_main_run();

	l_dbus_destroy(dbus);

	return 0;
}