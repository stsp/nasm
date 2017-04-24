/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2017 The NASM Authors - All Rights Reserved
 *   See the file AUTHORS included with the NASM distribution for
 *   the specific copyright holders.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following
 *   conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ----------------------------------------------------------------------- */

/*
 * path.c - host operating system specific pathname manipulation functions
 */

#include "compiler.h"
#include "nasmlib.h"
#include "error.h"

#if defined(unix) || defined(__unix) || defined(__unix__)
# define separators "/"
# define cleandirend "/"
# define leaveonclean 1
# define curdir "."
#elif defined(__MSDOS__) || defined(__WINDOWS__) || \
    defined(__OS2__) || defined(_WIN16) || defined(_WIN32)
# define separators "/\\:"
# define cleandirend "/\\"
# define leaveonclean 2         /* Leave \\ at the start alone */
# define curdir "."
#elif defined(Macintosh)        /* MacOS classic? */
# define separators ":"
# define curdir ":"
# define cleandirend ":"
# define leaveonclean 0
# define leave_leading 1
#elif defined(__VMS)
/*
 * VMS filenames may have ;version at the end.  Assume we should count that
 * as part of the filename anyway.
 */
# define separators ":]"
# define curdir "[]"
#else
/* No idea what to do here, do nothing.  Feel free to add new ones. */
# define curdir ""
#endif

static inline bool ismatch(const char *charset, char ch)
{
    const char *p;

    for (p = charset; *p; p++) {
        if (ch == *p)
            return true;
    }

    return false;
}

static const char *first_filename_char(const char *path)
{
#ifdef separators
    const char *p = path + strlen(path);

    while (p > path) {
        if (!ismatch(separators, p[-1]))
            return p;
        p--;
    }

    return p;
#else
    return path;
#endif
}

/* Return the filename portion of a PATH as a new string */
char *nasm_basename(const char *path)
{
    return nasm_strdup(first_filename_char(path));
}

/* Return the directory name portion of a PATH as a new string */
char *nasm_dirname(const char *path)
{
    const char *p = first_filename_char(path);
    const char *p0 = p;
    (void)p0;                   /* Don't warn if unused */

    if (p == path)
        return nasm_strdup(curdir);

#ifdef cleandirend
    while (p > path+leaveonclean) {
        if (ismatch(cleandirend, p[-1]))
            break;
        p--;
    }
#endif

#ifdef leave_leading
    /* If the directory contained ONLY separators, leave as-is */
    if (p == path+leaveonclean)
        p = p0;
#endif

    return nasm_strndup(path, p-path);
}