/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_UNISTD_H
#define WLIBC_UNISTD_H

#include <wlibc.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

_WLIBC_BEGIN_DECLS

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define F_OK 0x0 // File Exists
#define R_OK 0x4 // Read permission
#define W_OK 0x2 // Write permission
#define X_OK 0x1 // Execute/Search permission

// True for (NT AUTHORITY\SYSTEM) and (BUILTIN\Administrators)
#define ROOT_UID 0

WLIBC_API int wlibc_common_remove(int dirfd, const char *path, int flags);

WLIBC_API int wlibc_common_access(int dirfd, const char *path, int mode, int flags);

WLIBC_INLINE int access(const char *path, int mode)
{
	return wlibc_common_access(AT_FDCWD, path, mode, 0);
}

WLIBC_INLINE int eaccess(const char *path, int mode)
{
	return wlibc_common_access(AT_FDCWD, path, mode, AT_EACCESS);
}

WLIBC_INLINE int euidaccess(const char *path, int mode)
{
	// Same as eaccess
	return wlibc_common_access(AT_FDCWD, path, mode, AT_EACCESS);
}

WLIBC_INLINE int faccessat(int dirfd, const char *path, int mode, int flags)
{
	return wlibc_common_access(dirfd, path, mode, flags);
}

WLIBC_API int wlibc_close(int fd);
WLIBC_INLINE int close(int fd)
{
	return wlibc_close(fd);
}

WLIBC_API int wlibc_chdir(const char *name);
WLIBC_API int wlibc_fchdir(int fd);

WLIBC_INLINE int chdir(const char *name)
{
	return wlibc_chdir(name);
}

WLIBC_INLINE int fchdir(int fd)
{
	return wlibc_fchdir(fd);
}

WLIBC_API int wlibc_common_chown(int dirfd, const char *path, uid_t owner, gid_t group, int flags);

WLIBC_INLINE int chown(const char *path, uid_t owner, gid_t group)
{
	return wlibc_common_chown(AT_FDCWD, path, owner, group, 0);
}

WLIBC_INLINE int lchown(const char *path, uid_t owner, gid_t group)
{
	return wlibc_common_chown(AT_FDCWD, path, owner, group, AT_SYMLINK_NOFOLLOW);
}

WLIBC_INLINE int fchown(int fd, uid_t owner, gid_t group)
{
	return wlibc_common_chown(fd, NULL, owner, group, AT_EMPTY_PATH);
}

WLIBC_INLINE int fchownat(int dirfd, const char *path, uid_t owner, gid_t group, int flags)
{
	return wlibc_common_chown(dirfd, path, owner, group, flags);
}

WLIBC_API int wlibc_common_dup(int oldfd, int newfd, int flags);

WLIBC_INLINE int dup(int fd)
{
	return wlibc_common_dup(fd, -1, 0);
}

WLIBC_INLINE int dup2(int oldfd, int newfd)
{
	if (oldfd < 0 || newfd < 0)
	{
		errno = EINVAL;
		return -1;
	}

	return wlibc_common_dup(oldfd, newfd, 0);
}

WLIBC_INLINE int dup3(int oldfd, int newfd, int flags)
{
	if (oldfd < 0 || newfd < 0)
	{
		errno = EINVAL;
		return -1;
	}

	if (newfd == oldfd)
	{
		errno = EINVAL;
		return -1;
	}

	return wlibc_common_dup(oldfd, newfd, flags);
}

WLIBC_API int wlibc_fdatasync(int fd);
WLIBC_API int wlibc_fsync(int fd);

WLIBC_INLINE int fdatasync(int fd)
{
	return wlibc_fdatasync(fd);
}

WLIBC_INLINE int fsync(int fd)
{
	return wlibc_fsync(fd);
}

WLIBC_API char *wlibc_getcwd(char *buf, size_t size);
WLIBC_API wchar_t *wlibc_wgetcwd(wchar_t *wbuf, size_t size);

WLIBC_INLINE char *getcwd(char *buf, size_t size)
{
	return wlibc_getcwd(buf, size);
}

WLIBC_INLINE wchar_t *wgetcwd(wchar_t *wbuf, size_t size)
{
	return wlibc_wgetcwd(wbuf, size);
}

WLIBC_API gid_t wlibc_getgid(void);

WLIBC_INLINE gid_t getgid(void)
{
	return wlibc_getgid();
}

WLIBC_INLINE gid_t getegid(void)
{
	// Same as getgid
	return wlibc_getgid();
}

WLIBC_API pid_t wlibc_getpid(void);
WLIBC_API pid_t wlibc_getppid(void);

WLIBC_INLINE pid_t getpid(void)
{
	return wlibc_getpid();
}

WLIBC_INLINE pid_t getppid(void)
{
	return wlibc_getppid();
}

WLIBC_API pid_t wlibc_gettid(void);

WLIBC_INLINE pid_t gettid(void)
{
	return wlibc_gettid();
}

WLIBC_API uid_t wlibc_getuid(void);

WLIBC_INLINE uid_t getuid(void)
{
	return wlibc_getuid();
}

WLIBC_INLINE uid_t geteuid(void)
{
	// Same as getuid
	return wlibc_getuid();
}

WLIBC_API int wlibc_getdomainname(char *name, size_t length);
WLIBC_API int wlibc_gethostname(char *name, size_t length);

WLIBC_INLINE int getdomainname(char *name, size_t length)
{
	return wlibc_getdomainname(name, length);
}

WLIBC_INLINE int gethostname(char *name, size_t length)
{
	return wlibc_gethostname(name, length);
}

WLIBC_API int wlibc_getpagesize(void);
WLIBC_INLINE int getpagesize(void)
{
	return wlibc_getpagesize();
}

WLIBC_API int wlibc_common_pipe(int pipefd[2], int flags);

WLIBC_INLINE int pipe(int pipefd[2])
{
	return wlibc_common_pipe(pipefd, 0);
}

WLIBC_INLINE int pipe2(int pipefd[2], int flags)
{
	return wlibc_common_pipe(pipefd, flags);
}

WLIBC_API ssize_t wlibc_pread(int fd, void *buf, size_t count, off_t offset);
WLIBC_API ssize_t wlibc_pwrite(int fd, const void *buf, size_t count, off_t offset);

WLIBC_INLINE ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
	return wlibc_pread(fd, buf, count, offset);
}
WLIBC_INLINE ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
	return wlibc_pwrite(fd, buf, count, offset);
}

WLIBC_API int wlibc_isatty(int fd);
WLIBC_INLINE int isatty(int fd)
{
	return wlibc_isatty(fd);
}

WLIBC_API int wlibc_kill(pid_t pid, int sig);
WLIBC_INLINE int kill(pid_t pid, int sig)
{
	return wlibc_kill(pid, sig);
}

WLIBC_API int wlibc_common_link(int olddirfd, const char *restrict source, int newdirfd, const char *restrict target, int flags);
WLIBC_INLINE int linkat(int olddirfd, const char *restrict source, int newdirfd, const char *restrict target, int flags)
{
	return wlibc_common_link(olddirfd, source, newdirfd, target, flags);
}

WLIBC_INLINE int link(const char *restrict source, const char *restrict target)
{
	return wlibc_common_link(AT_FDCWD, source, AT_FDCWD, target, 0);
}

WLIBC_API off_t wlibc_lseek(int fd, off_t offset, int whence);
WLIBC_INLINE off_t lseek(int fd, off_t offset, int whence)
{
	return wlibc_lseek(fd, offset, whence);
}

WLIBC_API ssize_t wlibc_read(int fd, void *buf, size_t count);
WLIBC_INLINE ssize_t read(int fd, void *buf, size_t count)
{
	return wlibc_read(fd, buf, count);
}

WLIBC_INLINE int rmdir(const char *path)
{
	return wlibc_common_remove(AT_FDCWD, path, AT_REMOVEDIR);
}

WLIBC_INLINE int rmdirat(int dirfd, const char *path)
{
	return wlibc_common_remove(dirfd, path, AT_REMOVEDIR);
}

WLIBC_API ssize_t wlibc_common_readlink(int dirfd, const char *restrict path, char *restrict buf, size_t bufsiz);
WLIBC_INLINE ssize_t readlinkat(int dirfd, const char *restrict path, char *restrict buf, size_t bufsiz)
{
	return wlibc_common_readlink(dirfd, path, buf, bufsiz);
}

WLIBC_INLINE ssize_t readlink(const char *restrict path, char *restrict buf, size_t bufsiz)
{
	return wlibc_common_readlink(AT_FDCWD, path, buf, bufsiz);
}

WLIBC_API int wlibc_common_symlink(const char *restrict source, int dirfd, const char *restrict target, mode_t mode);

WLIBC_INLINE int symlinkat2(const char *restrict source, int dirfd, const char *restrict target, mode_t mode)
{
	return wlibc_common_symlink(source, dirfd, target, mode);
}

WLIBC_INLINE int symlinkat(const char *restrict source, int dirfd, const char *restrict target)
{
	return wlibc_common_symlink(source, dirfd, target, 0700);
}

WLIBC_INLINE int symlink(const char *restrict source, const char *restrict target)
{
	return wlibc_common_symlink(source, AT_FDCWD, target, 0700);
}

WLIBC_API int wlibc_common_sleep(long long nanoseconds_100);
WLIBC_INLINE int sleep(unsigned int seconds)
{
	return wlibc_common_sleep(seconds * 10000000);
}

WLIBC_INLINE int msleep(unsigned int milliseconds)
{
	return wlibc_common_sleep(milliseconds * 10000);
}

WLIBC_INLINE int usleep(useconds_t microseconds)
{
	return wlibc_common_sleep(microseconds * 10);
}

WLIBC_API int wlibc_truncate(const char *path, off_t length);
WLIBC_INLINE int truncate(const char *path, off_t length)
{
	return wlibc_truncate(path, length);
}

WLIBC_API int wlibc_ftruncate(int fd, off_t length);
WLIBC_INLINE int ftruncate(int fd, off_t length)
{
	return wlibc_ftruncate(fd, length);
}

WLIBC_API char *wlibc_ttyname(int fd);
WLIBC_API int wlibc_ttyname_r(int fd, char *buf, size_t bufsiz);

WLIBC_INLINE char *ttyname(int fd)
{
	return wlibc_ttyname(fd);
}

WLIBC_INLINE int ttyname_r(int fd, char *buf, size_t bufsiz)
{
	return wlibc_ttyname_r(fd, buf, bufsiz);
}

WLIBC_INLINE int unlink(const char *path)
{
	return wlibc_common_remove(AT_FDCWD, path, 0);
}

WLIBC_INLINE int unlinkat(int dirfd, const char *path, int flags)
{
	return wlibc_common_remove(dirfd, path, flags);
}

WLIBC_API ssize_t wlibc_write(int fd, const void *buf, size_t count);
WLIBC_INLINE ssize_t write(int fd, const void *buf, size_t count)
{
	return wlibc_write(fd, buf, count);
}

_WLIBC_END_DECLS

#endif
