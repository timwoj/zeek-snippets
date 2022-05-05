// The code below is heavily based on the implementation of glob() from UNIXem.
// The header and license for the original file is included below. The original
// code is available from https://github.com/synesissoftware/UNIXem.
//
// The changes that were made to this file:
// - Removed unixem_ from the front of names for the sake of easier inclusion
//   of these methods in other code without having to resort to remapping the
//   names with macros.
// - Removed #ifdef check for __COMO__ from argument list in glob() definition.
// - Lots of cleanup to C++-ify things a little bit and to fix warnings from
//   Resharper.

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

#include "Glob.h"

#include <cassert>
#include <cstddef>
#include <cstring>

#include <Windows.h>

/**
 * Reverse strpbrk. This searches a string for the last occurance of any character from
 * the character set and returns a pointer to that character.
 */
static char const* strrpbrk(char const* string, char const* strCharSet)
{
    char*       part = NULL;
    char const* pch = NULL;

    for(pch = strCharSet; *pch; ++pch)
    {
        char* p = const_cast<char*>(strrchr(string, *pch));

        if(NULL != p)
        {
            if(NULL == part || (part < p))
                part = p;
        }
    }

    return part;
}

/**
 * Checks whether a string is exactly '.' or '..'
 */
static bool isdots(char const* s)
{
    if('.' == s[0])
    {
        if('\0' == s[1])
            return true;
        if('.' == s[1] && '\0' == s[2])
            return true;
    }

    return false;
}

/**
 * It gives you back the matched contents of your pattern, so for Win32, the
 * directories must be included
 */
int glob(char const* pattern, int flags, int (*errfunc)(char const *, int), glob_t* pglob)
{
    assert(NULL != pglob);

    int result = GLOB_SUCCESS;
    char szRelative[1 + MAX_PATH]{};
    char szPattern3[1 + MAX_PATH]{};
    char const* effectivePattern = pattern;
    char const* leafMost;

    bool bMagic = (NULL != strpbrk(pattern, "?*"));
    bool bNoMagic = false;
    if(flags & GLOB_NOMAGIC)
        bNoMagic = ! bMagic;

    size_t maxMatches = 0;
    if(flags & GLOB_LIMIT)
        maxMatches = (size_t)pglob->gl_matchc;

    if(flags & GLOB_TILDE)
    {
        /* Check that begins with "~/" */
        if( '~' == pattern[0] &&
            (   '\0' == pattern[1] ||
                '/' == pattern[1] ||
                '\\' == pattern[1]))
        {
            char szPattern2[1 + MAX_PATH]{};
            (void)lstrcpyA(&szPattern2[0], "%HOMEDRIVE%%HOMEPATH%");

            if(DWORD dw = ExpandEnvironmentStringsA(&szPattern2[0], &szPattern3[0], sizeof(szPattern3) - 1); 0 != dw)
            {
                (void)lstrcpynA(&szPattern3[0] + dw - 1, &pattern[1], (int)(sizeof(szPattern3) - dw));
                szPattern3[sizeof(szPattern3) - 1] = '\0';

                effectivePattern = szPattern3;
            }
        }
    }

    char const* file_part = strrpbrk(effectivePattern, "\\/");

    if(NULL != file_part)
    {
        leafMost = ++file_part;

        (void)lstrcpyA(szRelative, effectivePattern);
        szRelative[file_part - effectivePattern] = '\0';
    }
    else
    {
        szRelative[0] = '\0';
        leafMost = effectivePattern;
    }

    bool bMagic0 = (leafMost == strpbrk(leafMost, "?*"));
    bool bLeafIsDots = isdots(leafMost);

    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(effectivePattern, &find_data);
    char* buffer = NULL;

    pglob->gl_pathc = 0;
    pglob->gl_pathv = NULL;

    if(0 == (flags & GLOB_DOOFFS))
        pglob->gl_offs = 0;

    if(hFind == INVALID_HANDLE_VALUE)
    {
        // If this was a pattern search, and the directory exists, then we return 0
        // matches, rather than GLOB_NOMATCH
        if( bMagic && NULL != file_part)
        {
            result = GLOB_SUCCESS;
        }
        else
        {
            if(NULL != errfunc)
                (void)errfunc(effectivePattern, (int)GetLastError());

            result = GLOB_NOMATCH;
        }
    }
    else
    {
        int     cbCurr      =   0;
        size_t  cbAlloc     =   0;
        size_t  cMatches    =   0;

        result = GLOB_SUCCESS;

        do
        {
            if( bMagic0 && 0 == (flags & GLOB_PERIOD) && '.' == find_data.cFileName[0])
                continue;

            if(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                /* Pattern must begin with '.' to match either dots directory */
                if( bMagic0 && GLOB_NODOTSDIRS == (flags & GLOB_NODOTSDIRS) && isdots(find_data.cFileName))
                    continue;

                if(flags & GLOB_MARK)
                    (void)lstrcatA(find_data.cFileName, "/");
            }
            else
            {
                if(flags & GLOB_ONLYDIR)
                    /* Skip all further actions, and get the next entry */
                    continue;
            }

			if(bLeafIsDots)
			{
				(void)lstrcpyA(find_data.cFileName, leafMost);

                if(flags & GLOB_MARK)
                    (void)lstrcatA(find_data.cFileName, "/");
			}

            int cch = lstrlenA(find_data.cFileName);
            if(NULL != file_part)
                cch += (int)(file_part - effectivePattern);

            if(size_t new_cbAlloc = (size_t)cbCurr + cch + 1; new_cbAlloc > cbAlloc)
            {
                new_cbAlloc *= 2;

                new_cbAlloc = (new_cbAlloc + 31) & ~(31);

                char* new_buffer = (char*)realloc(buffer, new_cbAlloc);

                if(new_buffer == NULL)
                {
                    result = GLOB_NOSPACE;
                    free(buffer);
                    buffer = NULL;
                    break;
                }

                buffer = new_buffer;
                cbAlloc = new_cbAlloc;
            }

            if (buffer + cbCurr != NULL)
            {
                (void)lstrcpynA(buffer + cbCurr, szRelative, 1 + (int)(file_part - effectivePattern));
                (void)lstrcatA(buffer + cbCurr, find_data.cFileName);
            }

            cbCurr += cch + 1;

            ++cMatches;
        }
        while(FindNextFileA(hFind, &find_data) && cMatches != maxMatches);

        (void)FindClose(hFind);

        if(result == 0)
        {
            /* Now expand the buffer, to fit in all the pointers. */
            size_t  cbPointers  =   (1 + cMatches + pglob->gl_offs) * sizeof(char*);
            char*   new_buffer  =   (char*)realloc(buffer, cbAlloc + cbPointers);

            if(new_buffer == NULL)
            {
                result = GLOB_NOSPACE;
                free(buffer);
            }
            else
            {
                char*   next_str;

                buffer = new_buffer;

                (void)memmove(new_buffer + cbPointers, new_buffer, cbAlloc);

                /* Handle the offsets. */
                char** begin = (char**)new_buffer;
                char** end = begin + pglob->gl_offs;

                for(; begin != end; ++begin)
                    *begin = NULL;

                /* Sort, or no sort. */
                char** pp = (char**)new_buffer + pglob->gl_offs;
                begin =   pp;
                end   =   begin + cMatches;

                if(flags & GLOB_NOSORT)
                {
                    /* The way we need in order to test the removal of dots in the findfile_sequence. */
                    *end = NULL;
                    for(begin = pp, next_str = buffer + cbPointers; begin != end; --end)
                    {
                        *(end - 1) = next_str;

                        /* Find the next string. */
                        next_str += 1 + lstrlenA(next_str);
                    }
                }
                else
                {
                    /* The normal way. */
                    for(begin = pp, next_str = buffer + cbPointers; begin != end; ++begin)
                    {
                        *begin = next_str;

                        /* Find the next string. */
                        next_str += 1 + lstrlenA(next_str);
                    }
                    *begin = NULL;
                }

                /* Return results to caller. */
                pglob->gl_pathc =   (int)cMatches;
                pglob->gl_matchc=   (int)cMatches;
                pglob->gl_flags =   0;
                if(bMagic)
                    pglob->gl_flags |= GLOB_MAGCHAR;

                pglob->gl_pathv =   (char**)new_buffer;
            }
        }

        if(0 == cMatches)
        {
            result = GLOB_NOMATCH;
        }
    }

    if(GLOB_NOMATCH == result)
    {
        if( (flags & GLOB_TILDE_CHECK) && effectivePattern == szPattern3)
        {
            result = GLOB_NOMATCH;
        }
        else if(bNoMagic ||
                (flags & GLOB_NOCHECK))
        {
            const size_t    effPattLen  =   strlen(effectivePattern);
            const size_t    cbNeeded    =   ((2 + pglob->gl_offs) * sizeof(char*)) + (1 + effPattLen);
            char**          pp          =   (char**)realloc(buffer, cbNeeded);

            if(NULL == pp)
            {
                result = GLOB_NOSPACE;
                free(buffer);
            }
            else
            {
                /* Handle the offsets. */
                char**  begin   =   pp;
                char**  end     =   pp + pglob->gl_offs;
                char*   dest    =   (char*)(pp + 2 + pglob->gl_offs);

                for(; begin != end; ++begin)
                {
                    *begin = NULL;
                }

                /* Synthesise the pattern result. */
                strcpy_s(dest, effPattLen + 1, effectivePattern);
                pp[0 + pglob->gl_offs] = dest;
                pp[1 + pglob->gl_offs] = NULL;

                /* Return results to caller. */
                pglob->gl_pathc =   1;
                pglob->gl_matchc=   1;
                pglob->gl_flags =   0;
                if(bMagic)
                    pglob->gl_flags |= GLOB_MAGCHAR;

                pglob->gl_pathv =   pp;

                result = 0;
            }
        }
    }
    else if(GLOB_SUCCESS == result)
    {
        if((size_t)pglob->gl_matchc == maxMatches)
            result = GLOB_NOSPACE;
    }

    return result;
}

void globfree(glob_t* pglob)
{
    if(pglob != NULL)
    {
        free(pglob->gl_pathv);
        pglob->gl_pathc = 0;
        pglob->gl_pathv = NULL;
    }
}