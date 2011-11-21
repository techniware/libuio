/*
 * libuio - UserspaceIO helper library
 *
 * Copyright (C) 2011 Benedikt Spranger
 * based on libUIO by Hans J. Koch
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA  02110-1301 USA
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "libuio_internal.h"

/**
 * @defgroup libuio_attr libuio attribute functions
 * @ingroup libuio_public
 * @brief public attribute functions
 * @{
 */

/**
 * list UIO attributes
 * @param info UIO device info struct
 * @returns attribute list or NULL on failure
 */
char **uio_list_attr (struct uio_info_t* info)
{
	struct dirent **namelist;
	char **list;
	int i, t = 0, nr;

	if (!info)
	{
		errno = EINVAL;
		g_error (_("uio_list_attr: %s"), g_strerror (errno));
		return NULL;
	}

	nr = scandir (info->path, &namelist, 0, alphasort);
	if (nr < 0)
	{
		g_error (_("scandir: %s"), g_strerror (errno));
		return NULL;
	}

	list = calloc (nr, sizeof (*list));
	if (!list)
	{
		errno = ENOMEM;
		g_error (_("calloc: %s"), g_strerror (errno));
		goto out;
	}

	for (i = 0; i < nr; i++)
	{
		if (!strcmp (namelist [i]->d_name, ".") ||
		    !strcmp (namelist [i]->d_name, ".."))
			continue;

		list [t] = strdup (namelist [i]->d_name);
		if (list [t])
			t++;
	}

out:
	for (i = 0; i < nr; i++)
		free (namelist [i]);
	free (namelist);

	return list;
}

/**
 * get UIO attribute
 * @param info UIO device info struct
 * @param attr attribute name
 * @returns UIO attribute content or NULL on failure
 */
char *uio_get_attr (struct uio_info_t* info, char *attr)
{
	char filename [PATH_MAX];

	if (!info || !attr)
	{
		g_error (_("uio_get_attr: %s\n"), g_strerror (EINVAL));
		return NULL;
	}
	snprintf (filename, PATH_MAX, "%s/%s", info->path, attr);

	return first_line_from_file (filename);
}

/**
 * set UIO attribute
 * @param info UIO device info struct
 * @param attr attribute name
 * @param value attribute content
 * @returns 0 on succes or -1 on failure
 */
int uio_set_attr (struct uio_info_t* info, char *attr, char *value)
{
	char filename [PATH_MAX];
	int err, fd, ret = -1;
	size_t len;

	if (!info || !attr || !value) {
		errno = EINVAL;
		g_error (_("uio_set_attr: %s"), g_strerror (errno));
		return -1;
	}
	snprintf (filename, PATH_MAX, "%s/%s", info->path, attr);

	fd = open (filename, O_WRONLY);
	if (fd < 0)
	{
		g_error (_("open: %s"), g_strerror (errno));
		return -1;
	}

	len = write (fd, value, strlen (value));
	if (len > 0)
		ret = 0;

	err = errno;
	close (fd);
	errno = err;

	return ret;
}

/** @} */
