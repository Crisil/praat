/* melder_readtext.cpp
 *
 * Copyright (C) 2008-2011,2014,2015 Paul Boersma
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

/*
 * pb 2007/12/09 made MelderFile_readText ignore null bytes
 * pb 2008/11/04 split off from melder_strings.c and melder_encodings.c
 * pb 2010/03/09 support Unicode values above 0xFFFF
 * pb 2011/04/05 C++
 * pb 2011/07/03 C++
 * pb 2014/12/17 int64_t
 * pb 2015/05/26 char32
 */

#include "melder.h"
#include "UnicodeData.h"
#include "abcio.h"
#define my  me ->

char32 MelderReadText_getChar (MelderReadText me) {
	if (my string32 != NULL) {
		if (* my readPointer32 == '\0') return 0;
		return * my readPointer32 ++;
	} else {
		if (* my readPointer8 == '\0') return 0;
		if (my input8Encoding == kMelder_textInputEncoding_UTF8) {
			char32 kar1 = (char32) (char8) * my readPointer8 ++;
			if (kar1 <= 0x00007F) {
				return kar1;
			} else if (kar1 <= 0x0000DF) {
				char32 kar2 = (char32) (char8) * my readPointer8 ++;
				return ((kar1 & 0x00001F) << 6) | (kar2 & 0x00003F);
			} else if (kar1 <= 0x0000EF) {
				char32 kar2 = (char32) (char8) * my readPointer8 ++;
				char32 kar3 = (char32) (char8) * my readPointer8 ++;
				return ((kar1 & 0x00000F) << 12) | ((kar2 & 0x00003F) << 6) | (kar3 & 0x00003F);
			} else if (kar1 <= 0x0000F4) {
				char32 kar2 = (char32) (char8) * my readPointer8 ++;
				char32 kar3 = (char32) (char8) * my readPointer8 ++;
				char32 kar4 = (char32) (char8) * my readPointer8 ++;
				return ((kar1 & 0x000007) << 18) | ((kar2 & 0x00003F) << 12) | ((kar3 & 0x00003F) << 6) | (kar4 & 0x00003F);
			} else {
				return UNICODE_REPLACEMENT_CHARACTER;
			}
		} else if (my input8Encoding == kMelder_textInputEncoding_MACROMAN) {
			return Melder_decodeMacRoman [(char8) * my readPointer8 ++];
		} else if (my input8Encoding == kMelder_textInputEncoding_WINDOWS_LATIN1) {
			return Melder_decodeWindowsLatin1 [(char8) * my readPointer8 ++];
		} else {
			/* Unknown encoding. */
			return (char32) (char8) * my readPointer8 ++;
		}
	}
}

char32 * MelderReadText_readLine (MelderReadText me) {
	if (my string32 != NULL) {
		Melder_assert (my readPointer32 != NULL);
		Melder_assert (my readPointer8 == NULL);
		if (*my readPointer32 == '\0') {   // tried to read past end of file
			return NULL;
		}
		char32 *result = my readPointer32;
		char32 *newline = str32chr (result, U'\n');
		if (newline != NULL) {
			*newline = '\0';
			my readPointer32 = newline + 1;
		} else {
			my readPointer32 += str32len (result);
		}
		return result;
	} else {
		Melder_assert (my string8 != NULL);
		Melder_assert (my readPointer32 == NULL);
		Melder_assert (my readPointer8 != NULL);
		if (*my readPointer8 == '\0') {   // tried to read past end of file
			return NULL;
		}
		char *result8 = my readPointer8;
		char *newline = strchr (result8, '\n');
		if (newline != NULL) {
			*newline = '\0';
			my readPointer8 = newline + 1;
		} else {
			my readPointer8 += strlen (result8);
		}
		static char32 *text32 = NULL;
		static int64_t size = 0;
		int64_t sizeNeeded = strlen (result8) + 1;
		if (sizeNeeded > size) {
			Melder_free (text32);
			text32 = Melder_malloc_f (char32, sizeNeeded + 100);
			size = sizeNeeded + 100;
		}
		Melder_8bitToChar32_inline (result8, text32, my input8Encoding);
		return text32;
	}
}
wchar_t * MelderReadText_readLineW (MelderReadText me) {
	autostring32 string32 = MelderReadText_readLine (me);
	return Melder_str32ToWcs (string32.peek());
}

int64_t MelderReadText_getNumberOfLines (MelderReadText me) {
	int64_t n = 0;
	if (my string32 != NULL) {
		char32 *p = & my string32 [0];
		for (; *p != U'\0'; p ++) if (*p == U'\n') n ++;
		if (p - my string32 > 1 && p [-1] != U'\n') n ++;
	} else {
		char *p = & my string8 [0];
		for (; *p != '\0'; p ++) if (*p == '\n') n ++;
		if (p - my string8 > 1 && p [-1] != '\n') n ++;
	}
	return n;
}

const wchar_t * MelderReadText_getLineNumber (MelderReadText me) {
	int64_t result = 1;
	if (my string32 != NULL) {
		char32 *p = my string32;
		while (my readPointer32 - p > 0) {
			if (*p == U'\0' || *p == U'\n') result ++;
			p ++;
		}
	} else {
		char *p = my string8;
		while (my readPointer8 - p > 0) {
			if (*p == '\0' || *p == '\n') result ++;
			p ++;
		}
		return Melder_integer (result);
	}
	return Melder_integer (result);
}

static size_t fread_multi (char *buffer, size_t numberOfBytes, FILE *f) {
	off_t offset = 0;
	size_t numberOfBytesRead = 0;
	const size_t chunkSize = 1000000000;
	while (numberOfBytes > chunkSize) {
		size_t numberOfBytesReadInChunk = fread (buffer + offset, sizeof (char), chunkSize, f);
		numberOfBytesRead += numberOfBytesReadInChunk;
		if (numberOfBytesReadInChunk < chunkSize) {
			return numberOfBytesRead;
		}
		numberOfBytes -= chunkSize;
		offset += chunkSize;
	}
	size_t numberOfBytesReadInLastChunk = fread (buffer + offset, sizeof (char), numberOfBytes, f);
	numberOfBytesRead += numberOfBytesReadInLastChunk;
	return numberOfBytesRead;
}

static char32 * _MelderFile_readText32 (MelderFile file, char **string8) {
	try {
		int type = 0;   // 8-bit
		autostring32 text;
		autofile f = Melder_fopen (file, "rb");
		if (fseeko (f, 0, SEEK_END) < 0) {
			Melder_throw ("Cannot count the bytes in the file.");
		}
		Melder_assert (sizeof (off_t) >= 8);
		int64_t length = ftello (f);
		rewind (f);
		if (length >= 2) {
			int firstByte = fgetc (f), secondByte = fgetc (f);
			if (firstByte == 0xFE && secondByte == 0xFF) {
				type = 1;   // big-endian 16-bit
			} else if (firstByte == 0xFF && secondByte == 0xFE) {
				type = 2;   // little-endian 16-bit
			}
		}
		if (type == 0) {
			rewind (f);   // length and type already set correctly.
			autostring8 text8bit = Melder_malloc (char, length + 1);
			Melder_assert (text8bit.peek() != NULL);
			size_t numberOfBytesRead = fread_multi (text8bit.peek(), (size_t) length, f);
			if ((int64_t) numberOfBytesRead < length)
				Melder_throw ("The file contains ", (double) length, " bytes, but we could read only ",
					(double) numberOfBytesRead, " of them.");
			text8bit [length] = '\0';
			/*
			 * Count and repair null bytes.
			 */
			if (length > 0) {
				int64_t numberOfNullBytes = 0;
				for (char *p = & text8bit [length - 1]; (int64_t) (p - text8bit.peek()) >= 0; p --) {
					if (*p == '\0') {
						numberOfNullBytes += 1;
						/*
						 * Shift.
						 */
						for (char *q = p; (int64_t) (q - text8bit.peek()) < length; q ++) {
							*q = q [1];
						}
					}
				}
				if (numberOfNullBytes > 0) {
					Melder_warning ("Ignored ", numberOfNullBytes, " null bytes in text file ", file, ".");
				}
			}
			if (string8 != NULL) {
				*string8 = text8bit.transfer();
				(void) Melder_killReturns_inline (*string8);
				return NULL;   // OK
			} else {
				text.reset (Melder_8bitToChar32 (text8bit.peek(), 0));
			}
		} else {
			length = length / 2 - 1;   // Byte Order Mark subtracted. Length = number of UTF-16 codes
			text.reset (Melder_malloc (char32, length + 1));
			if (type == 1) {
				for (int64_t i = 0; i < length; i ++) {
					char16 kar1 = bingetu2 (f);
					if (kar1 < 0xD800) {
						text [i] = (char32) kar1;   // convert up without sign extension
					} else if (kar1 < 0xDC00) {
						length --;
						char16 kar2 = bingetu2 (f);
						if (kar2 >= 0xDC00 && kar2 <= 0xDFFF) {
							text [i] = (char32) (0x010000 +
								(char32) (((char32) kar1 & 0x0003FF) << 10) +
								(char32)  ((char32) kar2 & 0x0003FF));
						} else {
							text [i] = UNICODE_REPLACEMENT_CHARACTER;
						}
					} else if (kar1 < 0xE000) {
						text [i] = UNICODE_REPLACEMENT_CHARACTER;
					} else {
						text [i] = (char32) kar1;   // convert up without sign extension
					}
				}
			} else {
				for (int64_t i = 0; i < length; i ++) {
					char16 kar1 = bingetu2LE (f);
					if (kar1 < 0xD800) {
						text [i] = (char32) kar1;   // convert up without sign extension
					} else if (kar1 < 0xDC00) {
						length --;
						char16 kar2 = bingetu2LE (f);
						if (kar2 >= 0xDC00 && kar2 <= 0xDFFF) {
							text [i] = (char32) (0x010000 +
								(char32) (((char32) kar1 & 0x0003FF) << 10) +
								(char32)  ((char32) kar2 & 0x0003FF));
						} else {
							text [i] = UNICODE_REPLACEMENT_CHARACTER;
						}
					} else if (kar1 < 0xE000) {
						text [i] = UNICODE_REPLACEMENT_CHARACTER;
					} else if (kar1 <= 0xFFFF) {
						text [i] = (char32) kar1;   // convert up without sign extension
					} else {
						Melder_fatal ("MelderFile_readText: unsigned short greater than 0xFFFF: should not occur.");
					}
				}
			}
			text [length] = '\0';
			(void) Melder_killReturns_inline32 (text.peek());
		}
		f.close (file);
		return text.transfer();
	} catch (MelderError) {
		Melder_throw ("Error reading file ", file, ".");
	}
}

char32 * MelderFile_readText32 (MelderFile file) {
	return _MelderFile_readText32 (file, NULL);
}
wchar_t * MelderFile_readText (MelderFile file) {
	autostring32 string32 = _MelderFile_readText32 (file, NULL);
	return Melder_str32ToWcs (string32.peek());
}

MelderReadText MelderReadText_createFromFile (MelderFile file) {
	autoMelderReadText me = Melder_calloc (struct structMelderReadText, 1);
	my string32 = _MelderFile_readText32 (file, & my string8);
	if (my string32 != NULL) {
		my readPointer32 = & my string32 [0];
	} else {
		Melder_assert (my string8 != NULL);
		my readPointer8 = & my string8 [0];
		my input8Encoding = Melder_getInputEncoding ();
		if (my input8Encoding == kMelder_textInputEncoding_UTF8 ||
			my input8Encoding == kMelder_textInputEncoding_UTF8_THEN_ISO_LATIN1 ||
			my input8Encoding == kMelder_textInputEncoding_UTF8_THEN_WINDOWS_LATIN1 ||
			my input8Encoding == kMelder_textInputEncoding_UTF8_THEN_MACROMAN)
		{
			if (Melder_str8IsValidUtf8 (my string8)) {
				my input8Encoding = kMelder_textInputEncoding_UTF8;
			} else if (my input8Encoding == kMelder_textInputEncoding_UTF8) {
				Melder_throw ("Text is not valid UTF-8; please try a different text input encoding.");
			} else if (my input8Encoding == kMelder_textInputEncoding_UTF8_THEN_ISO_LATIN1) {
				my input8Encoding = kMelder_textInputEncoding_ISO_LATIN1;
			} else if (my input8Encoding == kMelder_textInputEncoding_UTF8_THEN_WINDOWS_LATIN1) {
				my input8Encoding = kMelder_textInputEncoding_WINDOWS_LATIN1;
			} else if (my input8Encoding == kMelder_textInputEncoding_UTF8_THEN_MACROMAN) {
				my input8Encoding = kMelder_textInputEncoding_MACROMAN;
			}
		}
	}
	return me.transfer();
}

MelderReadText MelderReadText_createFromString (const wchar_t *string);

void MelderReadText_delete (MelderReadText me) {
	if (me == NULL) return;
	Melder_free (my string32);
	Melder_free (my string8);
	Melder_free (me);
}

/* End of file melder_readtext.cpp */
