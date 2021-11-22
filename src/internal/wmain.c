/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <wchar.h>
#include <stdlib.h>
#include <internal/misc.h>
#include <internal/fcntl.h>
#include <internal/dlfcn.h>
#include <internal/langinfo.h>
#include <internal/process.h>
#include <internal/signal.h>
#include <internal/stdio.h>
#include <internal/security.h>

extern int main(int argc, char **argv);

int wmain(int argc, wchar_t **wargv)
{
	char **argv = NULL;
	if (argc)
	{
		argv = (char **)malloc(sizeof(char *) * (argc + 1)); // argv ends with NULL
		for (int i = 0; i < argc; i++)
		{
			argv[i] = wc_to_mb(wargv[i]);
		}
		argv[argc] = NULL;
	}

#ifdef WLIBC_POSIX_IO
	// DO NOT change the order of this.
	init_fd_table();
	initialize_stdio();
	atexit(cleanup_fd_table);
	atexit(cleanup_stdio);
#endif
#ifdef WLIBC_DLFCN
	dlfcn_init();
	atexit(dlfcn_cleanup);
#endif
#ifdef WLIBC_LANGINFO
	langinfo_init();
	atexit(langinfo_cleanup);
#endif
#ifdef WLIBC_PROCESS
	process_init();
	atexit(process_cleanup);
#endif
#ifdef WLIBC_SIGNALS
	signal_init();
	atexit(signal_cleanup);
#endif
#ifdef WLIBC_ACLS
	initialize_sids();
	atexit(cleanup_security_decsriptors);
#endif 

	int exit_status = main(argc, argv);

	if (argc)
	{
		for (int i = 0; i < argc; i++)
		{
			free(argv[i]);
		}
		free(argv);
	}

	return exit_status;
}
