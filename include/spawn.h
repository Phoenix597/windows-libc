/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SPAWN_H
#define WLIBC_SPAWN_H

#include <wlibc.h>
#include <signal.h>
#include <sys/types.h>

_WLIBC_BEGIN_DECLS

/* Data structure to contain attributes for thread creation.  */
typedef struct
{
	short int flags;
	pid_t pgrp;
	sigset_t sigdefault;
	sigset_t sigmask;
	// struct sched_param sp;
	int policy;
} spawnattr_t;

typedef spawnattr_t posix_spawnattr_t;

/* Data structure to contain information about the actions to be
   performed in the new process with respect to file descriptors.  */

typedef enum _spawn_action_type
{
	open_action,
	close_action,
	dup2_action,
	chdir_action,
	fchdir_action
} spawn_action_type;

struct spawn_action
{
	spawn_action_type type;

	union {
		struct
		{
			int fd;
			int oflag;
			mode_t mode;
			int length;
			char *path;
		} open_action;

		struct
		{
			int fd;
		} close_action;

		struct
		{
			int oldfd;
			int newfd;
		} dup2_action;

		struct
		{
			int length;
			char *path;
		} chdir_action;

		struct
		{
			int fd;
		} fchdir_action;
	};
};

typedef struct
{
	unsigned short size;
	unsigned short used;
	struct spawn_action *actions;
} spawn_actions_t;

typedef spawn_actions_t posix_spawn_file_actions_t;

// Flags to be set in the posix_spawnattr_t.
#define POSIX_SPAWN_SETSCHEDULER  0x0 // Unsupported
#define POSIX_SPAWN_RESETIDS      0x0 // Unsupported
#define POSIX_SPAWN_USEVFORK      0x0 // Unsupported
#define POSIX_SPAWN_SETSID        0x0 // Unsupported
#define POSIX_SPAWN_SETPGROUP     0x1
#define POSIX_SPAWN_SETSIGDEF     0x2
#define POSIX_SPAWN_SETSIGMASK    0x4
#define POSIX_SPAWN_SETSCHEDPARAM 0x8

// Spawn API.
WLIBC_API int wlibc_spawn(pid_t *restrict pid, const char *restrict path, const spawn_actions_t *restrict actions,
						  const spawnattr_t *restrict attributes, int use_path, char *restrict const argv[], char *restrict const env[]);

WLIBC_INLINE int posix_spawn(pid_t *restrict pid, const char *restrict path, const posix_spawn_file_actions_t *restrict actions,
							 const posix_spawnattr_t *restrict attributes, char *restrict const argv[], char *restrict const env[])
{
	return wlibc_spawn(pid, path, actions, attributes, 0, argv, env);
}
WLIBC_INLINE int posix_spawnp(pid_t *restrict pid, const char *restrict path, const posix_spawn_file_actions_t *restrict actions,
							  const posix_spawnattr_t *restrict attributes, char *restrict const argv[], char *restrict const env[])
{
	return wlibc_spawn(pid, path, actions, attributes, 1, argv, env);
}

// Spawn attributes.
WLIBC_API int wlibc_spawnattr_init(spawnattr_t *attributes);
WLIBC_API int wlibc_spawnattr_getsigdefault(const spawnattr_t *restrict attributes, sigset_t *restrict sigdefault);
WLIBC_API int wlibc_spawnattr_setsigdefault(spawnattr_t *restrict attributes, const sigset_t *restrict sigdefault);
WLIBC_API int wlibc_spawnattr_getsigmask(const spawnattr_t *restrict attributes, sigset_t *restrict sigmask);
WLIBC_API int wlibc_spawnattr_setsigmask(spawnattr_t *restrict attributes, const sigset_t *restrict sigmask);
WLIBC_API int wlibc_spawnattr_getflags(const spawnattr_t *restrict attributes, short int *restrict flags);
WLIBC_API int wlibc_spawnattr_setflags(spawnattr_t *attributes, short int flags);

WLIBC_INLINE int posix_spawnattr_init(posix_spawnattr_t *attributes)
{
	return wlibc_spawnattr_init(attributes);
}

WLIBC_INLINE int posix_spawnattr_destroy(posix_spawnattr_t *attributes)
{
	// nop
	return 0;
}

WLIBC_INLINE int posix_spawnattr_getsigdefault(const posix_spawnattr_t *restrict attributes, sigset_t *restrict sigdefault)
{
	return wlibc_spawnattr_getsigdefault(attributes, sigdefault);
}

WLIBC_INLINE int posix_spawnattr_setsigdefault(posix_spawnattr_t *restrict attributes, const sigset_t *restrict sigdefault)
{
	return wlibc_spawnattr_setsigdefault(attributes, sigdefault);
}

WLIBC_INLINE int posix_spawnattr_getsigmask(const posix_spawnattr_t *restrict attributes, sigset_t *restrict sigmask)
{
	return wlibc_spawnattr_getsigmask(attributes, sigmask);
}

WLIBC_INLINE int posix_spawnattr_setsigmask(posix_spawnattr_t *restrict attributes, const sigset_t *restrict sigmask)
{
	return wlibc_spawnattr_setsigmask(attributes, sigmask);
}

WLIBC_INLINE int posix_spawnattr_getflags(const posix_spawnattr_t *restrict attributes, short int *restrict flags)
{
	return wlibc_spawnattr_getflags(attributes, flags);
}

WLIBC_INLINE int posix_spawnattr_setflags(posix_spawnattr_t *attributes, short int flags)
{
	return wlibc_spawnattr_setflags(attributes, flags);
}

// extern int posix_spawnattr_getpgroup(const posix_spawnattr_t *restrict attributes, pid_t *restrict pgroup);
// extern int posix_spawnattr_setpgroup(posix_spawnattr_t *attributes, pid_t pgroup);
// extern int posix_spawnattr_getschedpolicy(const posix_spawnattr_t *restrict attributes, int *restrict schedpolicy);
// extern int posix_spawnattr_setschedpolicy(posix_spawnattr_t *attributes, int schedpolicy);
//  extern int posix_spawnattr_getschedparam(const posix_spawnattr_t *restrict attributes, struct sched_param *restrict schedparam);
//  extern int posix_spawnattr_setschedparam(posix_spawnattr_t *restrict attributes, const struct sched_param *restrict schedparam);

// Spawn actions.
WLIBC_API int wlibc_spawn_file_actions_init(spawn_actions_t *actions);
WLIBC_API int wlibc_spawn_file_actions_destroy(spawn_actions_t *actions);
WLIBC_API int wlibc_spawn_file_actions_addopen(spawn_actions_t *actions, int fd, const char *restrict path, int oflag, mode_t mode);
WLIBC_API int wlibc_spawn_file_actions_addclose(spawn_actions_t *actions, int fd);
WLIBC_API int wlibc_spawn_file_actions_adddup2(spawn_actions_t *actions, int oldfd, int newfd);
WLIBC_API int wlibc_spawn_file_actions_addchdir(spawn_actions_t *restrict actions, const char *restrict path);
WLIBC_API int wlibc_spawn_file_actions_addfchdir(spawn_actions_t *actions, int fd);

WLIBC_INLINE int posix_spawn_file_actions_init(posix_spawn_file_actions_t *actions)
{
	return wlibc_spawn_file_actions_init(actions);
}

WLIBC_INLINE int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *actions)
{
	return wlibc_spawn_file_actions_destroy(actions);
}

WLIBC_INLINE int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *restrict actions, int fd, const char *restrict path,
												  int oflag, mode_t mode)
{
	return wlibc_spawn_file_actions_addopen(actions, fd, path, oflag, mode);
}

WLIBC_INLINE int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *actions, int fd)
{
	return wlibc_spawn_file_actions_addclose(actions, fd);
}

WLIBC_INLINE int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *actions, int oldfd, int newfd)
{
	return wlibc_spawn_file_actions_adddup2(actions, oldfd, newfd);
}

WLIBC_INLINE int posix_spawn_file_actions_addchdir(posix_spawn_file_actions_t *restrict actions, const char *restrict path)
{
	return wlibc_spawn_file_actions_addchdir(actions, path);
}

WLIBC_INLINE int posix_spawn_file_actions_addfchdir(posix_spawn_file_actions_t *actions, int fd)
{
	return wlibc_spawn_file_actions_addfchdir(actions, fd);
}

#define posix_spawn_file_actions_addchdir_np  posix_spawn_file_actions_addchdir
#define posix_spawn_file_actions_addfchdir_np posix_spawn_file_actions_addfchdir

_WLIBC_END_DECLS

#endif
