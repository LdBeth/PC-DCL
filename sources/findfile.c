#ifndef _WIN32

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fnmatch.h>

#include "dcl.h"
#include "platform.h"
#include "findfile.h"
#include "dbglog.h"

typedef struct {
	DIR* dirStream;
	char szDir[_MAX_DIR];
	char szPattern[_MAX_FNAME + _MAX_EXT];
} ff_t;

/*
	https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/findfirst-functions
*/
intptr_t _findfirst(const char *szFileSpec, struct _finddata_t *fileinfo)
{
	char szDir[_MAX_DIR];
	char szFile[_MAX_FNAME];
	char szExt[_MAX_EXT];
	ff_t* hFF = NULL;

	if (szFileSpec == NULL || fileinfo == NULL || strlen(szFileSpec) >= _MAX_PATH) {
		errno = EINVAL;
		return -1;
	}

	_splitpath(szFileSpec, NULL, szDir, szFile, szExt);
	if (*szDir == 0)
		strcpy(szDir, ".");

	if ((hFF = (ff_t*)malloc(sizeof(ff_t))) == NULL) {
		errno = ENOMEM;
		return -1;
	}

	if ((hFF->dirStream = opendir(szDir)) != NULL) {
		DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<_findfirst> Opendir %s OK.", szDir);

		strcpy(hFF->szDir, szDir);
		strcpy(hFF->szPattern, szFile);
		strcat(hFF->szPattern, szExt);

		if (_findnext((intptr_t)hFF, fileinfo) != 0) {
			DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<_findfirst> Find next %s error %d.", szDir, errno);

			int findNextErrno = errno;

			if (closedir(hFF->dirStream) != 0)
				DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<_findfirst> closedir %d.", errno);

			free(hFF);
			
			errno = findNextErrno;
			return -1;
		}
	} else {
		DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<_findfirst> Opendir %s error %d.", szDir, errno);

		free(hFF);
		errno = EINVAL;
		return -1;
	}

	return (intptr_t)hFF;
}

/*
	https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/findnext-functions
*/
int _findnext(intptr_t handle, struct _finddata_t *fileinfo)
{
	if (handle == -1) {
		errno = EINVAL;
		return -1;
	}

	const ff_t* const hFF = (const ff_t* const)handle;
	struct dirent* dirEnt;
	struct stat fileAttributes;
	char szPath[_MAX_PATH];

	if (hFF == NULL || fileinfo == NULL) {
		errno = EINVAL;
		return -1;
	}

	errno = 0;
	while ((dirEnt = readdir(hFF->dirStream)) != NULL) {
		DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<_findnext> readdir OK pattern=%s name=%s", hFF->szPattern, dirEnt->d_name);

		if (fnmatch(hFF->szPattern, dirEnt->d_name, 0) == 0) {
			_makepath(szPath, NULL, hFF->szDir, dirEnt->d_name, NULL);
			if (stat(szPath, &fileAttributes) == 0) {
					DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<_findnext> stat %s ok attr=0x%X", szPath, fileAttributes.st_mode);

					fileinfo->attrib = _A_NORMAL;
					if (S_ISDIR(fileAttributes.st_mode)) fileinfo->attrib |= _A_SUBDIR;
					if (!(fileAttributes.st_mode & S_IWUSR)) fileinfo->attrib |= _A_RDONLY;
					fileinfo->time_create = fileAttributes.st_ctime;
					fileinfo->time_access = fileAttributes.st_atime;
					fileinfo->time_write = fileAttributes.st_mtime;
					fileinfo->size = fileAttributes.st_size;

					memset(fileinfo->name, 0, _MAX_PATH);
					strncpy(fileinfo->name, dirEnt->d_name, strlen(dirEnt->d_name));

					return 0;
			} else {
					DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<_findnext> stat %s error %d.", szPath, errno);

					if (errno == ENAMETOOLONG)
						errno = ENOMEM;
					else if (errno != ENOENT)
						errno = EINVAL;
					
					return -1;
			}
		}
	}

	if (errno) {
		DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<Dcl_FindNextFile> readdir error %d", errno);

		errno = ENOENT;
		return -1;
	}

	// No match found.

	errno = ENOENT;
	return -1;
}

/*
	https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/findclose
*/
int _findclose(intptr_t handle)
{
	if (handle == -1) {
		errno = ENOENT;
		return -1;
	}

	ff_t* hFF = (ff_t*)handle;
	if (hFF == NULL) {
		errno = ENOENT;
		return -1;
	}

	if (closedir(hFF->dirStream) != 0) {
		DebugLogV(DEBUGLOG_LEVEL_DEBUG, "<_findclose> closedir %d.", errno);
		errno = ENOENT;
		free(hFF);
		return -1;
	}

	free(hFF);
	return 0;
}

#endif
