/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <Windows.h>
#include <internal/error.h>
#include <internal/misc.h>
#include <errno.h>
#include <internal/fcntl.h>
#include <time.h>
#include <unistd.h>

// ssize_t common_readlink(const wchar_t *restrict wpath, wchar_t *restrict wbuf, size_t bufsiz, int give_absolute);
// ssize_t do_readlink(HANDLE handle, char *restrict buf, size_t bufsiz);

/* 116444736000000000 is the number of 100 nanosecond intervals from
   January 1st 1601 to January 1st 1970 (UTC)
*/
struct timespec FILETIME_to_timespec(FILETIME FT)
{
	struct timespec result;
	time_t epoch = ((time_t)FT.dwHighDateTime << 32) + FT.dwLowDateTime;
	epoch -= 116444736000000000LL;
	result.tv_sec = epoch / 10000000;
	result.tv_nsec = (epoch % 10000000) * 100;
	return result;
}

struct timespec LARGE_INTEGER_to_timespec(LARGE_INTEGER LT)
{
	struct timespec result;
	time_t epoch = LT.QuadPart - 116444736000000000LL;
	result.tv_sec = epoch / 10000000;
	result.tv_nsec = (epoch % 10000000) * 100;
	return result;
}

#if 0
int do_stat(HANDLE file, struct stat *statbuf)
{
	DWORD type = GetFileType(file);

	if (type == FILE_TYPE_DISK)
	{
		statbuf->st_rdev = 0;

		// st_mode
		BY_HANDLE_FILE_INFORMATION FILE_INFO;
		GetFileInformationByHandle(file, &FILE_INFO);
		DWORD attributes = FILE_INFO.dwFileAttributes;
		// From readdir.c
		if (attributes & FILE_ATTRIBUTE_REPARSE_POINT)
		{
			statbuf->st_mode = S_IFLNK;
			statbuf->st_mode |= S_IREAD | S_IWRITE | S_IEXEC;
		}
		else if (attributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			statbuf->st_mode = S_IFDIR;
			statbuf->st_mode |= S_IREAD | S_IWRITE | S_IEXEC;
		}
		else if ((attributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE |
								 FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_SPARSE_FILE | FILE_ATTRIBUTE_COMPRESSED |
								 FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_ATTRIBUTE_ENCRYPTED)) == 0)
		{
			statbuf->st_mode = S_IFREG;
			if (attributes & FILE_ATTRIBUTE_READONLY)
			{
				statbuf->st_mode |= S_IREAD;
			}
			else
			{
				statbuf->st_mode |= S_IREAD | S_IWRITE;
			}
		}

		// st_[amc]tim
		statbuf->st_atim = FILETIME_to_timespec(FILE_INFO.ftLastAccessTime);
		statbuf->st_mtim = FILETIME_to_timespec(FILE_INFO.ftLastWriteTime);
		statbuf->st_ctim = FILETIME_to_timespec(FILE_INFO.ftCreationTime);

		// st_dev
		wchar_t wbuf[MAX_PATH];
		if (!GetFinalPathNameByHandle(file, wbuf, MAX_PATH, FILE_NAME_NORMALIZED))
		{
			map_win32_error_to_wlibc(GetLastError());
			return -1;
		}
		char drive_name;
		wcstombs(&drive_name, wbuf + 4, 1);     // '//?/'
		statbuf->st_dev = drive_name - 'A' + 1; // A ->1, B->2 ...

		// Set the S_IEXEC bit here as it requires the name
		if (has_executable_extenstion(wbuf + 4))
		{
			statbuf->st_mode |= S_IEXEC;
		}

		// st_ino
		FILE_ID_INFO INO_INFO;
		if (!GetFileInformationByHandleEx(file, FileIdInfo, &INO_INFO, sizeof(FILE_ID_INFO)))
		{
			map_win32_error_to_wlibc(GetLastError());
			return -1;
		}
		memcpy(&statbuf->st_ino, INO_INFO.FileId.Identifier, sizeof(ino_t)); // Capture the first 8 bytes, the rest are zero

		// st_nlink
		FILE_STANDARD_INFO LINK_INFO;
		if (!GetFileInformationByHandleEx(file, FileStandardInfo, &LINK_INFO, sizeof(FILE_STANDARD_INFO)))
		{
			map_win32_error_to_wlibc(GetLastError());
			return -1;
		}
		statbuf->st_nlink = LINK_INFO.NumberOfLinks;

		// block info
		wchar_t root[4];
		root[0] = wbuf[4];
		root[1] = L':';
		root[2] = L'\\';
		root[3] = L'\0';

		DWORD sectors_per_cluster, bytes_per_sector, number_of_free_clusters, total_number_of_clusters;
		if (!GetDiskFreeSpace(root, &sectors_per_cluster, &bytes_per_sector, &number_of_free_clusters, &total_number_of_clusters))
		{
			map_win32_error_to_wlibc(GetLastError());
			return -1;
		}
		statbuf->st_blksize = sectors_per_cluster * bytes_per_sector;
		statbuf->st_blocks = (blkcnt_t)(LINK_INFO.AllocationSize.QuadPart / bytes_per_sector) +
							 ((blkcnt_t)(LINK_INFO.AllocationSize.QuadPart % bytes_per_sector) == 0 ? 0 : 1);

		// st_size
		if (statbuf->st_mode & S_IFLNK)
		{
			wchar_t symbuf[MAX_PATH];
			statbuf->st_size = common_readlink(wbuf, symbuf, MAX_PATH, 0);
			if (statbuf->st_size == -1)
			{
				statbuf->st_size = 0; // unresolved symlink
			}
		}
		else if (statbuf->st_mode & S_IFREG)
		{
			LARGE_INTEGER FILE_SIZE;
			GetFileSizeEx(file, &FILE_SIZE);
			statbuf->st_size = FILE_SIZE.QuadPart;
		}
		else
		{
			statbuf->st_size = statbuf->st_blksize;
		}
	}

	else
	{
		// Default values
		struct timespec time_data = {0, 0};
		statbuf->st_atim = time_data;
		statbuf->st_ctim = time_data;
		statbuf->st_mtim = time_data;

		statbuf->st_size = 0;
		statbuf->st_ino = 0;
		statbuf->st_nlink = 1;
		statbuf->st_blksize = 4096; // default
		statbuf->st_blocks = 0;

		statbuf->st_dev = 0;

		if (type == FILE_TYPE_PIPE)
		{
			statbuf->st_mode = S_IFIFO;
			statbuf->st_rdev = 0;
		}
		else if (type == FILE_TYPE_CHAR)
		{
			statbuf->st_mode = S_IFCHR;
			statbuf->st_rdev = 1;
		}
		else
		{
			// Something bad happened, program flow should never reach here
			errno = EINVAL;
			return -1;
		}
	}

	// Set these two to zero
	statbuf->st_uid = 0;
	statbuf->st_gid = 0;

	return 0;
}

int common_stat(const wchar_t *restrict wname, struct stat *restrict statbuf, int do_lstat)
{
	if (statbuf == NULL)
	{
		errno = EFAULT;
		return -1;
	}

	int lstat_flags = 0;

	if (do_lstat)
	{
		lstat_flags |= FILE_FLAG_OPEN_REPARSE_POINT;
	}

	wchar_t *wname_proper = NULL;
	if (wcscmp(wname, L"/dev/null") == 0)
	{
		wchar_t *wname_proper = L"NUL";
	}
	else if (wcscmp(wname, L"/dev/tty") == 0)
	{
		wchar_t *wname_proper = L"CON";
	}
	else
	{
		wname_proper = (wchar_t *)wname;
	}

	HANDLE file = CreateFile(wname_proper, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
							 OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | lstat_flags, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	int result = do_stat(file, statbuf);

	CloseHandle(file);

	return result;
}

int wlibc_stat(const char *restrict name, struct stat *restrict statbuf)
{
	if (name == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wname = mb_to_wc(name);
	int status = common_stat(wname, statbuf, 0);
	free(wname);

	return status;
}

int wlibc_wstat(const wchar_t *restrict wname, struct stat *restrict statbuf)
{
	if (wname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_stat(wname, statbuf, 0);
}

int wlibc_lstat(const char *restrict name, struct stat *restrict statbuf)
{
	if (name == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wname = mb_to_wc(name);
	int status = common_stat(wname, statbuf, 1);
	free(wname);

	return status;
}

int wlibc_wlstat(const wchar_t *restrict wname, struct stat *restrict statbuf)
{
	if (wname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_stat(wname, statbuf, 1);
}

int wlibc_fstat(int fd, struct stat *statbuf)
{

	if (!validate_fd(fd))
	{
		errno = EBADF;
		return -1;
	}

	// Hack for pipes
	if (get_fd_type(fd) == PIPE_HANDLE)
	{
		// Fill statbuf here itself and return
		struct timespec time_data = {0, 0};
		statbuf->st_atim = time_data;
		statbuf->st_ctim = time_data;
		statbuf->st_mtim = time_data;

		statbuf->st_dev = 0;
		statbuf->st_rdev = 0;

		statbuf->st_size = 0;
		statbuf->st_ino = 0;
		statbuf->st_nlink = 1;
		statbuf->st_blksize = 4096;
		statbuf->st_blocks = 0;

		statbuf->st_mode = S_IFIFO;

		statbuf->st_uid = 0;
		statbuf->st_gid = 0;

		return 0;
	}

	HANDLE file = get_fd_handle(fd);
	return do_stat(file, statbuf);
}
#endif

int do_stat(HANDLE handle, struct stat *restrict statbuf)
{
	IO_STATUS_BLOCK I;
	NTSTATUS status;

	FILE_FS_DEVICE_INFORMATION device_info;
	status = NtQueryVolumeInformationFile(handle, &I, &device_info, sizeof(FILE_FS_DEVICE_INFORMATION), FileFsDeviceInformation);
	// This is important
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	memset(statbuf, 0, sizeof(struct stat));

	DEVICE_TYPE type = device_info.DeviceType;
	if (type == FILE_DEVICE_DISK)
	{
		FILE_STAT_INFORMATION stat_info;
		status = NtQueryInformationFile(handle, &I, &stat_info, sizeof(FILE_STAT_INFORMATION), FileStatInformation);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		DWORD attributes = stat_info.FileAttributes;
		// From readdir.c
		if (attributes & FILE_ATTRIBUTE_REPARSE_POINT)
		{
			ULONG reparse_tag = stat_info.ReparseTag;
			if (reparse_tag == IO_REPARSE_TAG_SYMLINK || reparse_tag == IO_REPARSE_TAG_MOUNT_POINT)
			{
				statbuf->st_mode = S_IFLNK;
				statbuf->st_mode |= S_IREAD | S_IWRITE | S_IEXEC;
			}
			// if(reparse_tag == IO_REPARSE_TAG_AF_UNIX)
			//{
			//	statbuf->st_mode = S_IFSOCK;
			//	statbuf->st_mode |= S_IREAD | S_IWRITE | S_IEXEC;
			//}
		}
		else if (attributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			statbuf->st_mode = S_IFDIR;
			statbuf->st_mode |= S_IREAD | S_IWRITE | S_IEXEC;
		}
		else if ((attributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE |
								 FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_SPARSE_FILE | FILE_ATTRIBUTE_COMPRESSED |
								 FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_ATTRIBUTE_ENCRYPTED)) == 0)
		{
			statbuf->st_mode = S_IFREG;
			if (attributes & FILE_ATTRIBUTE_READONLY)
			{
				statbuf->st_mode |= S_IREAD;
			}
			else
			{
				statbuf->st_mode |= S_IREAD | S_IWRITE;
			}
		}

		//// Set the S_IEXEC bit here as it requires the name
		// if (has_executable_extenstion(wbuf + 4))
		//{
		//	statbuf->st_mode |= S_IEXEC;
		//}

		statbuf->st_ino = stat_info.FileId.QuadPart;
		statbuf->st_nlink = stat_info.NumberOfLinks;
		statbuf->st_size = stat_info.EndOfFile.QuadPart;

		// st_[amc]tim
		statbuf->st_atim = LARGE_INTEGER_to_timespec(stat_info.LastAccessTime);
		statbuf->st_mtim = LARGE_INTEGER_to_timespec(stat_info.LastWriteTime);
		statbuf->st_ctim = LARGE_INTEGER_to_timespec(stat_info.CreationTime);

		if (statbuf->st_mode & S_IFLNK)
		{
			statbuf->st_size = -1;
			PREPARSE_DATA_BUFFER reparse_buffer = (PREPARSE_DATA_BUFFER)malloc(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
			status = NtFsControlFile(handle, NULL, NULL, NULL, &I, FSCTL_GET_REPARSE_POINT, NULL, 0, reparse_buffer,
									 MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
			if (status != STATUS_SUCCESS)
			{
				// don't return just set errno
				map_ntstatus_to_errno(status);
			}

			if (reparse_buffer->ReparseTag == IO_REPARSE_TAG_SYMLINK)
			{
				if (reparse_buffer->SymbolicLinkReparseBuffer.PrintNameLength != 0)
				{
					statbuf->st_size = reparse_buffer->SymbolicLinkReparseBuffer.PrintNameLength / sizeof(WCHAR);
				}
				else if (reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameLength != 0)
				{
					statbuf->st_size = reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
				}
			}

			if (reparse_buffer->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
			{
				if (reparse_buffer->MountPointReparseBuffer.PrintNameLength != 0)
				{
					statbuf->st_size = reparse_buffer->MountPointReparseBuffer.PrintNameLength / sizeof(WCHAR);
				}
				else if (reparse_buffer->MountPointReparseBuffer.SubstituteNameLength != 0)
				{
					statbuf->st_size = reparse_buffer->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
				}
			}
			free(reparse_buffer);
		}

		FILE_FS_SIZE_INFORMATION size_info;
		status = NtQueryVolumeInformationFile(handle, &I, &size_info, sizeof(FILE_FS_SIZE_INFORMATION), FileFsSizeInformation);
		if (status == STATUS_SUCCESS)
		{
			statbuf->st_blksize = size_info.BytesPerSector * size_info.SectorsPerAllocationUnit;
			if (statbuf->st_mode & S_IFDIR)
			{
				statbuf->st_size = statbuf->st_blksize;
			}
			statbuf->st_blocks = statbuf->st_size / statbuf->st_blksize + (statbuf->st_size % statbuf->st_blksize == 0 ? 0 : 1);
		}
		else
		{
			// just set errno
			map_ntstatus_to_errno(status);
		}

		PFILE_FS_VOLUME_INFORMATION volume_info = (PFILE_FS_VOLUME_INFORMATION)malloc(128); // Max label length is 32(WCHAR) or 64 bytes
		status = NtQueryVolumeInformationFile(handle, &I, volume_info, 1024, FileFsVolumeInformation);
		if (status == STATUS_SUCCESS)
		{
			statbuf->st_dev = volume_info->VolumeSerialNumber;
		}
		else
		{
			// just set errno
			map_ntstatus_to_errno(status);
		}

		free(volume_info);
	}
	else if (type == FILE_DEVICE_NULL || type == FILE_DEVICE_CONSOLE)
	{
		statbuf->st_mode = S_IFCHR;
		statbuf->st_nlink = 1;
		// To differentiate between NUL and CON use st_dev and st_rdev.
		// We don't use st_ino because they have no meaning as these files are devices in the object manager.
		if (type == FILE_DEVICE_NULL)
		{
			statbuf->st_rdev = 1;
			statbuf->st_dev = 1;
		}
		else // (type == FILE_DEVICE_CONSOLE)
		{
			statbuf->st_rdev = 2;
			statbuf->st_dev = 2;
		}
	}
	else if (type == FILE_DEVICE_NAMED_PIPE)
	{
		statbuf->st_mode = S_IFIFO;
		statbuf->st_rdev = 0;
		statbuf->st_nlink = 1;
		statbuf->st_dev = 3;
	}

	return 0;
}

int common_stat(int dirfd, const char *restrict name, struct stat *restrict statbuf, int flags)
{
	wchar_t *u16_ntpath = get_absolute_ntpath(dirfd, name);
	if (u16_ntpath == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	HANDLE handle = just_open(u16_ntpath, FILE_READ_ATTRIBUTES | READ_CONTROL, 0, FILE_OPEN,
							  flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno will be set by just_open
		return -1;
	}

	int result = do_stat(handle, statbuf);
	NtClose(handle);
	return result;
}

int wlibc_common_stat(int dirfd, const char *restrict name, struct stat *restrict statbuf, int flags)
{
	if (statbuf == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if (flags != 0 && flags != AT_SYMLINK_NOFOLLOW && flags != AT_EMPTY_PATH)
	{
		errno = EINVAL;
		return -1;
	}

	if (flags != AT_EMPTY_PATH)
	{
		VALIDATE_PATH_AND_DIRFD(name, dirfd);
	}
	else
	{
		if (!validate_fd(dirfd))
		{
			errno = EBADF;
			return -1;
		}

		return do_stat(get_fd_handle(dirfd), statbuf);
	}

	return common_stat(dirfd, name, statbuf, flags);
}
