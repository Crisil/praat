#ifndef _melder_h_
#define _melder_h_
/* melder.h
 *
 * Copyright (C) 1992-2012,2013,2014,2015 Paul Boersma
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
	#define strequ  ! strcmp
	#define strnequ  ! strncmp
	#define wcsequ  ! wcscmp
	#define wcsnequ  ! wcsncmp
	#define Melder_strequ  ! Melder_strcmp
	#define Melder_strnequ  ! Melder_strncmp
	#define Melder_wcsequ  ! Melder_wcscmp
	#define Melder_wcsnequ  ! Melder_wcsncmp
	#define Melder_str32equ  ! Melder_str32cmp
	#define Melder_str32nequ  ! Melder_str32ncmp
#include <stdarg.h>
#include <stddef.h>
#include <wchar.h>
#ifdef __MINGW32__
	#undef swprintf
	#define swprintf  _snwprintf
	//#define swprintf  __mingw_snwprintf
	#include <sys/types.h>   // for off_t
#endif
#include <stdbool.h>
/*
 * The following two lines are for obsolete (i.e. C99) versions of stdint.h
 */
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#include <stdint.h>
#ifndef INT54_MAX
	#define INT54_MAX   9007199254740991LL
	#define INT54_MIN  -9007199254740991LL
#endif

typedef wchar_t wchar;
typedef unsigned char char8_t;
typedef char32_t char32;
typedef char16_t char16;
typedef char8_t char8;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

bool Melder_wcsequ_firstCharacterCaseInsensitive (const wchar_t *string1, const wchar_t *string2);
bool Melder_str32equ_firstCharacterCaseInsensitive (const char32 *string1, const char32 *string2);

#include "enums.h"

#include "melder_enums.h"

#ifndef TRUE
	#define TRUE  1
#endif
#ifndef FALSE
	#define FALSE  0
#endif
#ifndef NULL
	#define NULL  ((void *) 0)
#endif

/*
 * Debugging.
 */
int Melder_fatal (const char *format, ...);
	/* Give error message, abort program. */
	/* Should only be caused by programming errors. */

int Melder_assert_ (const char *condition, const char *fileName, int lineNumber);
	/* Call Melder_fatal with a message based on the following template: */
	/*    "Assertion failed in file <fileName> on line <lineNumber>: <condition>" */

void Melder_setTracing (bool tracing);
extern bool Melder_isTracing;

/*
 * char8 handling
 */
#define U8  (const char8 *)
inline static int64_t str8len (const char8 *string) {
	return (int64_t) strlen ((const char *) string);
}
inline static char8 * str8chr (const char8 *string, int kar) {
	return (char8 *) strchr ((const char *) string, kar);
}
inline static char8 * str8str (const char8 *string, const char8 *find) {
	return (char8 *) strstr ((const char *) string, (const char *) find);
}
inline static char8 * str8cat (char8 *string1, const char8 *string2) {
	return (char8 *) strcat ((char *) string1, (const char *) string2);
}
inline static int str8ncmp (const char8 *string1, const char8 *string2, int64_t n) {
	return strncmp ((const char *) string1, (const char *) string2, (size_t) n);
}
#define str8nequ  ! str8ncmp
/*
 * char16 handling.
 */
inline static int64_t str16len (const char16_t *string) {
	if (sizeof (wchar_t) == 2) {
		return (int64_t) wcslen ((const wchar_t *) string);
	} else {
		const char16_t *p = string;
		while (*p != u'\0') ++ p;
		return (int64_t) (p - string);
	}
}
inline static char16_t * str16cpy (char16_t *target, const char16_t *source) {
	if (sizeof (wchar_t) == 2) {
		return (char16_t *) wcscpy ((wchar_t *) target, (const wchar_t *) source);
	} else {
		char16_t *p = target;
		while (* source != u'\0') * p ++ = * source ++;
		*p = u'\0';
		return target;
	}
}
/*
 * char32 handling.
 */
inline static int64 str32len (const char32 *string) {
	if (sizeof (wchar_t) == 4) {
		return (int64) wcslen ((const wchar_t *) string);
	} else {
		const char32 *p = string;
		while (*p != U'\0') ++ p;
		return (int64) (p - string);
	}
}
inline static char32 * str32cpy (char32 *target, const char32 *source) {
	if (sizeof (wchar_t) == 4) {
		return (char32 *) wcscpy ((wchar_t *) target, (const wchar_t *) source);
	} else {
		char32 *p = target;
		while (* source != U'\0') * p ++ = * source ++;
		*p = U'\0';
		return target;
	}
}
inline static char32 * str32ncpy (char32 *target, const char32 *source, int64 n) {
	if (sizeof (wchar_t) == 4) {
		return (char32 *) wcsncpy ((wchar_t *) target, (const wchar_t *) source, (size_t) n);
	} else {
		char32 *p = target;
		for (; n > 0 && *source != U'\0'; -- n) * p ++ = * source ++;
		for (; n > 0; -- n) * p ++ = U'\0';
		return target;
	}
}
inline static int str32cmp (const char32 *string1, const char32 *string2) {
	if (sizeof (wchar_t) == 4) {
		return wcscmp ((const wchar_t *) string1, (const wchar_t *) string2);
	} else {
		for (;; ++ string1, ++ string2) {
			int32_t diff = (int32) *string1 - (int32) *string2;
			if (diff) return (int) diff;
			if (*string1 == U'\0') return 0;
		}
	}
}
#define str32equ  ! str32cmp
inline static int str32ncmp (const char32 *string1, const char32 *string2, int64 n) {
	if (sizeof (wchar_t) == 4) {
		return wcsncmp ((const wchar_t *) string1, (const wchar_t *) string2, (size_t) n);
	} else {
		for (; n > 0; -- n, ++ string1, ++ string2) {
			int32 diff = (int32) *string1 - (int32) *string2;
			if (diff) return (int) diff;
			if (*string1 == U'\0') return 0;
		}
		return 0;
	}
}
#define str32nequ  ! str32ncmp
inline static char32 * str32chr (const char32 *string, char32 kar) {
	if (sizeof (wchar_t) == 4) {
		return (char32 *) wcschr ((const wchar *) string, (wchar) kar);
	} else {
		for (; *string != kar; ++ string) {
			if (*string == U'\0')
				return NULL;
		}
		return (char32 *) string;
	}
}
inline static char32 * str32rchr (const char32 *string, char32 kar) {
	if (sizeof (wchar_t) == 4) {
		return (char32 *) wcsrchr ((const wchar *) string, (wchar) kar);
	} else {
		char32 *result = NULL;
		for (; *string != U'\0'; ++ string) {
			if (*string == kar) result = (char32 *) string;
		}
		return result;
	}
}
inline static char32 * str32str (const char32 *string, const char32 *find) {
	if (sizeof (wchar_t) == 4) {
		return (char32 *) wcsstr ((const wchar_t *) string, (const wchar_t *) find);
	} else {
		size_t length = str32len (find);
		if (length == 0) return (char32 *) string;
		char32 firstCharacter = * find ++;   // optimization
		do {
			char32 kar;
			do {
				kar = * string ++;
				if (kar == U'\0') return NULL;
			} while (kar != firstCharacter);
		} while (str32ncmp (string, find, length - 1));
		return (char32 *) (string - 1);
	}
}
extern "C" char * Melder_peekStr32ToUtf8 (const char32 *string);
inline static long a32tol (const char32 *string) {
	if (sizeof (wchar_t) == 4) {
		return wcstol ((const wchar_t *) string, NULL, 10);
	} else {
		return atol (Melder_peekStr32ToUtf8 (string));
	}
}

/*
 * Operating system version control.
 */
#define ALLOW_GDK_DRAWING  1
/* */

typedef struct { double red, green, blue, transparency; } double_rgbt;

/********** NUMBER TO STRING CONVERSION **********/

/**
	The following routines return a static string, chosen from a circularly used set of 11 buffers.
	You can call at most 11 of them in one Melder_casual call, for instance.
*/

const  char   * Melder8_integer  (int64_t value);   // the past text format: 8-bit characters
const wchar_t * MelderW_integer  (int64_t value);   // the current text format: "wide" characters
const  char32 * Melder32_integer (int64_t value);   // the future text format: 32-bit characters
#define Melder_integer  MelderW_integer

const  char   * Melder8_bigInteger  (int64_t value);
const wchar_t * MelderW_bigInteger  (int64_t value);
const  char32 * Melder32_bigInteger (int64_t value);
#define Melder_bigInteger  MelderW_bigInteger

const  char   * Melder8_boolean  (bool value);
const wchar_t * MelderW_boolean  (bool value);
const  char32 * Melder32_boolean (bool value);
#define Melder_boolean  MelderW_boolean
	// "yes" or "no"

/**
	Format a double value as "--undefined--" or something in the "%.15g", "%.16g", or "%.17g" formats.
*/
const  char   * Melder8_double  (double value);
const wchar_t * MelderW_double  (double value);
const  char32 * Melder32_double (double value);
#define Melder_double  MelderW_double

/**
	Format a double value as "--undefined--" or something in the "%.9g" format.
*/
const  char   * Melder8_single  (double value);
const wchar_t * MelderW_single  (double value);
const  char32 * Melder32_single (double value);
#define Melder_single  MelderW_single

/**
	Format a double value as "--undefined--" or something in the "%.4g" format.
*/
const  char   * Melder8_half  (double value);
const wchar_t * MelderW_half  (double value);
const  char32 * Melder32_half (double value);
#define Melder_half  MelderW_half

/**
	Format a double value as "--undefined--" or something in the "%.*f" format.
*/
const  char   * Melder8_fixed  (double value, int precision);
const wchar_t * MelderW_fixed  (double value, int precision);
const  char32 * Melder32_fixed (double value, int precision);
#define Melder_fixed  MelderW_fixed

/**
	Format a double value with a specified precision. If exponent is -2 and precision is 2, you get things like 67E-2 or 0.00024E-2.
*/
const  char   * Melder8_fixedExponent  (double value, int exponent, int precision);
const wchar_t * MelderW_fixedExponent  (double value, int exponent, int precision);
const  char32 * Melder32_fixedExponent (double value, int exponent, int precision);
#define Melder_fixedExponent  MelderW_fixedExponent

/**
	Format a double value as a percentage. If precision is 3, you get things like "0" or "34.400%" or "0.014%" or "0.001%" or "0.0000007%".
*/
const  char   * Melder8_percent  (double value, int precision);
const wchar_t * MelderW_percent  (double value, int precision);
const  char32 * Melder32_percent (double value, int precision);
#define Melder_percent  MelderW_percent

/**
	Convert a formatted floating-point string to something suitable for visualization with the Graphics library.
	For instance, "1e+4" is turned into "10^^4", and "-1.23456e-78" is turned into "-1.23456\.c10^^-78".
*/
const wchar_t * Melder_float (const wchar_t *number);

/**
	Format the number that is specified by its natural logarithm.
	For instance, -10000 is formatted as "1.135483865315339e-4343", which is a floating-point representation of exp(-10000).
*/
const  char   * Melder8_naturalLogarithm  (double lnNumber);
const wchar_t * MelderW_naturalLogarithm  (double lnNumber);
const  char32 * Melder32_naturalLogarithm (double lnNumber);
#define Melder_naturalLogarithm  MelderW_naturalLogarithm

/********** STRING TO NUMBER CONVERSION **********/

int Melder_isStringNumeric_nothrow (const wchar_t *string);
double Melder_a8tof (const char *string);
double Melder_atof (const wchar_t *string);
double Melder_a32tof (const char32 *string);
	/*
	 * "3.14e-3" -> 3.14e-3
	 * "15.6%" -> 0.156
	 * "fghfghj" -> NUMundefined
	 */

/********** CONSOLE **********/

void Melder_writeToConsole (const wchar_t *message, bool useStderr);

/********** MEMORY ALLOCATION ROUTINES **********/

/* These routines call malloc, free, realloc, and calloc. */
/* If out of memory, the non-f versions throw an error message (like "Out of memory"); */
/* the f versions open up a rainy day fund or crash Praat. */
/* These routines also maintain a count of the total number of blocks allocated. */

void Melder_alloc_init (void);   // to be called around program start-up
void Melder_message_init (void);   // to be called around program start-up
void * _Melder_malloc (int64_t size);
#define Melder_malloc(type,numberOfElements)  (type *) _Melder_malloc ((numberOfElements) * (int64_t) sizeof (type))
void * _Melder_malloc_f (int64_t size);
#define Melder_malloc_f(type,numberOfElements)  (type *) _Melder_malloc_f ((numberOfElements) * (int64_t) sizeof (type))
void * Melder_realloc (void *pointer, int64_t size);
void * Melder_realloc_f (void *pointer, int64_t size);
void * _Melder_calloc (int64_t numberOfElements, int64_t elementSize);
#define Melder_calloc(type,numberOfElements)  (type *) _Melder_calloc (numberOfElements, sizeof (type))
void * _Melder_calloc_f (int64_t numberOfElements, int64_t elementSize);
#define Melder_calloc_f(type,numberOfElements)  (type *) _Melder_calloc_f (numberOfElements, sizeof (type))
char * Melder_strdup (const char *string);
char * Melder_strdup_f (const char *string);
wchar_t * Melder_wcsdup (const wchar_t *string);
wchar_t * Melder_wcsdup_f (const wchar_t *string);
char16 * Melder_str16dup (const char16 *string);
char32 * Melder_str32dup (const char32 *string);
char32 * Melder_str32dup_f (const char32 *string);
int Melder_strcmp (const char *string1, const char *string2);   // regards null string as empty string
int Melder_wcscmp (const wchar_t *string1, const wchar_t *string2);   // regards null string as empty string
int Melder_str32cmp (const char32 *string1, const char32 *string2);   // regards null string as empty string
int Melder_strncmp (const char *string1, const char *string2, int64_t n);
int Melder_wcsncmp (const wchar_t *string1, const wchar_t *string2, int64_t n);
int Melder_str32ncmp (const char32 *string1, const char32 *string2, int64_t n);
wchar_t * Melder_wcstok (wchar_t *string, const wchar_t *delimiter, wchar_t **last);   // circumvents platforms where wcstok has only two arguments

/**
 * Text encodings.
 */
void Melder_textEncoding_prefs (void);
void Melder_setInputEncoding (enum kMelder_textInputEncoding encoding);
int Melder_getInputEncoding (void);
void Melder_setOutputEncoding (enum kMelder_textOutputEncoding encoding);
enum kMelder_textOutputEncoding Melder_getOutputEncoding (void);

/*
 * Some other encodings. Although not used in the above set/get functions,
 * these constants should stay separate from the above encoding constants
 * because they occur in the same fields of struct MelderFile.
 */
const uint32_t kMelder_textInputEncoding_FLAC = 0x464C4143;
const uint32_t kMelder_textOutputEncoding_ASCII = 0x41534349;
const uint32_t kMelder_textOutputEncoding_ISO_LATIN1 = 0x4C415401;
const uint32_t kMelder_textOutputEncoding_FLAC = 0x464C4143;

bool Melder_isValidAscii (const wchar_t *string);
bool Melder_isValidAscii (const char32 *string);
bool Melder_str8IsValidUtf8 (const char *string);
bool Melder_isEncodable (const wchar_t *string, int outputEncoding);
bool Melder_isEncodable (const char32 *string, int outputEncoding);
extern char32 Melder_decodeMacRoman [256];
extern char32 Melder_decodeWindowsLatin1 [256];

long Melder_killReturns_inlineW  (wchar_t *text);
long Melder_killReturns_inline32 (char32 *text);

size_t wcslen_utf8  (const wchar_t *wcs, bool expandNewlines);
size_t wcslen_utf16 (const wchar_t *wcs, bool expandNewlines);
size_t wcslen_char32 (const wchar_t *wcs, bool expandNewlines);
size_t str32len_utf8 (const char32 *string, bool expandNewlines);

void Melder_8bitToWcs_inline   (const char *source, wchar_t *target, int inputEncoding);
void Melder_8bitToChar32_inline (const char *source, char32 *target, int inputEncoding);
	// errors: Text is not valid UTF-8.
wchar_t * Melder_8bitToWcs (const char *string, int inputEncoding);
char32 * Melder_8bitToChar32 (const char *string, int inputEncoding);
	// errors: Out of memory; Text is not valid UTF-8.
wchar_t * Melder_utf8ToWcs (const char *string);
char32 * Melder_utf8ToChar32 (const char *string);
	// errors: Out of memory; Text is not valid UTF-8.

char32 * Melder_utf8ToChar32_f (const char *string);   // for use in string constants only
	// crashes: Out of memory; Text is not valid UTF-8.

void Melder_wcsToUtf8_inline (const wchar_t *wcs, char *utf8);
void Melder_str32ToUtf8_inline (const char32 *string, char *utf8);
char * Melder_wcsToUtf8 (const wchar_t *string);
char * Melder_str32ToUtf8 (const char32 *string);
	// errors: Out of memory.
void Melder_wcsTo8bitFileRepresentation_inline (const wchar_t *wcs, char *utf8);
void Melder_8bitFileRepresentationToWcs_inline (const char *utf8, wchar_t *wcs);
extern "C" wchar_t * Melder_peekUtf8ToWcs (const char *string);
extern "C" char * Melder_peekWcsToUtf8 (const wchar_t *string);
extern "C" char * Melder_peekStr32ToUtf8 (const char32 *string);
extern "C" const uint16_t * Melder_peekWcsToUtf16 (const wchar_t *string);   // char16_t is C++-only
const void * Melder_peekWcsToCfstring (const wchar_t *string);
const void * Melder_peekStr32ToCfstring (const char32 *string);
void Melder_fwriteWcsAsUtf8 (const wchar_t *ptr, size_t n, FILE *f);
void Melder_fwriteStr32AsUtf8 (const char32 *ptr, size_t n, FILE *f);

char16 * Melder_peekUtf32to16 (const char32 *text, bool nativizeNewlines);
char32 * Melder_peekUtf16to32 (const char16 *text);

char16 * Melder_str32to16 (const char32 *text);
char32 * Melder_str16to32 (const char16 *text);

inline static wchar_t * Melder_peekStr32ToWcs (const char32 *string) {
	if (sizeof (wchar_t) == 4) {
		return (wchar_t *) string;
	} else {
		return (wchar_t *) Melder_peekUtf32to16 (string, false);
	}
}
inline static wchar_t * Melder_str32ToWcs (const char32 *string) {
	if (sizeof (wchar_t) == 4) {
		return Melder_wcsdup ((const wchar_t *) string);
	} else {
		return (wchar_t *) Melder_str32to16 (string);
	}
}
inline static char32 * Melder_peekWcsToStr32 (const wchar_t *string) {
	if (sizeof (wchar_t) == 4) {
		return (char32 *) string;
	} else {
		return (char32 *) Melder_peekUtf16to32 ((const char16 *) string);
	}
}
inline static char32 * Melder_wcsToStr32 (const wchar_t *string) {
	if (sizeof (wchar_t) == 4) {
		return (char32 *) Melder_wcsdup (string);
	} else {
		return Melder_str16to32 ((const char16 *) string);
	}
}

/********** FILES **********/

#if defined (_WIN32)
	#define Melder_DIRECTORY_SEPARATOR  '\\'
#else
	#define Melder_DIRECTORY_SEPARATOR  '/'
#endif

struct FLAC__StreamDecoder;
struct FLAC__StreamEncoder;

#define kMelder_MAXPATH 1023   /* excluding the null byte */

struct structMelderFile {
	FILE *filePointer;
	wchar_t path [kMelder_MAXPATH+1];
	enum class Format { none = 0, binary = 1, text = 2 } format;
	bool openForReading, openForWriting, verbose, requiresCRLF;
	unsigned long outputEncoding;
	int indent;
	struct FLAC__StreamEncoder *flacEncoder;
};
typedef struct structMelderFile *MelderFile;

struct structMelderDir {
	wchar_t path [kMelder_MAXPATH+1];
};
typedef struct structMelderDir *MelderDir;

#if defined (macintosh) && useCarbon
	void Melder_machToFile (void *void_fsref, MelderFile file);
#endif

const wchar_t * MelderFile_name (MelderFile file);
wchar_t * MelderDir_name (MelderDir dir);
void Melder_pathToDir (const wchar_t *path, MelderDir dir);
void Melder_pathToFile (const wchar_t *path, MelderFile file);
inline static void Melder_pathToFile (const char32 *path, MelderFile file) { Melder_pathToFile (Melder_peekStr32ToWcs (path), file); }
void Melder_relativePathToFile (const wchar_t *path, MelderFile file);
inline static void Melder_relativePathToFile (const char32 *path, MelderFile file) { Melder_relativePathToFile (Melder_peekStr32ToWcs (path), file); }
wchar_t * Melder_dirToPath (MelderDir dir);
	/* Returns a pointer internal to 'dir', like "/u/paul/praats" or "D:\Paul\Praats" */
wchar_t * Melder_fileToPath (MelderFile file);
void MelderFile_copy (MelderFile file, MelderFile copy);
void MelderDir_copy (MelderDir dir, MelderDir copy);
bool MelderFile_equal (MelderFile file1, MelderFile file2);
bool MelderDir_equal (MelderDir dir1, MelderDir dir2);
void MelderFile_setToNull (MelderFile file);
bool MelderFile_isNull (MelderFile file);
void MelderDir_setToNull (MelderDir dir);
bool MelderDir_isNull (MelderDir dir);
void MelderDir_getFile (MelderDir parent, const wchar_t *fileName, MelderFile file);
void MelderDir_relativePathToFile (MelderDir dir, const wchar_t *path, MelderFile file);
void MelderFile_getParentDir (MelderFile file, MelderDir parent);
void MelderDir_getParentDir (MelderDir file, MelderDir parent);
bool MelderDir_isDesktop (MelderDir dir);
void MelderDir_getSubdir (MelderDir parent, const wchar_t *subdirName, MelderDir subdir);
void Melder_rememberShellDirectory (void);
wchar_t * Melder_getShellDirectory (void);
void Melder_getHomeDir (MelderDir homeDir);
void Melder_getPrefDir (MelderDir prefDir);
void Melder_getTempDir (MelderDir tempDir);

bool MelderFile_exists (MelderFile file);
bool MelderFile_readable (MelderFile file);
long MelderFile_length (MelderFile file);
void MelderFile_delete (MelderFile file);

/* The following two should be combined with each other and with Windows extension setting: */
FILE * Melder_fopen (MelderFile file, const char *type);
void Melder_fclose (MelderFile file, FILE *stream);
void Melder_files_cleanUp (void);

void Melder_tracingToFile (MelderFile file);
void Melder_trace_FMT (const char *fileName, int lineNumber, const char *functionName, const char *format, ...);
#ifdef NDEBUG
	#define Melder_assert(x)   ((void) 0)
	#define trace(x)   ((void) 0)
#else
	#define Melder_assert(x)   ((x) ? (void) (0) : (void) Melder_assert_ (#x, __FILE__, __LINE__))
	#define trace(...)   ((! Melder_isTracing) ? (void) 0 : Melder_trace_FMT (__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__))
#endif

/* So these will be the future replacements for the above, as soon as we rid of text files: */
MelderFile MelderFile_open (MelderFile file);
MelderFile MelderFile_append (MelderFile file);
MelderFile MelderFile_create (MelderFile file);
void * MelderFile_read (MelderFile file, long nbytes);
char * MelderFile_readLine (MelderFile file);
void MelderFile_writeCharacter (MelderFile file, wchar_t kar);
void MelderFile_writeCharacter (MelderFile file, char32 kar);
void MelderFile_write (MelderFile file, const wchar_t *s1);
inline static
void MelderFile_write (MelderFile file, const char32 *s1) {
	MelderFile_write (file, Melder_peekStr32ToWcs (s1));
}
void MelderFile_write (MelderFile file, const wchar_t *s1, const wchar_t *s2);
inline static
void MelderFile_write (MelderFile file, const char32 *s1, const char32 *s2) {
	MelderFile_write (file, Melder_peekStr32ToWcs (s1), Melder_peekStr32ToWcs (s2));
}
void MelderFile_write (MelderFile file, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3);
inline static
void MelderFile_write (MelderFile file, const char32 *s1, const char32 *s2, const char32 *s3) {
	MelderFile_write (file, Melder_peekStr32ToWcs (s1), Melder_peekStr32ToWcs (s2), Melder_peekStr32ToWcs (s3));
}
void MelderFile_write (MelderFile file, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4);
inline static
void MelderFile_write (MelderFile file, const char32 *s1, const char32 *s2, const char32 *s3, const char32 *s4) {
	MelderFile_write (file, Melder_peekStr32ToWcs (s1), Melder_peekStr32ToWcs (s2), Melder_peekStr32ToWcs (s3), Melder_peekStr32ToWcs (s4));
}
void MelderFile_write (MelderFile file, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5);
inline static
void MelderFile_write (MelderFile file, const char32 *s1, const char32 *s2, const char32 *s3, const char32 *s4, const char32 *s5) {
	MelderFile_write (file, Melder_peekStr32ToWcs (s1), Melder_peekStr32ToWcs (s2), Melder_peekStr32ToWcs (s3), Melder_peekStr32ToWcs (s4), Melder_peekStr32ToWcs (s5));
}
void MelderFile_write (MelderFile file, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6);
inline static
void MelderFile_write (MelderFile file, const char32 *s1, const char32 *s2, const char32 *s3, const char32 *s4, const char32 *s5, const char32 *s6) {
	MelderFile_write (file, Melder_peekStr32ToWcs (s1), Melder_peekStr32ToWcs (s2), Melder_peekStr32ToWcs (s3), Melder_peekStr32ToWcs (s4), Melder_peekStr32ToWcs (s5), Melder_peekStr32ToWcs (s6));
}
void MelderFile_write (MelderFile file, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7);
inline static
void MelderFile_write (MelderFile file, const char32 *s1, const char32 *s2, const char32 *s3, const char32 *s4, const char32 *s5, const char32 *s6, const char32 *s7) {
	MelderFile_write (file, Melder_peekStr32ToWcs (s1), Melder_peekStr32ToWcs (s2), Melder_peekStr32ToWcs (s3), Melder_peekStr32ToWcs (s4), Melder_peekStr32ToWcs (s5), Melder_peekStr32ToWcs (s6), Melder_peekStr32ToWcs (s7));
}
void MelderFile_write (MelderFile file, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8);
void MelderFile_write (MelderFile file, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8, const wchar_t *s9);
void MelderFile_rewind (MelderFile file);
void MelderFile_seek (MelderFile file, long position, int direction);
long MelderFile_tell (MelderFile file);
void MelderFile_close (MelderFile file);
void MelderFile_close_nothrow (MelderFile file);

/* Read and write whole text files. */
wchar_t * MelderFile_readText (MelderFile file);
char32 * MelderFile_readText32 (MelderFile file);
void MelderFile_writeText (MelderFile file, const wchar_t *text, enum kMelder_textOutputEncoding outputEncoding);
void MelderFile_writeText32 (MelderFile file, const char32 *text, enum kMelder_textOutputEncoding outputEncoding);
void MelderFile_appendText (MelderFile file, const wchar_t *text);
void MelderFile_appendText32 (MelderFile file, const char32 *text);

void Melder_createDirectory (MelderDir parent, const wchar_t *subdirName, int mode);
inline static
void Melder_createDirectory (MelderDir parent, const char32 *subdirName, int mode) {
	Melder_createDirectory (parent, Melder_peekStr32ToWcs (subdirName), mode);
}

void Melder_getDefaultDir (MelderDir dir);
void Melder_setDefaultDir (MelderDir dir);
void MelderFile_setDefaultDir (MelderFile file);

/* Use the following functions to pass unchanged text or file names to Melder_* functions. */
/* Backslashes are replaced by "\bs". */
/* The trick is that they return one of 11 cyclically used static strings, */
/* so you can use up to 11 strings in a single Melder_* call. */
wchar_t * Melder_peekExpandBackslashes (const wchar_t *message);
const wchar_t * MelderFile_messageName (MelderFile file);   // Calls Melder_peekExpandBackslashes ().

/*
 * Some often used characters.
 */
#define L_LEFT_SINGLE_QUOTE  L"\u2018"
#define L_RIGHT_SINGLE_QUOTE  L"\u2019"
#define L_LEFT_DOUBLE_QUOTE  L"\u201c"
#define L_RIGHT_DOUBLE_QUOTE  L"\u201d"
#define L_LEFT_GUILLEMET  L"\u00ab"
#define L_RIGHT_GUILLEMET  L"\u00bb"

#define Melder_free(pointer)  _Melder_free ((void **) & (pointer))
void _Melder_free (void **pointer);
/*
	Preconditions:
		none (*pointer may be NULL).
	Postconditions:
		*pointer == NULL;
*/

double Melder_allocationCount (void);
/*
	Returns the total number of successful calls to
	Melder_malloc, Melder_realloc (if 'ptr' is NULL), Melder_calloc, and Melder_strdup,
	since the start of the process. Mainly for debugging purposes.
*/

double Melder_deallocationCount (void);
/*
	Returns the total number of successful calls to Melder_free,
	since the start of the process. Mainly for debugging purposes.
*/

double Melder_allocationSize (void);
/*
	Returns the total number of bytes allocated in calls to
	Melder_malloc, Melder_realloc (if moved), Melder_calloc, and Melder_strdup,
	since the start of the process. Mainly for debugging purposes.
*/

double Melder_reallocationsInSituCount (void);
double Melder_movingReallocationsCount (void);

/********** STRINGS **********/

/* These are routines for never having to check string boundaries again. */

typedef struct {
	int64 length;
	int64 bufferSize;
	wchar *string;   // a growing buffer, never shrunk (can only be freed by MelderString_free)
} MelderString;
typedef struct {
	int64 length;
	int64 bufferSize;
	char16 *string;   // a growing buffer, never shrunk (can only be freed by MelderString16_free)
} MelderString16;
typedef struct {
	int64 length;
	int64 bufferSize;
	char32 *string;   // a growing buffer, never shrunk (can only be freed by MelderString32_free)
} MelderString32;

void MelderString_free (MelderString *me);   // frees the "string" attribute only (and sets other attributes to zero)
void MelderString16_free (MelderString16 *me);   // frees the "string" attribute only (and sets other attributes to zero)
void MelderString32_free (MelderString32 *me);   // frees the "string" attribute only (and sets other attributes to zero)
void MelderString_empty (MelderString *me);   // sets to empty string (buffer not freed)
void MelderString16_empty (MelderString16 *me);   // sets to empty string (buffer not freed)
void MelderString32_empty (MelderString32 *me);   // sets to empty string (buffer not freed)
void MelderString_copy (MelderString *me, const wchar_t *source);
void MelderString32_copy (MelderString32 *me, const char32 *source);
void MelderString_ncopy (MelderString *me, const wchar_t *source, int64 n);
void MelderString32_ncopy (MelderString32 *me, const char32 *source, int64 n);
void MelderString_append (MelderString *me, const wchar_t *s1);
void MelderString_append (MelderString *me, const wchar_t *s1, const wchar_t *s2);
void MelderString_append (MelderString *me, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3);
void MelderString_append (MelderString *me, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4);
void MelderString_append (MelderString *me, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4,
	const wchar_t *s5);
void MelderString_append (MelderString *me, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4,
	const wchar_t *s5, const wchar_t *s6);
void MelderString_append (MelderString *me, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4,
	const wchar_t *s5, const wchar_t *s6, const wchar_t *s7);
void MelderString_append (MelderString *me, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4,
	const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8);
void MelderString_append (MelderString *me, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4,
	const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8, const wchar_t *s9);
void MelderString32_append (MelderString32 *me, const char32 *s1);
void MelderString32_append (MelderString32 *me, const char32 *s1, const char32 *s2);
void MelderString32_append (MelderString32 *me, const char32 *s1, const char32 *s2, const char32 *s3);
void MelderString32_append (MelderString32 *me, const char32 *s1, const char32 *s2, const char32 *s3, const char32 *s4);
void MelderString32_append (MelderString32 *me, const char32 *s1, const char32 *s2, const char32 *s3, const char32 *s4,
	const char32 *s5);
void MelderString32_append (MelderString32 *me, const char32 *s1, const char32 *s2, const char32 *s3, const char32 *s4,
	const char32 *s5, const char32 *s6);
void MelderString32_append (MelderString32 *me, const char32 *s1, const char32 *s2, const char32 *s3, const char32 *s4,
	const char32 *s5, const char32 *s6, const char32 *s7);
void MelderString32_append (MelderString32 *me, const char32 *s1, const char32 *s2, const char32 *s3, const char32 *s4,
	const char32 *s5, const char32 *s6, const char32 *s7, const char32 *s8);
void MelderString32_append (MelderString32 *me, const char32 *s1, const char32 *s2, const char32 *s3, const char32 *s4,
	const char32 *s5, const char32 *s6, const char32 *s7, const char32 *s8, const char32 *s9);
void MelderString_appendCharacter (MelderString *me, wchar_t character);
void MelderString16_appendCharacter (MelderString16 *me, wchar_t character);
void MelderString16_appendCharacter (MelderString16 *me, char32 character);
void MelderString32_appendCharacter (MelderString32 *me, char32 character);
void MelderString_get (MelderString *me, wchar_t *destination);   // performs no boundary checking
void MelderString32_get (MelderString32 *me, char32 *destination);
double MelderString_allocationCount (void);
double MelderString_deallocationCount (void);
double MelderString_allocationSize (void);
double MelderString_deallocationSize (void);

struct structMelderReadText {
	char32 *string32, *readPointer32;
	char *string8, *readPointer8;
	unsigned long input8Encoding;
};
typedef struct structMelderReadText *MelderReadText;

MelderReadText MelderReadText_createFromFile (MelderFile file);
MelderReadText MelderReadText_createFromString (const wchar_t *string);
char32 MelderReadText_getChar (MelderReadText text);
char32 * MelderReadText_readLine (MelderReadText text);
wchar_t * MelderReadText_readLineW (MelderReadText text);
int64_t MelderReadText_getNumberOfLines (MelderReadText me);
const wchar_t * MelderReadText_getLineNumber (MelderReadText text);
void MelderReadText_delete (MelderReadText text);

const wchar_t * Melder_wcscat (const wchar_t *s1, const wchar_t *s2);
const wchar_t * Melder_wcscat (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3);
const wchar_t * Melder_wcscat (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4);
const wchar_t * Melder_wcscat (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5);
const wchar_t * Melder_wcscat (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6);
const wchar_t * Melder_wcscat (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7);
const wchar_t * Melder_wcscat (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8);
const wchar_t * Melder_wcscat (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8, const wchar_t *s9);
const char32 * Melder_str32cat (const char32 *s1, const char32 *s2);
const char32 * Melder_str32cat (const char32 *s1, const char32 *s2, const char32 *s3);
const char32 * Melder_str32cat (const char32 *s1, const char32 *s2, const char32 *s3, const char32 *s4);
const char32 * Melder_str32cat (const char32 *s1, const char32 *s2, const char32 *s3, const char32 *s4, const char32 *s5);
const char32 * Melder_str32cat (const char32 *s1, const char32 *s2, const char32 *s3, const char32 *s4, const char32 *s5, const char32 *s6);
const char32 * Melder_str32cat (const char32 *s1, const char32 *s2, const char32 *s3, const char32 *s4, const char32 *s5, const char32 *s6, const char32 *s7);
const char32 * Melder_str32cat (const char32 *s1, const char32 *s2, const char32 *s3, const char32 *s4, const char32 *s5, const char32 *s6, const char32 *s7, const char32 *s8);
const char32 * Melder_str32cat (const char32 *s1, const char32 *s2, const char32 *s3, const char32 *s4, const char32 *s5, const char32 *s6, const char32 *s7, const char32 *s8, const char32 *s9);

/********** NUMBER AND STRING COMPARISON **********/

int Melder_numberMatchesCriterion (double value, int which_kMelder_number, double criterion);
int Melder_stringMatchesCriterion (const wchar_t *value, int which_kMelder_string, const wchar_t *criterion);
inline static
int Melder_stringMatchesCriterion (const char32 *value, int which_kMelder_string, const char32 *criterion) {
	return Melder_stringMatchesCriterion (Melder_peekStr32ToWcs (value), which_kMelder_string, Melder_peekStr32ToWcs (criterion));
}

/********** STRING PARSING **********/

/*
	These functions regard a string as a sequence of tokens,
	separated (and perhaps preceded and followed) by white space.
	The tokens cannot contain spaces themselves (there are no escapes).
	Typical use:
		for (token = Melder_firstToken (string); token != NULL; token = Melder_nextToken ()) {
			... do something with the token ...
		}
*/

long Melder_countTokens (const wchar_t *string);
wchar_t *Melder_firstToken (const wchar_t *string);
wchar_t *Melder_nextToken (void);
wchar_t ** Melder_getTokens (const wchar_t *string, long *n);
void Melder_freeTokens (wchar_t ***tokens);
long Melder_searchToken (const wchar_t *string, wchar_t **tokens, long n);

/********** MESSAGING ROUTINES **********/

/* These functions are called like printf ().
	Default Melder does fprintf to stderr,
	except Melder_information, which does fprintf to stdout.
	These functions show generic, native, and mixed strings correctly,
	and perform quote conversion, if that flag is not off; see under "NON-ASCII CHARACTERS".
	The alphabet is Roman, so that symbols from the Symbol and Phonetic alphabets
	are not translated (by default, \mu is shown as \mu and \as as a).
*/

void Melder_casual (const char *format, ...);
void Melder_casual1 (const wchar_t *s1);
void Melder_casual2 (const wchar_t *s1, const wchar_t *s2);
void Melder_casual3 (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3);
void Melder_casual4 (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4);
void Melder_casual5 (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5);
void Melder_casual6 (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6);
void Melder_casual7 (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7);
void Melder_casual8 (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8);
void Melder_casual9 (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8, const wchar_t *s9);
/*
	Function:
		Sends a message without user interference.
	Behaviour:
		Writes to stderr on Unix, otherwise to a special window.
*/

/* Give information to stdout (batch), or to an "Info" window (interactive), or to a diverted string. */

void MelderInfo_open (void);   // clear the Info window in the background
void MelderInfo_write (const wchar_t *s1);   // write a string to the Info window in the background
inline static
void MelderInfo_write (const char32 *s1) {
	MelderInfo_write (Melder_peekStr32ToWcs (s1));
}
void MelderInfo_write (const wchar_t *s1, const wchar_t *s2);   // write two strings to the Info window in the background
inline static
void MelderInfo_write (const char32 *s1, const char32 *s2) {
	MelderInfo_write (Melder_peekStr32ToWcs (s1), Melder_peekStr32ToWcs (s2));
}
void MelderInfo_write (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3);
void MelderInfo_write (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4);
void MelderInfo_write (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5);
void MelderInfo_write (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6);
void MelderInfo_write (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7);
void MelderInfo_write (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8);
void MelderInfo_write (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8, const wchar_t *s9);
void MelderInfo_writeLine (const wchar_t *s1);   /* Write a string to the Info window in the background; add a new-line. */
void MelderInfo_writeLine (const wchar_t *s1, const wchar_t *s2);
inline static
void MelderInfo_writeLine (const char32 *s1, const char32 *s2) {
	MelderInfo_writeLine (Melder_peekStr32ToWcs (s1), Melder_peekStr32ToWcs (s2));
}
void MelderInfo_writeLine (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3);
inline static
void MelderInfo_writeLine (const char32 *s1, const char32 *s2, const char32 *s3) {
	MelderInfo_writeLine (Melder_peekStr32ToWcs (s1), Melder_peekStr32ToWcs (s2), Melder_peekStr32ToWcs (s3));
}
void MelderInfo_writeLine  (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4);
void MelderInfo_writeLine  (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5);
void MelderInfo_writeLine  (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6);
void MelderInfo_writeLine  (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7);
void MelderInfo_writeLine  (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8);
void MelderInfo_writeLine  (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8, const wchar_t *s9);
void MelderInfo_close (void);   // drain the background info to the Info window, making sure there is a line break
void MelderInfo_drain (void);   // drain the background info to the Info window, without adding any extra line break

void Melder_information (const wchar_t *s1);
void Melder_information (const wchar_t *s1, const wchar_t *s2);
void Melder_information (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3);
void Melder_information (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4);
void Melder_information (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5);
void Melder_information (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6);
void Melder_information (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7);
void Melder_information (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8);
void Melder_information (const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8, const wchar_t *s9);

void Melder_informationReal (double value, const wchar_t *units);   /* %.17g or --undefined--; units may be NULL */

void Melder_divertInfo (MelderString *buffer);   /* NULL = back to normal. */

class autoMelderDivertInfo {
	public:
		autoMelderDivertInfo (MelderString *buffer) { Melder_divertInfo (buffer); }
		~autoMelderDivertInfo () { Melder_divertInfo (NULL); }
};

void Melder_clearInfo (void);   /* Clear the Info window. */
const wchar_t * Melder_getInfo (void);
void Melder_help (const wchar_t *query);

void Melder_search (void);
	
void Melder_beep (void);

extern int Melder_debug;

/* The following trick uses Melder_debug only because it is the only plain variable known to exist at the moment. */
#define Melder_offsetof(klas,member) (int) ((char *) & ((klas) & Melder_debug) -> member - (char *) & Melder_debug)

/********** ERROR **********/

class MelderError { };

typedef class structThing *Thing;
wchar_t *Thing_messageName (Thing me);
struct MelderArg {
	int type;
	union {
		const wchar_t *argW;
		const  char   *arg8;
	};
	MelderArg (const wchar_t *      arg) : type (1), argW (arg) { }
	MelderArg (const  char   *      arg) : type (2), arg8 (arg) { }
	MelderArg (const  char32 *      arg) : type (1), argW (Melder_peekStr32ToWcs (arg)) { }
	MelderArg (const double         arg) : type (1), argW (Melder_double          (arg)) { }
	MelderArg (const          long long  arg) : type (1), argW (Melder_integer         (arg)) { }
	MelderArg (const unsigned long long  arg) : type (1), argW (Melder_integer         ((int64_t) arg)) { }
	MelderArg (const          long  arg) : type (1), argW (Melder_integer         (arg)) { }
	MelderArg (const unsigned long  arg) : type (1), argW (Melder_integer         ((int64_t) arg)) { }   // ignore ULL above 2^63
	MelderArg (const          int   arg) : type (1), argW (Melder_integer         (arg)) { }
	MelderArg (const unsigned int   arg) : type (1), argW (Melder_integer         (arg)) { }
	MelderArg (const          short arg) : type (1), argW (Melder_integer         (arg)) { }
	MelderArg (const unsigned short arg) : type (1), argW (Melder_integer         (arg)) { }
	MelderArg (Thing                arg) : type (1), argW (Thing_messageName      (arg)) { }
	MelderArg (MelderFile           arg) : type (1), argW (MelderFile_messageName (arg)) { }
};
void Melder_throw (const MelderArg& arg1);
void Melder_throw (const MelderArg& arg1, const MelderArg& arg2);
void Melder_throw (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3);
void Melder_throw (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4);
void Melder_throw (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4, const MelderArg& arg5);
void Melder_throw (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6);
void Melder_throw (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7);
void Melder_throw (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8);
void Melder_throw (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8, const MelderArg& arg9);
void Melder_throw (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8,
	const MelderArg& arg9, const MelderArg& arg10);
void Melder_throw (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8,
	const MelderArg& arg9, const MelderArg& arg10, const MelderArg& arg11);
void Melder_throw (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8,
	const MelderArg& arg9, const MelderArg& arg10, const MelderArg& arg11, const MelderArg& arg12, const MelderArg& arg13 = L"");
void Melder_throw (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4, const MelderArg& arg5,
	const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8, const MelderArg& arg9, const MelderArg& arg10,
	const MelderArg& arg11, const MelderArg& arg12, const MelderArg& arg13, const MelderArg& arg14, const MelderArg& arg15 = L"");
void Melder_throw (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8,
	const MelderArg& arg9, const MelderArg& arg10, const MelderArg& arg11, const MelderArg& arg12,
	const MelderArg& arg13, const MelderArg& arg14, const MelderArg& arg15, const MelderArg& arg16,
	const MelderArg& arg17 = L"", const MelderArg& arg18 = L"", const MelderArg& arg19 = L"", const MelderArg& arg20 = L"");
void Melder_error_noLine (const MelderArg& arg1);
void Melder_error_ (const MelderArg& arg1);
void Melder_error_ (const MelderArg& arg1, const MelderArg& arg2);
void Melder_error_ (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3);
void Melder_error_ (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4);
void Melder_error_ (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4, const MelderArg& arg5);
void Melder_error_ (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6);
void Melder_error_ (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7);
void Melder_error_ (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8);
void Melder_error_ (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8, const MelderArg& arg9);
void Melder_error_ (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8,
	const MelderArg& arg9, const MelderArg& arg10);
void Melder_error_ (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8,
	const MelderArg& arg9, const MelderArg& arg10, const MelderArg& arg11);
void Melder_error_ (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8,
	const MelderArg& arg9, const MelderArg& arg10, const MelderArg& arg11, const MelderArg& arg12, const MelderArg& arg13 = L"");
void Melder_error_ (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4, const MelderArg& arg5,
	const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8, const MelderArg& arg9, const MelderArg& arg10,
	const MelderArg& arg11, const MelderArg& arg12, const MelderArg& arg13, const MelderArg& arg14, const MelderArg& arg15 = L"");
void Melder_error_ (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8,
	const MelderArg& arg9, const MelderArg& arg10, const MelderArg& arg11, const MelderArg& arg12,
	const MelderArg& arg13, const MelderArg& arg14, const MelderArg& arg15, const MelderArg& arg16,
	const MelderArg& arg17 = L"", const MelderArg& arg18 = L"", const MelderArg& arg19 = L"", const MelderArg& arg20 = L"");
#define Melder_throw(...)  do { Melder_error_ (__VA_ARGS__); throw MelderError (); } while (false)

void Melder_flushError (const char *format, ...);
	/* Send all deferred error messages to stderr (batch) or to an "Error" dialog, */
	/* including, if 'format' is not NULL, the error message generated by this routine. */

bool Melder_hasError ();
bool Melder_hasError (const wchar_t *partialError);
	/* Returns 1 if there is an error message in store, otherwise 0. */

void Melder_clearError (void);
	/* Cancel all stored error messages. */

wchar_t * Melder_getError (void);
	/* Returns the error string. Mainly used with wcsstr. */

/********** WARNING: ive warning to stderr (batch) or to a "Warning" dialog **********/

void Melder_warning (const MelderArg& arg1);
void Melder_warning (const MelderArg& arg1, const MelderArg& arg2);
void Melder_warning (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3);
void Melder_warning (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4);
void Melder_warning (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4, const MelderArg& arg5);
void Melder_warning (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6);
void Melder_warning (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7);
void Melder_warning (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8);
void Melder_warning (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8, const MelderArg& arg9);
void Melder_warning (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8,
	const MelderArg& arg9, const MelderArg& arg10);
void Melder_warning (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8,
	const MelderArg& arg9, const MelderArg& arg10, const MelderArg& arg11);
void Melder_warning (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8,
	const MelderArg& arg9, const MelderArg& arg10, const MelderArg& arg11, const MelderArg& arg12, const MelderArg& arg13 = L"");
void Melder_warning (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4, const MelderArg& arg5,
	const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8, const MelderArg& arg9, const MelderArg& arg10,
	const MelderArg& arg11, const MelderArg& arg12, const MelderArg& arg13, const MelderArg& arg14, const MelderArg& arg15 = L"");
void Melder_warning (const MelderArg& arg1, const MelderArg& arg2, const MelderArg& arg3, const MelderArg& arg4,
	const MelderArg& arg5, const MelderArg& arg6, const MelderArg& arg7, const MelderArg& arg8,
	const MelderArg& arg9, const MelderArg& arg10, const MelderArg& arg11, const MelderArg& arg12,
	const MelderArg& arg13, const MelderArg& arg14, const MelderArg& arg15, const MelderArg& arg16,
	const MelderArg& arg17 = L"", const MelderArg& arg18 = L"", const MelderArg& arg19 = L"", const MelderArg& arg20 = L"");

void Melder_warningOff (void);
void Melder_warningOn (void);

class autoMelderWarningOff {
public:
	autoMelderWarningOff () { Melder_warningOff (); }
	~autoMelderWarningOff () { Melder_warningOn (); }
};

/********** PROGRESS ROUTINES **********/

void Melder_progress (double progress, const wchar_t *s1);
void Melder_progress (double progress, const wchar_t *s1, const wchar_t *s2);
void Melder_progress (double progress, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3);
void Melder_progress (double progress, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4);
void Melder_progress (double progress, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5);
void Melder_progress (double progress, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6);
void Melder_progress (double progress, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7);
void Melder_progress (double progress, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8);
void Melder_progress (double progress, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8, const wchar_t *s9);
void Melder_progressOff (void);
void Melder_progressOn (void);
/*
	Function:
		Show the progress of a time-consuming process.
	Arguments:
		Any of 's1' through 's9' may be NULL.
	Batch behaviour:
		Does nothing, always returns 1.
	Interactive behaviour:
		Shows the progress of a time-consuming process:
		- if 'progress' <= 0.0, show a window with text and a Cancel button, and return 1;
		- if 0.0 < 'progress' < 1.0, show text and a partially filled progress bar,
		  and return 0 if user interrupts, else return 1;
		- if 'progress' >= 1, hide the window.
	Usage:
		- call with 'progress' = 0.0 before the process starts:
		      (void) Melder_progress (0.0, "Starting work...");
		- at every turn in your loop, call with 'progress' between 0 and 1:
		      Melder_progress (i / (n + 1.0), L"Working on part ", i, L" out of ", n, L"...");
		  an exception is thrown if the user clicks Cancel; if you don't want that, catch it:
		      try {
		          Melder_progress (i / (n + 1.0), L"Working on part ", i, L" out of ", n, L"...");
		      } catch (MelderError) {
		          Melder_clearError ();
		          break;
		      }
		- after the process has finished, call with 'progress' = 1.0:
		      (void) Melder_progress (1.0, NULL);
		- the first and third steps can be automated by autoMelderProgress:
		      autoMelderProgress progress ("Starting work...");
*/
class autoMelderProgress {
public:
	autoMelderProgress (const wchar_t *message) {
		Melder_progress (0.0, message);
	}
	~autoMelderProgress () {
		Melder_progress (1.0, NULL);
	}
};

void * Melder_monitor (double progress, const wchar_t *s1);
void * Melder_monitor (double progress, const wchar_t *s1, const wchar_t *s2);
void * Melder_monitor (double progress, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3);
void * Melder_monitor (double progress, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4);
void * Melder_monitor (double progress, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5);
void * Melder_monitor (double progress, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6);
void * Melder_monitor (double progress, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7);
void * Melder_monitor (double progress, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8);
void * Melder_monitor (double progress, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const wchar_t *s4, const wchar_t *s5, const wchar_t *s6, const wchar_t *s7, const wchar_t *s8, const wchar_t *s9);
/*
	Function:
		Show the progress of a time-consuming process.
	Arguments:
		Any of 's1' through 's9' may be NULL.
	Batch behaviour:
		Does nothing, returns NULL if 'progress' <= 0.0 and a non-NULL pointer otherwise.
	Interactive behaviour:
		Shows the progress of a time-consuming process:
		- if 'progress' <= 0.0, show a window with text and a Cancel button and
		  room for a square drawing, and return a Graphics;
		- if 0.0 < 'progress' < 1.0, show text and a partially filled progress bar,
		  and return NULL if user interrupts, else return a non-NULL pointer;
		- if 'progress' >= 1, hide the window.
	Usage:
		- call with 'progress' = 0.0 before the process starts.
		- assign the return value to a Graphics:
		      Graphics graphics = Melder_monitor (0.0, L"Starting work...");
		- at every turn of your loop, draw something in the Graphics:
		      if (graphics) {   // always check; might be batch
		          Graphics_clearWs (graphics);   // only if you redraw all every time
		          Graphics_polyline (graphics, ...);
		          Graphics_text (graphics, ...);
		      }
		- immediately after this in your loop, call with 'progress' between 0 and 1:
		      Melder_monitor (i / (n + 1.0), L"Working on part ", Melder_integer (i), L" out of ", Melder_integer (n), L"...");
		  an exception is thrown if the user clicks Cancel; if you don't want that, catch it:
		      try {
		          Melder_monitor (i / (n + 1.0), L"Working on part ", Melder_integer (i), L" out of ", Melder_integer (n), L"...");
		      } catch (MelderError) {
		          Melder_clearError ();
		          break;
		      }
		- after the process has finished, call with 'progress' = 1.0:
		      (void) Melder_monitor (1.0, NULL);
		- the first and third steps can be automated by autoMelderMonitor:
		      autoMelderMonitor monitor ("Starting work...");
		      if (monitor.graphics()) {   // always check; might be batch
		          Graphics_clearWs (monitor.graphics());   // only if you redraw all every time
		          Graphics_polyline (monitor.graphics(), ...);
		          Graphics_text (monitor.graphics(), ...);
		      }
*/
typedef class structGraphics *Graphics;
class autoMelderMonitor {
	Graphics d_graphics;
public:
	autoMelderMonitor (const wchar_t *message) {
		d_graphics = (Graphics) Melder_monitor (0.0, message);
	}
	~autoMelderMonitor () {
		Melder_monitor (1.0, NULL);
	}
	Graphics graphics () { return d_graphics; }
};

/********** RECORD AND PLAY ROUTINES **********/

int Melder_publish (void *anything);

int Melder_record (double duration);
int Melder_recordFromFile (MelderFile fs);
void Melder_play (void);
void Melder_playReverse (void);
int Melder_publishPlayed (void);

/********** SYSTEM VERSION **********/

extern unsigned long Melder_systemVersion;
/*
	For Macintosh, this is set in praat_init.
*/

/********** SCRATCH TEXT BUFFERS **********/

extern char Melder_buffer1 [30001], Melder_buffer2 [30001];
/*
	Every Melder routine uses both of these buffers:
	one for sprintfing the message,
	and one for translating this message to a native string.
	You can use these buffers,
	but be careful not to call any other routines that use them at the same time;
	the following routines are guaranteed not to call the Melder library:
	 - Mac Toolbox, XWindows, X Toolkit, Motif, and XVT routines,
		except those who dispatch events (XtDispatchEvent, xvt_process_pending_events).
	 - Longchar_*
	This means that you can use these buffers for reading and writing with
	the Longchar library.
*/

/********** ENFORCE INTERACTIVE BEHAVIOUR **********/

/* Procedures to enforce interactive behaviour of the Melder_XXXXXX routines. */

void MelderGui_create (/* GuiWindow */ void *parent);
/*
	'parent' is the top-level widget returned by GuiAppInitialize.
*/

extern bool Melder_batch;   // true if run from the batch or from an interactive command-line interface
extern bool Melder_backgrounding;   /* True if running a script. */
extern bool Melder_consoleIsAnsi;
extern bool Melder_asynchronous;   // true if specified by the "asynchronous" directive in a script
#ifndef CONTROL_APPLICATION
	typedef struct structGuiWindow *GuiWindow;
	extern GuiWindow Melder_topShell;
#endif

/********** OVERRIDE DEFAULT BEHAVIOUR **********/

/* Procedures to override default message methods. */
/* They may chage the string arguments. */
/* Many of these routines are called by MelderMotif_create and MelderXvt_create. */

void Melder_setCasualProc (void (*casualProc) (const wchar_t *message));
void Melder_setProgressProc (int (*progressProc) (double progress, const wchar_t *message));
void Melder_setMonitorProc (void * (*monitorProc) (double progress, const wchar_t *message));
void Melder_setInformationProc (void (*informationProc) (const wchar_t *message));
void Melder_setHelpProc (void (*help) (const wchar_t *query));
void Melder_setSearchProc (void (*search) (void));
void Melder_setWarningProc (void (*warningProc) (const wchar_t *message));
void Melder_setErrorProc (void (*errorProc) (const wchar_t *message));
void Melder_setFatalProc (void (*fatalProc) (const wchar_t *message));
void Melder_setPublishProc (int (*publish) (void *));
void Melder_setRecordProc (int (*record) (double));
void Melder_setRecordFromFileProc (int (*recordFromFile) (MelderFile));
void Melder_setPlayProc (void (*play) (void));
void Melder_setPlayReverseProc (void (*playReverse) (void));
void Melder_setPublishPlayedProc (int (*publishPlayed) (void));

double Melder_stopwatch (void);

long Melder_killReturns_inline (char *text);
/*
	 Replaces all bare returns (Mac) or return / linefeed sequences (Win) with bare linefeeds (generic = Unix).
	 Returns new length of string (equal to or less than old length).
*/

/********** AUDIO **********/

#if defined (macintosh) || defined (_WIN32) || defined (linux)
	#define kMelderAudio_inputUsesPortAudio_DEFAULT  true
		// Mac: in order to have CoreAudio
		// Win: in order to allow recording for over 64 megabytes (paMME)
		// Linux: in order to use ALSA and therefore be compatible with Ubuntu 10.10 and later
#else
	#define kMelderAudio_inputUsesPortAudio_DEFAULT  false
#endif
void MelderAudio_setInputUsesPortAudio (bool inputUsesPortAudio);
bool MelderAudio_getInputUsesPortAudio (void);
#if defined (macintosh) || defined (linux)
	#define kMelderAudio_outputUsesPortAudio_DEFAULT  true
		// Mac: in order to have CoreAudio
		// Linux: in order to use ALSA and therefore be compatible with Ubuntu 10.10 and later
#else
	#define kMelderAudio_outputUsesPortAudio_DEFAULT  false
		// Win: in order to reduce the long latencies of paMME and to avoid the incomplete implementation of paDirectSound
#endif
void MelderAudio_setOutputUsesPortAudio (bool outputUsesPortAudio);
bool MelderAudio_getOutputUsesPortAudio (void);
#if 1
	#define kMelderAudio_outputSilenceBefore_DEFAULT  0.0
		// Mac: in order to switch off the BOING caused by the automatic gain control
#endif
void MelderAudio_setOutputSilenceBefore (double silenceBefore);
double MelderAudio_getOutputSilenceBefore (void);
#if defined (macintosh)
	#define kMelderAudio_outputSilenceAfter_DEFAULT  0.0
		// Mac: in order to reduce the BOING caused by the automatic gain control when the user replays immediately after a sound has finished
#else
	#define kMelderAudio_outputSilenceAfter_DEFAULT  0.5
		// Win: in order to get rid of the click on some cards
		// Linux: in order to get rid of double playing of a sounding buffer
#endif
void MelderAudio_setOutputSilenceAfter (double silenceAfter);
double MelderAudio_getOutputSilenceAfter (void);
void MelderAudio_setUseInternalSpeaker (bool useInternalSpeaker);   // for HP-UX and Sun
bool MelderAudio_getUseInternalSpeaker (void);
void MelderAudio_setOutputMaximumAsynchronicity (enum kMelder_asynchronicityLevel maximumAsynchronicity);
enum kMelder_asynchronicityLevel MelderAudio_getOutputMaximumAsynchronicity (void);
long MelderAudio_getOutputBestSampleRate (long fsamp);

extern bool MelderAudio_isPlaying;
void MelderAudio_play16 (int16_t *buffer, long sampleRate, long numberOfSamples, int numberOfChannels,
	bool (*playCallback) (void *playClosure, long numberOfSamplesPlayed),   // return true to continue, false to stop
	void *playClosure);
bool MelderAudio_stopPlaying (bool isExplicit);   // returns true if sound was playing
#define MelderAudio_IMPLICIT  false
#define MelderAudio_EXPLICIT  true
long MelderAudio_getSamplesPlayed (void);
bool MelderAudio_stopWasExplicit (void);

void Melder_audio_prefs (void);   // in init file

/********** AUDIO FILES **********/

/* Audio file types. */
#define Melder_AIFF  1
#define Melder_AIFC  2
#define Melder_WAV  3
#define Melder_NEXT_SUN  4
#define Melder_NIST  5
#define Melder_FLAC 6
#define Melder_MP3 7
#define Melder_NUMBER_OF_AUDIO_FILE_TYPES  7
const wchar_t * Melder_audioFileTypeString (int audioFileType);   /* "AIFF", "AIFC", "WAV", "NeXT/Sun", "NIST", "FLAC", "MP3" */
/* Audio encodings. */
#define Melder_LINEAR_8_SIGNED  1
#define Melder_LINEAR_8_UNSIGNED  2
#define Melder_LINEAR_16_BIG_ENDIAN  3
#define Melder_LINEAR_16_LITTLE_ENDIAN  4
#define Melder_LINEAR_24_BIG_ENDIAN  5
#define Melder_LINEAR_24_LITTLE_ENDIAN  6
#define Melder_LINEAR_32_BIG_ENDIAN  7
#define Melder_LINEAR_32_LITTLE_ENDIAN  8
#define Melder_MULAW  9
#define Melder_ALAW  10
#define Melder_SHORTEN  11
#define Melder_POLYPHONE  12
#define Melder_IEEE_FLOAT_32_BIG_ENDIAN  13
#define Melder_IEEE_FLOAT_32_LITTLE_ENDIAN  14
#define Melder_FLAC_COMPRESSION_16 15
#define Melder_FLAC_COMPRESSION_24 16
#define Melder_FLAC_COMPRESSION_32 17
#define Melder_MPEG_COMPRESSION_16 18
#define Melder_MPEG_COMPRESSION_24 19
#define Melder_MPEG_COMPRESSION_32 20
int Melder_defaultAudioFileEncoding (int audioFileType, int numberOfBitsPerSamplePoint);   /* BIG_ENDIAN, BIG_ENDIAN, LITTLE_ENDIAN, BIG_ENDIAN, LITTLE_ENDIAN */
void MelderFile_writeAudioFileHeader (MelderFile file, int audioFileType, long sampleRate, long numberOfSamples, int numberOfChannels, int numberOfBitsPerSamplePoint);
void MelderFile_writeAudioFileTrailer (MelderFile file, int audioFileType, long sampleRate, long numberOfSamples, int numberOfChannels, int numberOfBitsPerSamplePoint);
void MelderFile_writeAudioFile (MelderFile file, int audioFileType, const short *buffer, long sampleRate, long numberOfSamples, int numberOfChannels, int numberOfBitsPerSamplePoint);

int MelderFile_checkSoundFile (MelderFile file, int *numberOfChannels, int *encoding,
	double *sampleRate, long *startOfData, int32 *numberOfSamples);
/* Returns information about a just opened audio file.
 * The return value is the audio file type, or 0 if it is not a sound file or in case of error.
 * The data start at 'startOfData' bytes from the start of the file.
 */
int Melder_bytesPerSamplePoint (int encoding);
void Melder_readAudioToFloat (FILE *f, int numberOfChannels, int encoding, double **buffer, long numberOfSamples);
/* Reads channels into buffer [ichannel], which are base-1.
 */
void Melder_readAudioToShort (FILE *f, int numberOfChannels, int encoding, short *buffer, long numberOfSamples);
/* If stereo, buffer will contain alternating left and right values.
 * Buffer is base-0.
 */
void MelderFile_writeFloatToAudio (MelderFile file, int numberOfChannels, int encoding, double **buffer, long numberOfSamples, int warnIfClipped);
void MelderFile_writeShortToAudio (MelderFile file, int numberOfChannels, int encoding, const short *buffer, long numberOfSamples);

/********** QUANTITY **********/

#define MelderQuantity_NONE  0
#define MelderQuantity_TIME_SECONDS  1
#define MelderQuantity_FREQUENCY_HERTZ  2
#define MelderQuantity_FREQUENCY_BARK  3
#define MelderQuantity_DISTANCE_FROM_GLOTTIS_METRES  4
#define MelderQuantity_NUMBER_OF_QUANTITIES  4
const wchar_t * MelderQuantity_getText (int quantity);   // e.g. "Time"
const wchar_t * MelderQuantity_getWithUnitText (int quantity);   // e.g. "Time (s)"
const wchar_t * MelderQuantity_getLongUnitText (int quantity);   // e.g. "seconds"
const wchar_t * MelderQuantity_getShortUnitText (int quantity);   // e.g. "s"

/********** MISCELLANEOUS **********/

wchar_t * Melder_getenv (const wchar_t *variableName);
void Melder_system (const char32 *command);   // spawn a system command
double Melder_clock (void);   // seconds since 1969

struct autoMelderProgressOff {
	autoMelderProgressOff () { Melder_progressOff (); }
	~autoMelderProgressOff () { Melder_progressOn (); }
};

struct autoMelderString : MelderString {
	autoMelderString () { length = 0; bufferSize = 0; string = NULL; }
	~autoMelderString () { Melder_free (string); }
	wchar_t * transfer () {
		wchar_t *tmp = string;
		string = NULL;
		length = 0;
		bufferSize = 0;
		return tmp;
	}
};
struct autoMelderString32 : MelderString32 {
	autoMelderString32 () { length = 0; bufferSize = 0; string = NULL; }
	~autoMelderString32 () { Melder_free (string); }
	char32 * transfer () {
		char32 *tmp = string;
		string = NULL;
		length = 0;
		bufferSize = 0;
		return tmp;
	}
};

struct autoMelderReadText {
	MelderReadText text;
	autoMelderReadText (MelderReadText a_text) : text (a_text) {
	}
	~autoMelderReadText () {
		if (text) MelderReadText_delete (text);
	}
	MelderReadText operator-> () const {   // as r-value
		return text;
	}
	MelderReadText peek () const {
		return text;
	}
	MelderReadText transfer () {
		MelderReadText tmp = text;
		text = NULL;
		return tmp;
	}
};

class autofile {
	FILE *ptr;
public:
	autofile (FILE *f) : ptr (f) {
	}
	autofile () : ptr (NULL) {
	}
	~autofile () {
		if (ptr) fclose (ptr);   // no error checking, because this is a destructor, only called after a throw, because otherwise you'd use f.close(file)
	}
	operator FILE * () {
		return ptr;
	}
	void reset (FILE *f) {
		if (ptr) fclose (ptr);   // BUG: not a normal closure
		ptr = f;
	}
	void close (MelderFile file) {
		if (ptr) {
			FILE *tmp = ptr;
			ptr = NULL;
			Melder_fclose (file, tmp);
		}
	}
};

class autoMelderFile {
	MelderFile file;
public:
	autoMelderFile (MelderFile a_file) : file (a_file) {
	}
	~autoMelderFile () {
		if (file) MelderFile_close_nothrow (file);
	}
	void close () {
		if (file && file -> filePointer) {
			MelderFile tmp = file;
			file = NULL;
			MelderFile_close (tmp);
		}
	}
	MelderFile transfer () {
		MelderFile tmp = file;
		file = NULL;
		return tmp;
	}
};

class autoMelderSaveDefaultDir {
	structMelderDir saveDir;
public:
	autoMelderSaveDefaultDir () {
		Melder_getDefaultDir (& saveDir);
	}
	~autoMelderSaveDefaultDir () {
		Melder_setDefaultDir (& saveDir);
	}
};

class autoMelderSetDefaultDir {
	structMelderDir saveDir;
public:
	autoMelderSetDefaultDir (MelderDir dir) {
		Melder_getDefaultDir (& saveDir);
		Melder_setDefaultDir (dir);
	}
	~autoMelderSetDefaultDir () {
		Melder_setDefaultDir (& saveDir);
	}
};

class autoMelderFileSetDefaultDir {
	structMelderDir saveDir;
public:
	autoMelderFileSetDefaultDir (MelderFile file) {
		Melder_getDefaultDir (& saveDir);
		MelderFile_setDefaultDir (file);
	}
	~autoMelderFileSetDefaultDir () {
		Melder_setDefaultDir (& saveDir);
	}
};

class autoMelderTokens {
	wchar_t **tokens;
public:
	autoMelderTokens () {
		tokens = NULL;
	}
	autoMelderTokens (const wchar_t *string, long *n) {
		tokens = Melder_getTokens (string, n);
	}
	~autoMelderTokens () {
		if (tokens) Melder_freeTokens (& tokens);
	}
	wchar_t*& operator[] (long i) {
		return tokens [i];
	}
	wchar_t ** peek () const {
		return tokens;
	}
	void reset (const wchar_t *string, long *n) {
		if (tokens) Melder_freeTokens (& tokens);
		tokens = Melder_getTokens (string, n);
	}
};

template <class T>
class _autostring {
	T *ptr;
public:
	_autostring (T *string) : ptr (string) {
		//if (Melder_debug == 39) Melder_casual ("autostring: constructor from C-string %ld", ptr);
	}
	_autostring () : ptr (0) {
		//if (Melder_debug == 39) Melder_casual ("autostring: zero constructor");
	}
	~_autostring () {
		//if (Melder_debug == 39) Melder_casual ("autostring: entering destructor ptr = %ld", ptr);
		if (ptr) Melder_free (ptr);
		//if (Melder_debug == 39) Melder_casual ("autostring: leaving destructor");
	}
	#if 0
	void operator= (T *string) {
		//if (Melder_debug == 39) Melder_casual ("autostring: entering assignment from C-string; old = %ld", ptr);
		if (ptr) Melder_free (ptr);
		ptr = string;
		//if (Melder_debug == 39) Melder_casual ("autostring: leaving assignment from C-string; new = %ld", ptr);
	}
	#endif
	template <class U> T& operator[] (U i) {
		return ptr [i];
	}
	T * peek () const {
		return ptr;
	}
	T ** operator& () {
		return & ptr;
	}
	T * transfer () {
		T *tmp = ptr;
		ptr = NULL;
		return tmp;
	}
	void reset (T *string) {
		if (ptr) Melder_free (ptr);
		ptr = string;
	}
	template <class U> void resize (U new_size) {
		T *tmp = (T *) Melder_realloc (ptr, new_size * sizeof (T));
		ptr = tmp;
	}
private:
	_autostring& operator= (const _autostring&);   // disable copy assignment
	//_autostring (_autostring &);   // disable copy constructor (trying it this way also disables good things like autostring s1 = wcsdup("hello");)
	template <class Y> _autostring (_autostring<Y> &);   // disable copy constructor
};

typedef _autostring <wchar_t> autostring;
typedef _autostring <char> autostring8;
typedef _autostring <char16> autostring16;
typedef _autostring <char32> autostring32;

class autoMelderAudioSaveMaximumAsynchronicity {
	enum kMelder_asynchronicityLevel d_saveAsynchronicity;
public:
	autoMelderAudioSaveMaximumAsynchronicity () {
		d_saveAsynchronicity = MelderAudio_getOutputMaximumAsynchronicity ();
		trace ("value was %d", (int) d_saveAsynchronicity);
	}
	~autoMelderAudioSaveMaximumAsynchronicity () {
		MelderAudio_setOutputMaximumAsynchronicity (d_saveAsynchronicity);
		trace ("value set to %d", (int) d_saveAsynchronicity);
	}
};

struct autoMelderAsynchronous {
	autoMelderAsynchronous () { Melder_asynchronous = true; }
	~autoMelderAsynchronous () { Melder_asynchronous = false; }
};

/* End of file melder.h */
#endif
