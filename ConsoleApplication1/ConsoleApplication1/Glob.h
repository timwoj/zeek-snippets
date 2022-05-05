#pragma once

// The code below is heavily based on the implementation of glob() from UNIXem.
// The header and license for the original file is included below. The original
// code is available from https://github.com/synesissoftware/UNIXem.
//
// The changes that were made to this file:
// - Changed all macros to be constexpr ints instead of #defined values.
// - Removed unixem_ from the front of names for the sake of easier inclusion
//   of these methods in other code without having to resort to remapping the
//   names with macros.
// - Removed #ifdef check for __COMO__ from argument list in glob() definition.
// - Converted to doxygen-style comments.

/* /////////////////////////////////////////////////////////////////////////
 * File:    glob.c
 *
 * Purpose: Definition of the glob() API functions for the Win32 platform.
 *
 * Created: 13th November 2002
 * Updated: 10th January 2017
 *
 * Home:    http://synesis.com.au/software/
 *
 * Copyright (c) 2002-2017, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Software nor the
 *   names of any contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////// */

/* Result codes */
constexpr auto GLOB_SUCCESS = (0)                 /*!< (Result code:) The operation completed successfully. */;
constexpr auto GLOB_NOSPACE = (1)                 /*!< (Result code:) An attempt to allocate memory failed, or if errno was 0 GLOB_LIMIT was specified in the flags and ARG_MAX patterns were matched. */;
constexpr auto GLOB_ABORTED = (2)                 /*!< (Result code:) The scan was stopped because an error was encountered and either GLOB_ERR was set or (*errfunc)() returned non-zero. */;
constexpr auto GLOB_NOMATCH = (3)                 /*!< (Result code:) The pattern does not match any existing pathname, and GLOB_NOCHECK was not set int flags. */;

/* Flags */
// The following flags map directly to the POSIX implementation of glob().
constexpr auto GLOB_ERR      = 0x00000001         /*!< Return on read failures. */;
constexpr auto GLOB_MARK     = 0x00000002         /*!< Append a slash to each name. */;
constexpr auto GLOB_NOSORT   = 0x00000004         /*!< Don't sort the names. */;
constexpr auto GLOB_DOOFFS   = 0x00000008         /*!< Insert PGLOB->gl_offs NULLs. Supported from version 1.6 of UNIXem. */;
constexpr auto GLOB_NOCHECK  = 0x00000010         /*!< If nothing matches, return the pattern. Supported from version 1.6 of UNIXem. */;
constexpr auto GLOB_APPEND   = 0x00000020         /*!< Append to results of a previous call. Not currently supported in this implementation. */;
constexpr auto GLOB_NOESCAPE = 0x00000040         /*!< Backslashes don't quote metacharacters. Has no effect in this implementation, since escaping is not supported. */;

// The following flags are custom to UNIXem's implementation only.
constexpr auto GLOB_PERIOD      = 0x00000080      /*!< Leading '.' can be matched by metachars. Supported from version 1.6 of UNIXem. */;
constexpr auto GLOB_MAGCHAR     = 0x00000100      /*!< Set in gl_flags if any metachars seen. Supported from version 1.6 of UNIXem. */;
//constexpr auto GLOB_ALTDIRFUNC  = 0x00000200      /*!< Use gl_opendir et al functions. Not currently supported in this implementation. */;
//constexpr auto GLOB_BRACE       = 0x00000400      /*!< Expand "{a,b}" to "a" "b". Not currently supported in this implementation. */;
constexpr auto GLOB_NOMAGIC     = 0x00000800      /*!< If no magic chars, return the pattern. Supported from version 1.6 of UNIXem. */;
constexpr auto GLOB_TILDE       = 0x00001000      /*!< Expand ~user and ~ to home directories. Partially supported from version 1.6 of UNIXem: leading ~ is expanded to %HOMEDRIVE%%HOMEPATH%. */;
constexpr auto GLOB_ONLYDIR     = 0x00002000      /*!< Match only directories. This implementation guarantees to only return directories when this flag is specified. */;
constexpr auto GLOB_TILDE_CHECK = 0x00004000      /*!< Like GLOB_TILDE but return an GLOB_NOMATCH even if GLOB_NOCHECK specified. Supported from version 1.6 of UNIXem. */;
constexpr auto GLOB_ONLYREG     = 0x00008000      /*!< Match only regular files. Supported from version 1.6 of UNIXem. */;
constexpr auto GLOB_NODOTSDIRS  = 0x00010000      /*!< Elide "." and ".." directories from wildcard searches. Supported from version 1.6 of UNIXem. */;
constexpr auto GLOB_LIMIT       = 0x00020000      /*!< Limits the search to the number specified by the caller in gl_matchc. Supported from version 1.6 of UNIXem. */;

/** Result structure for glob()
 *
 * This structure is used by glob() to return the results of the
 * search.
 */
typedef struct
{
  int       gl_pathc;   /*!< count of total paths so far */
  int       gl_matchc;  /*!< count of paths matching pattern */
  int       gl_offs;    /*!< reserved at beginning of gl_pathv */
  int       gl_flags;   /*!< returned flags */
  char**    gl_pathv;   /*!< list of paths matching pattern */
} glob_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Generates pathnames matching a pattern
 *
 * This function is a pathname generator that implements the rules for
 * file name pattern matching used by the UNIX shell.
 *
 * @param pattern The pattern controlling the search
 * @param flags A combination of the <b>GLOB_*</b> flags
 * @param errfunc A function that is called each time part of the search processing fails
 * @param pglob Pointer to a glob_t structure to receive the search results
 * @return 0 on success, otherwise one of the <b>GLOB_*</b> result codes
 */
int glob(char const* pattern, int flags, int (*errfunc)(char const*, int), glob_t* pglob);

/**
 * Frees the results of a call to glob()
 *
 * This function releases any memory allocated in a call to glob().
 * It must always be called for a successful call to glob().
 *
 * @param pglob Pointer to a glob_t structure to receive the search results
 */
void glob_free(glob_t* pglob);

#ifdef __cplusplus
}
#endif /* __cplusplus */
