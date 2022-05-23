/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/fcntl.h>
#include <internal/error.h>
#include <sys/ioctl.h>

#pragma warning(push)
#pragma warning(disable: 4100)

int wlibc_ioctl(int fd, unsigned long request, va_list args)
{
	// Only support regular files
	if (!validate_fd(fd))
	{
		errno = EBADF;
		return -1;
	}

	// TODO add ioctl codes that can be implemented.

	return 0;
}

#pragma warning(pop)
