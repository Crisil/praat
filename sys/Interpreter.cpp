/* Interpreter.cpp
 *
 * Copyright (C) 1993-2011,2013,2014,2015 Paul Boersma
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
 * pb 2002/03/07 GPL
 * pb 2002/03/25 option menus
 * pb 2002/06/04 include the script compiler
 * pb 2002/09/26 removed bug: crashed if a line in a form contained only the word "comment"
 * pb 2002/11/25 Melder_double
 * pb 2002/12/10 include files
 * pb 2002/12/14 more informative error messages
 * pb 2003/05/19 Melder_atof
 * pb 2003/07/15 assert
 * pb 2003/07/19 if undefined fails
 * pb 2004/10/16 C++ compatible structs
 * pb 2004/12/06 made Interpreter_getArgumentsFromDialog resistant to changes in the script while the dialog is up
 * pb 2005/01/01 there can be spaces before the "form" statement
 * pb 2005/11/26 allow mixing of "option" and "button", as in Ui.c
 * pb 2006/01/11 local variables
 * pb 2007/02/05 preferencesDirectory$, homeDirectory$, temporaryDirectory$
 * pb 2007/04/02 allow comments (with '#' or ';' or empty lines) in forms
 * pb 2007/04/19 allow comments with '!' in forms
 * pb 2007/05/24 some wchar_t
 * pb 2007/06/09 wchar_t
 * pb 2007/08/12 more wchar_t
 * pb 2007/11/30 removed bug: allowed long arguments to the "call" statement (thanks to Ingmar Steiner)
 * pb 2007/12/10 predefined numeric variables macintosh/windows/unix
 * pb 2008/04/30 new Formula API
 * pb 2008/05/01 arrays
 * pb 2008/05/15 praatVersion, praatVersion$
 * pb 2009/01/04 Interpreter_voidExpression
 * pb 2009/01/17 arguments to UiForm callbacks
 * pb 2009/01/20 pause forms
 * pb 2009/03/17 split up structPraat
 * pb 2009/12/22 invokingButtonTitle
 * pb 2010/04/30 guard against leading nonbreaking spaces
 * pb 2011/05/14 C++
 * pb 2015/05/30 char32
 */

#include <ctype.h>
#include "Interpreter.h"
#include "praatP.h"
extern structMelderDir praatDir;
#include "praat_script.h"
#include "Formula.h"
#include "praat_version.h"
#include "UnicodeData.h"

#define Interpreter_WORD 1
#define Interpreter_REAL 2
#define Interpreter_POSITIVE 3
#define Interpreter_INTEGER 4
#define Interpreter_NATURAL 5
#define Interpreter_BOOLEAN 6
#define Interpreter_SENTENCE 7
#define Interpreter_TEXT 8
#define Interpreter_CHOICE 9
#define Interpreter_OPTIONMENU 10
#define Interpreter_BUTTON 11
#define Interpreter_OPTION 12
#define Interpreter_COMMENT 13

Thing_implement (InterpreterVariable, SimpleString, 0);

void structInterpreterVariable :: v_destroy () {
	Melder_free (string);
	Melder_free (stringValue);
	NUMmatrix_free (numericArrayValue. data, 1, 1);
	InterpreterVariable_Parent :: v_destroy ();
}

static InterpreterVariable InterpreterVariable_create (const char32 *key) {
	try {
		if (key [0] == U'e' && key [1] == U'\0')
			Melder_throw ("You cannot use 'e' as the name of a variable (e is the constant 2.71...).");
		if (key [0] == U'p' && key [1] == U'i' && key [2] == U'\0')
			Melder_throw ("You cannot use 'pi' as the name of a variable (pi is the constant 3.14...).");
		if (key [0] == U'u' && key [1] == U'n' && key [2] == U'd' && key [3] == U'e' && key [4] == U'f' && key [5] == U'i' &&
			key [6] == U'n' && key [7] == U'e' && key [8] == U'd' && key [9] == U'\0')
			Melder_throw ("You cannot use 'undefined' as the name of a variable.");
		autoInterpreterVariable me = Thing_new (InterpreterVariable);
		my string = Melder_str32dup (key);
		return me.transfer();
	} catch (MelderError) {
		Melder_throw ("Interpreter variable not created.");
	}
}

Thing_implement (Interpreter, Thing, 0);

void structInterpreter :: v_destroy () {
	Melder_free (environmentName);
	for (int ipar = 1; ipar <= Interpreter_MAXNUM_PARAMETERS; ipar ++)
		Melder_free (arguments [ipar]);
	forget (variables);
	Interpreter_Parent :: v_destroy ();
}

Interpreter Interpreter_create (char32 *environmentName, ClassInfo editorClass) {
	try {
		autoInterpreter me = Thing_new (Interpreter);
		my variables = SortedSetOfString32_create ();
		my environmentName = Melder_str32dup (environmentName);
		my editorClass = editorClass;
		return me.transfer();
	} catch (MelderError) {
		Melder_throw ("Interpreter not created.");
	}
}

Interpreter Interpreter_createFromEnvironment (Editor editor) {
	if (editor == NULL) return Interpreter_create (NULL, NULL);
	return Interpreter_create (Melder_peekWcsToStr32 (editor -> name), editor -> classInfo);
}

void Melder_includeIncludeFiles (char32 **text) {
	for (int depth = 0; ; depth ++) {
		char32 *head = *text;
		long numberOfIncludes = 0;
		if (depth > 10)
			Melder_throw ("Include files nested too deep. Probably cyclic.");
		for (;;) {
			char32 *includeLocation, *includeFileName, *tail, *newText;
			long headLength, includeTextLength, newLength;
			/*
				Look for an include statement. If not found, we have finished.
			 */
			includeLocation = str32nequ (head, U"include ", 8) ? head : str32str (head, U"\ninclude ");
			if (includeLocation == NULL) break;
			if (includeLocation != head) includeLocation += 1;
			numberOfIncludes += 1;
			/*
				Separate out the head.
			 */
			*includeLocation = U'\0';
			/*
				Separate out the name of the include file.
			 */
			includeFileName = includeLocation + 8;
			while (*includeFileName == U' ' || *includeFileName == U'\t') includeFileName ++;
			tail = includeFileName;
			while (*tail != U'\n' && *tail != U'\0') tail ++;
			if (*tail == U'\n') {
				*tail = U'\0';
				tail += 1;
			}
			/*
				Get the contents of the include file.
			 */
			structMelderFile includeFile = { 0 };
			Melder_relativePathToFile (includeFileName, & includeFile);
			autostring32 includeText;
			try {
				includeText.reset (MelderFile_readText32 (& includeFile));
			} catch (MelderError) {
				Melder_throw ("Include file ", & includeFile, " not read.");
			}
			/*
				Construct the new text.
			 */
			headLength = (head - *text) + str32len (head);
			includeTextLength = str32len (includeText.peek());
			newLength = headLength + includeTextLength + 1 + str32len (tail);
			newText = Melder_malloc (char32, newLength + 1);
			str32cpy (newText, *text);
			str32cpy (newText + headLength, includeText.peek());
			str32cpy (newText + headLength + includeTextLength, U"\n");
			str32cpy (newText + headLength + includeTextLength + 1, tail);
			/*
				Replace the old text with the new. This will work even within an autostring.
			 */
			Melder_free (*text);
			*text = newText;
			/*
				Cycle.
			 */
			head = *text + headLength + includeTextLength + 1;
		}
		if (numberOfIncludes == 0) break;
	}
}

long Interpreter_readParameters (Interpreter me, char32 *text) {
	char32 *formLocation = NULL;
	long npar = 0;
	my dialogTitle [0] = U'\0';
	/*
	 * Look for a "form" line.
	 */
	{// scope
		char32 *p = text;
		for (;;) {
			while (*p == ' ' || *p == '\t') p ++;
			if (str32nequ (p, U"form ", 5)) {
				formLocation = p;
				break;
			}
			while (*p != '\0' && *p != '\n') p ++;
			if (*p == '\0') break;
			p ++;   /* Skip newline symbol. */
		}
	}
	/*
	 * If there is no "form" line, there are no parameters.
	 */
	if (formLocation) {
		char32 *dialogTitle = formLocation + 5, *newLine;
		while (*dialogTitle == U' ' || *dialogTitle == U'\t') dialogTitle ++;
		newLine = str32chr (dialogTitle, U'\n');
		if (newLine) *newLine = U'\0';
		str32cpy (my dialogTitle, dialogTitle);
		if (newLine) *newLine = U'\n';
		my numberOfParameters = 0;
		while (newLine) {
			char32 *line = newLine + 1, *p;
			int type = 0;
			while (*line == U' ' || *line == U'\t') line ++;
			while (*line == U'#' || *line == U';' || *line == U'!' || *line == U'\n') {
				newLine = str32chr (line, U'\n');
				if (newLine == NULL)
					Melder_throw ("Unfinished form.");
				line = newLine + 1;
				while (*line == U' ' || *line == U'\t') line ++;
			}
			if (str32nequ (line, U"endform", 7)) break;
			if (str32nequ (line, U"word ", 5)) { type = Interpreter_WORD; p = line + 5; }
			else if (str32nequ (line, U"real ", 5)) { type = Interpreter_REAL; p = line + 5; }
			else if (str32nequ (line, U"positive ", 9)) { type = Interpreter_POSITIVE; p = line + 9; }
			else if (str32nequ (line, U"integer ", 8)) { type = Interpreter_INTEGER; p = line + 8; }
			else if (str32nequ (line, U"natural ", 8)) { type = Interpreter_NATURAL; p = line + 8; }
			else if (str32nequ (line, U"boolean ", 8)) { type = Interpreter_BOOLEAN; p = line + 8; }
			else if (str32nequ (line, U"sentence ", 9)) { type = Interpreter_SENTENCE; p = line + 9; }
			else if (str32nequ (line, U"text ", 5)) { type = Interpreter_TEXT; p = line + 5; }
			else if (str32nequ (line, U"choice ", 7)) { type = Interpreter_CHOICE; p = line + 7; }
			else if (str32nequ (line, U"optionmenu ", 11)) { type = Interpreter_OPTIONMENU; p = line + 11; }
			else if (str32nequ (line, U"button ", 7)) { type = Interpreter_BUTTON; p = line + 7; }
			else if (str32nequ (line, U"option ", 7)) { type = Interpreter_OPTION; p = line + 7; }
			else if (str32nequ (line, U"comment ", 8)) { type = Interpreter_COMMENT; p = line + 8; }
			else {
				newLine = str32chr (line, U'\n');
				if (newLine) *newLine = U'\0';
				Melder_error_ ("Unknown parameter type:\n\"", line, "\".");
				if (newLine) *newLine = U'\n';
				throw MelderError ();
				return 0;
			}
			/*
				Example:
					form Something
						real Time_(s) 3.14 (= pi)
						choice Colour 2
							button Red
							button Green
							button Blue
					endform
				my parameters [1] := "Time_(s)"
				my parameters [2] := "Colour"
				my parameters [3] := ""
				my parameters [4] := ""
				my parameters [5] := ""
				my arguments [1] := "3.14 (= pi)"
				my arguments [2] := "2"
				my arguments [3] := "Red"   (funny, but needed in Interpreter_getArgumentsFromString)
				my arguments [4] := "Green"
				my arguments [5] := "Blue"
			*/
			if (type <= Interpreter_OPTIONMENU) {
				while (*p == U' ' || *p == U'\t') p ++;
				if (*p == U'\n' || *p == U'\0')
					Melder_throw ("Missing parameter:\n\"", line, "\".");
				char32 *q = my parameters [++ my numberOfParameters];
				while (*p != U' ' && *p != U'\t' && *p != U'\n' && *p != U'\0') * (q ++) = * (p ++);
				*q = U'\0';
				npar ++;
			} else {
				my parameters [++ my numberOfParameters] [0] = U'\0';
			}
			while (*p == U' ' || *p == U'\t') p ++;
			newLine = str32chr (p, U'\n');
			if (newLine) *newLine = U'\0';
			Melder_free (my arguments [my numberOfParameters]);
			my arguments [my numberOfParameters] = Melder_str32dup_f (p);
			if (newLine) *newLine = U'\n';
			my types [my numberOfParameters] = type;
		}
	} else {
		npar = my numberOfParameters = 0;
	}
	return npar;
}

UiForm Interpreter_createForm (Interpreter me, GuiWindow parent, const wchar_t *path,
	void (*okCallback) (UiForm, int, Stackel, const wchar_t *, Interpreter, const wchar_t *, bool, void *), void *okClosure,
	bool selectionOnly)
{
	UiForm form = UiForm_create (parent,
		Melder_str32cat (selectionOnly ? U"Run script (selection only): " : U"Run script: ", my dialogTitle),
		okCallback, okClosure, NULL, NULL);
	Any radio = NULL;
	if (path) UiForm_addText (form, L"$file", path);
	for (int ipar = 1; ipar <= my numberOfParameters; ipar ++) {
		/*
		 * Convert underscores to spaces.
		 */
		char32 parameter [100], *p = & parameter [0];
		str32cpy (parameter, my parameters [ipar]);
		while (*p) { if (*p == '_') *p = ' '; p ++; }
		switch (my types [ipar]) {
			case Interpreter_WORD:
				UiForm_addWord (form, parameter, my arguments [ipar]); break;
			case Interpreter_REAL:
				UiForm_addReal (form, parameter, my arguments [ipar]); break;
			case Interpreter_POSITIVE:
				UiForm_addPositive (form, parameter, my arguments [ipar]); break;
			case Interpreter_INTEGER:
				UiForm_addInteger (form, parameter, my arguments [ipar]); break;
			case Interpreter_NATURAL:
				UiForm_addNatural (form, parameter, my arguments [ipar]); break;
			case Interpreter_BOOLEAN:
				UiForm_addBoolean (form, parameter, my arguments [ipar] [0] == '1' ||
					my arguments [ipar] [0] == 'y' || my arguments [ipar] [0] == 'Y' ||
					(my arguments [ipar] [0] == 'o' && my arguments [ipar] [1] == 'n')); break;
			case Interpreter_SENTENCE:
				UiForm_addSentence (form, parameter, my arguments [ipar]); break;
			case Interpreter_TEXT:
				UiForm_addText (form, parameter, my arguments [ipar]); break;
			case Interpreter_CHOICE:
				radio = UiForm_addRadio (form, parameter, a32tol (my arguments [ipar])); break;
			case Interpreter_OPTIONMENU:
				radio = UiForm_addOptionMenu (form, parameter, a32tol (my arguments [ipar])); break;
			case Interpreter_BUTTON:
				if (radio) UiRadio_addButton (radio, my arguments [ipar]); break;
			case Interpreter_OPTION:
				if (radio) UiOptionMenu_addButton (radio, my arguments [ipar]); break;
			case Interpreter_COMMENT:
				UiForm_addLabel (form, parameter, my arguments [ipar]); break;
			default:
				UiForm_addWord (form, parameter, my arguments [ipar]); break;
		}
		/*
		 * Strip parentheses and colon off parameter name.
		 */
		if ((p = str32chr (my parameters [ipar], U'(')) != NULL) {
			*p = U'\0';
			if (p - my parameters [ipar] > 0 && p [-1] == U'_') p [-1] = U'\0';
		}
		p = my parameters [ipar];
		if (*p != U'\0' && p [str32len (p) - 1] == U':') p [str32len (p) - 1] = U'\0';
	}
	UiForm_finish (form);
	return form;
}

void Interpreter_getArgumentsFromDialog (Interpreter me, Any dialog) {
	for (int ipar = 1; ipar <= my numberOfParameters; ipar ++) {
		char32 parameter [100], *p;
		/*
		 * Strip parentheses and colon off parameter name.
		 */
		if ((p = str32chr (my parameters [ipar], U'(')) != NULL) {
			*p = U'\0';
			if (p - my parameters [ipar] > 0 && p [-1] == U'_') p [-1] = U'\0';
		}
		p = my parameters [ipar];
		if (*p != U'\0' && p [str32len (p) - 1] == U':') p [str32len (p) - 1] = U'\0';
		/*
		 * Convert underscores to spaces.
		 */
		str32cpy (parameter, my parameters [ipar]);
		p = & parameter [0]; while (*p) { if (*p == U'_') *p = U' '; p ++; }
		switch (my types [ipar]) {
			case Interpreter_REAL:
			case Interpreter_POSITIVE: {
				double value = UiForm_getReal_check (dialog, parameter);
				Melder_free (my arguments [ipar]);
				my arguments [ipar] = Melder_calloc_f (char32, 40);
				str32cpy (my arguments [ipar], Melder32_double (value));
				break;
			}
			case Interpreter_INTEGER:
			case Interpreter_NATURAL:
			case Interpreter_BOOLEAN: {
				long value = UiForm_getInteger (dialog, parameter);
				Melder_free (my arguments [ipar]);
				my arguments [ipar] = Melder_calloc_f (char32, 40);
				str32cpy (my arguments [ipar], Melder32_integer (value));
				break;
			}
			case Interpreter_CHOICE:
			case Interpreter_OPTIONMENU: {
				long integerValue = 0;
				char32 *stringValue = NULL;
				integerValue = UiForm_getInteger (dialog, parameter);
				stringValue = UiForm_getString (dialog, parameter);
				Melder_free (my arguments [ipar]);
				my arguments [ipar] = Melder_calloc_f (char32, 40);
				str32cpy (my arguments [ipar], Melder32_integer (integerValue));
				str32cpy (my choiceArguments [ipar], stringValue);
				break;
			}
			case Interpreter_BUTTON:
			case Interpreter_OPTION:
			case Interpreter_COMMENT:
				break;
			default: {
				char32 *value = UiForm_getString (dialog, parameter);
				Melder_free (my arguments [ipar]);
				my arguments [ipar] = Melder_str32dup_f (value);
				break;
			}
		}
	}
}

void Interpreter_getArgumentsFromString (Interpreter me, const char32 *arguments) {
	int size = my numberOfParameters;
	long length = str32len (arguments);
	while (size >= 1 && my parameters [size] [0] == U'\0')
		size --;   /* Ignore fields without a variable name (button, comment). */
	for (int ipar = 1; ipar <= size; ipar ++) {
		char32 *p = my parameters [ipar];
		/*
		 * Ignore buttons and comments again.
		 */
		if (! *p) continue;
		/*
		 * Strip parentheses and colon off parameter name.
		 */
		if ((p = str32chr (p, U'(')) != NULL) {
			*p = U'\0';
			if (p - my parameters [ipar] > 0 && p [-1] == U'_') p [-1] = U'\0';
		}
		p = my parameters [ipar];
		if (*p != U'\0' && p [str32len (p) - 1] == U':') p [str32len (p) - 1] = U'\0';
	}
	for (int ipar = 1; ipar < size; ipar ++) {
		int ichar = 0;
		/*
		 * Ignore buttons and comments again. The buttons will keep their labels as "arguments".
		 */
		if (my parameters [ipar] [0] == U'\0') continue;
		Melder_free (my arguments [ipar]);   // erase the current values, probably the default values
		my arguments [ipar] = Melder_calloc_f (char32, length + 1);   // replace with the actual arguments
		/*
		 * Skip spaces until next argument.
		 */
		while (*arguments == U' ' || *arguments == U'\t') arguments ++;
		/*
		 * The argument is everything up to the next space, or, if that starts with a double quote,
		 * everything between this quote and the matching double quote;
		 * in this case, the argument can represent a double quote by a sequence of two double quotes.
		 * Example: the string
		 *     "I said ""hello"""
		 * will be passed to the dialog as a single argument containing the text
		 *     I said "hello"
		 */
		if (*arguments == U'\"') {
			arguments ++;   // do not include leading double quote
			for (;;) {
				if (*arguments == U'\0')
					Melder_throw ("Missing matching quote.");
				if (*arguments == U'\"' && * ++ arguments != U'\"') break;   // remember second quote
				my arguments [ipar] [ichar ++] = *arguments ++;
			}
		} else {
			while (*arguments != U' ' && *arguments != U'\t' && *arguments != U'\0')
				my arguments [ipar] [ichar ++] = *arguments ++;
		}
		my arguments [ipar] [ichar] = U'\0';   // trailing null byte
	}
	/* The last item is handled separately, because it consists of the rest of the line.
	 * Leading spaces are skipped, but trailing spaces are included.
	 */
	if (size > 0) {
		while (*arguments == U' ' || *arguments == U'\t') arguments ++;
		Melder_free (my arguments [size]);
		my arguments [size] = Melder_str32dup_f (arguments);
	}
	/*
	 * Convert booleans and choices to numbers.
	 */
	for (int ipar = 1; ipar <= size; ipar ++) {
		if (my types [ipar] == Interpreter_BOOLEAN) {
			char32 *arg = & my arguments [ipar] [0];
			if (str32equ (arg, U"1") || str32equ (arg, U"yes") || str32equ (arg, U"on") ||
			    str32equ (arg, U"Yes") || str32equ (arg, U"On") || str32equ (arg, U"YES") || str32equ (arg, U"ON"))
			{
				str32cpy (arg, U"1");
			} else if (str32equ (arg, U"0") || str32equ (arg, U"no") || str32equ (arg, U"off") ||
			    str32equ (arg, U"No") || str32equ (arg, U"Off") || str32equ (arg, U"NO") || str32equ (arg, U"OFF"))
			{
				str32cpy (arg, U"0");
			} else {
				Melder_throw ("Unknown value \"", arg, "\" for boolean \"", my parameters [ipar], "\".");
			}
		} else if (my types [ipar] == Interpreter_CHOICE) {
			int jpar;
			char32 *arg = & my arguments [ipar] [0];
			for (jpar = ipar + 1; jpar <= my numberOfParameters; jpar ++) {
				if (my types [jpar] != Interpreter_BUTTON && my types [jpar] != Interpreter_OPTION)
					Melder_throw ("Unknown value \"", arg, "\" for choice \"", my parameters [ipar], "\".");
				if (str32equ (my arguments [jpar], arg)) {   // the button labels are in the arguments; see Interpreter_readParameters
					str32cpy (arg, Melder32_integer (jpar - ipar));
					str32cpy (my choiceArguments [ipar], my arguments [jpar]);
					break;
				}
			}
			if (jpar > my numberOfParameters)
				Melder_throw ("Unknown value \"", arg, "\" for choice \"", my parameters [ipar], "\".");
		} else if (my types [ipar] == Interpreter_OPTIONMENU) {
			int jpar;
			char32 *arg = & my arguments [ipar] [0];
			for (jpar = ipar + 1; jpar <= my numberOfParameters; jpar ++) {
				if (my types [jpar] != Interpreter_OPTION && my types [jpar] != Interpreter_BUTTON)
					Melder_throw ("Unknown value \"", arg, "\" for option menu \"", my parameters [ipar], "\".");
				if (str32equ (my arguments [jpar], arg)) {
					str32cpy (arg, Melder32_integer (jpar - ipar));
					str32cpy (my choiceArguments [ipar], my arguments [jpar]);
					break;
				}
			}
			if (jpar > my numberOfParameters)
				Melder_throw ("Unknown value \"", arg, "\" for option menu \"", my parameters [ipar], "\".");
		}
	}
}

void Interpreter_getArgumentsFromArgs (Interpreter me, int narg, Stackel args) {
	trace ("%d arguments", narg);
	int size = my numberOfParameters;
	while (size >= 1 && my parameters [size] [0] == '\0')
		size --;   // ignore trailing fields without a variable name (button, comment)
	for (int ipar = 1; ipar <= size; ipar ++) {
		char32 *p = my parameters [ipar];
		/*
		 * Ignore buttons and comments again.
		 */
		if (! *p) continue;
		/*
		 * Strip parentheses and colon off parameter name.
		 */
		if ((p = str32chr (p, U'(')) != NULL) {
			*p = U'\0';
			if (p - my parameters [ipar] > 0 && p [-1] == U'_') p [-1] = U'\0';
		}
		p = my parameters [ipar];
		if (*p != U'\0' && p [str32len (p) - 1] == U':') p [str32len (p) - 1] = U'\0';
	}
	int iarg = 0;
	for (int ipar = 1; ipar <= size; ipar ++) {
		/*
		 * Ignore buttons and comments again. The buttons will keep their labels as "arguments".
		 */
		if (my parameters [ipar] [0] == U'\0') continue;
		Melder_free (my arguments [ipar]);   // erase the current values, probably the default values
		if (iarg == narg)
			Melder_throw ("Found ", narg, " arguments but expected more.");
		Stackel arg = & args [++ iarg];
		my arguments [ipar] =
			arg -> which == Stackel_NUMBER ? Melder_str32dup (Melder32_double (arg -> number)) :
			arg -> which == Stackel_STRING ? Melder_str32dup (arg -> string) : NULL;   // replace with the actual arguments
		Melder_assert (my arguments [ipar] != NULL);
	}
	if (iarg < narg)
		Melder_throw ("Found ", narg, " arguments but expected only ", iarg, ".");
	/*
	 * Convert booleans and choices to numbers.
	 */
	for (int ipar = 1; ipar <= size; ipar ++) {
		if (my types [ipar] == Interpreter_BOOLEAN) {
			char32 *arg = & my arguments [ipar] [0];
			if (str32equ (arg, U"1") || str32equ (arg, U"yes") || str32equ (arg, U"on") ||
			    str32equ (arg, U"Yes") || str32equ (arg, U"On") || str32equ (arg, U"YES") || str32equ (arg, U"ON"))
			{
				str32cpy (arg, U"1");
			} else if (str32equ (arg, U"0") || str32equ (arg, U"no") || str32equ (arg, U"off") ||
			    str32equ (arg, U"No") || str32equ (arg, U"Off") || str32equ (arg, U"NO") || str32equ (arg, U"OFF"))
			{
				str32cpy (arg, U"0");
			} else {
				Melder_throw ("Unknown value \"", arg, "\" for boolean \"", my parameters [ipar], "\".");
			}
		} else if (my types [ipar] == Interpreter_CHOICE) {
			int jpar;
			char32 *arg = & my arguments [ipar] [0];
			for (jpar = ipar + 1; jpar <= my numberOfParameters; jpar ++) {
				if (my types [jpar] != Interpreter_BUTTON && my types [jpar] != Interpreter_OPTION)
					Melder_throw ("Unknown value \"", arg, "\" for choice \"", my parameters [ipar], "\".");
				if (str32equ (my arguments [jpar], arg)) {   // the button labels are in the arguments; see Interpreter_readParameters
					str32cpy (arg, Melder32_integer (jpar - ipar));
					str32cpy (my choiceArguments [ipar], my arguments [jpar]);
					break;
				}
			}
			if (jpar > my numberOfParameters)
				Melder_throw ("Unknown value \"", arg, "\" for choice \"", my parameters [ipar], "\".");
		} else if (my types [ipar] == Interpreter_OPTIONMENU) {
			int jpar;
			char32 *arg = & my arguments [ipar] [0];
			for (jpar = ipar + 1; jpar <= my numberOfParameters; jpar ++) {
				if (my types [jpar] != Interpreter_OPTION && my types [jpar] != Interpreter_BUTTON)
					Melder_throw ("Unknown value \"", arg, "\" for option menu \"", my parameters [ipar], "\".");
				if (str32equ (my arguments [jpar], arg)) {
					str32cpy (arg, Melder32_integer (jpar - ipar));
					str32cpy (my choiceArguments [ipar], my arguments [jpar]);
					break;
				}
			}
			if (jpar > my numberOfParameters)
				Melder_throw ("Unknown value \"", arg, "\" for option menu \"", my parameters [ipar], "\".");
		}
	}
}

static void Interpreter_addNumericVariable (Interpreter me, const char32 *key, double value) {
	autoInterpreterVariable variable = InterpreterVariable_create (key);
	variable -> numericValue = value;
	Collection_addItem (my variables, variable.transfer());
}

static void Interpreter_addStringVariable (Interpreter me, const char32 *key, const char32 *value) {
	autoInterpreterVariable variable = InterpreterVariable_create (key);
	variable -> stringValue = Melder_str32dup (value);
	Collection_addItem (my variables, variable.transfer());
}

InterpreterVariable Interpreter_hasVariable (Interpreter me, const char32 *key) {
	Melder_assert (key != NULL);
	long variableNumber = SortedSetOfString32_lookUp (my variables,
		key [0] == U'.' ? Melder_str32cat (my procedureNames [my callDepth], key) : key);
	return variableNumber ? (InterpreterVariable) my variables -> item [variableNumber] : NULL;
}

InterpreterVariable Interpreter_lookUpVariable (Interpreter me, const char32 *key) {
	Melder_assert (key != NULL);
	const char32 *variableNameIncludingProcedureName =
		key [0] == U'.' ? Melder_str32cat (my procedureNames [my callDepth], key) : key;
	long variableNumber = SortedSetOfString32_lookUp (my variables, variableNameIncludingProcedureName);
	if (variableNumber) return (InterpreterVariable) my variables -> item [variableNumber];   // already exists
	/*
	 * The variable doesn't yet exist: create a new one.
	 */
	autoInterpreterVariable variable = InterpreterVariable_create (variableNameIncludingProcedureName);
	InterpreterVariable variable_ref = variable.peek();
	Collection_addItem (my variables, variable.transfer());
	return variable_ref;
}

static long lookupLabel (Interpreter me, const char32 *labelName) {
	for (long ilabel = 1; ilabel <= my numberOfLabels; ilabel ++)
		if (str32equ (labelName, my labelNames [ilabel]))
			return ilabel;
	Melder_throw ("Unknown label \"", labelName, "\".");
}

static bool isCommand (const char32 *p) {
	/*
	 * Things that start with "nowarn", "noprogress", or "nocheck" are commands.
	 */
	if (p [0] == U'n' && p [1] == U'o' &&
		(str32nequ (p + 2, U"warn ", 5) || str32nequ (p + 2, U"progress ", 9) || str32nequ (p + 2, U"check ", 6))) return true;
	if (str32nequ (p, U"demo ", 5)) return true;
	/*
	 * Otherwise, things that start with lower case are formulas.
	 */
	if (! isupper ((int) *p)) return false;
	/*
	 * The remaining possibility is things that start with upper case.
	 * If they contain an underscore, they are object names, hence we must have a formula.
	 * Otherwise, we have a command.
	 */
	while (isalnum ((int) *p)) p ++;
	return *p != '_';
}

static void parameterToVariable (Interpreter me, int type, const char32 *in_parameter, int ipar) {
	char32 parameter [200];
	Melder_assert (type != 0);
	str32cpy (parameter, in_parameter);
	if (type >= Interpreter_REAL && type <= Interpreter_BOOLEAN) {
		Interpreter_addNumericVariable (me, parameter, Melder_a32tof (my arguments [ipar]));
	} else if (type == Interpreter_CHOICE || type == Interpreter_OPTIONMENU) {
		Interpreter_addNumericVariable (me, parameter, Melder_a32tof (my arguments [ipar]));
		str32cpy (parameter + str32len (parameter), U"$");
		Interpreter_addStringVariable (me, parameter, my choiceArguments [ipar]);
	} else if (type == Interpreter_BUTTON || type == Interpreter_OPTION || type == Interpreter_COMMENT) {
		/* Do not add a variable. */
	} else {
		str32cpy (parameter + str32len (parameter), U"$");
		Interpreter_addStringVariable (me, parameter, my arguments [ipar]);
	}
}

void Interpreter_run (Interpreter me, char32 *text) {
	autoNUMvector <char32 *> lines;   // not autostringvector, because the elements are reference copies
	long lineNumber = 0;
	bool assertionFailed = false;
	try {
		static MelderString valueString = { 0 };   // to divert the info
		static MelderString32 assertErrorString = { 0 };
		char32 *command = text;
		autoMelderString32 command2;
		autoMelderString32 buffer;
		long numberOfLines = 0, assertErrorLineNumber = 0, callStack [1 + Interpreter_MAX_CALL_DEPTH];
		int atLastLine = FALSE, fromif = FALSE, fromendfor = FALSE, callDepth = 0, chopped = 0, ipar;
		my callDepth = 0;
		/*
		 * The "environment" is NULL if we are in the Praat shell, or an editor otherwise.
		 */
		if (my editorClass) {
			praatP. editor = praat_findEditorFromString (my environmentName);
		} else {
			praatP. editor = NULL;
		}
		/*
		 * Start.
		 */
		my running = true;
		/*
		 * Count lines and set the newlines to zero.
		 */
		while (! atLastLine) {
			char32 *endOfLine = command;
			while (*endOfLine != U'\n' && *endOfLine != U'\0') endOfLine ++;
			if (*endOfLine == U'\0') atLastLine = TRUE;
			*endOfLine = U'\0';
			numberOfLines ++;
			command = endOfLine + 1;
		}
		/*
		 * Remember line starts and labels.
		 */
		lines.reset (1, numberOfLines);
		for (lineNumber = 1, command = text; lineNumber <= numberOfLines; lineNumber ++, command += str32len (command) + 1 + chopped) {
			int length;
			while (*command == U' ' || *command == U'\t' || *command == UNICODE_NO_BREAK_SPACE) command ++;   // nbsp can occur for scripts copied from the manual
			length = str32len (command);
			/*
			 * Chop trailing spaces?
			 */
			/*chopped = 0;
			while (length > 0) { char kar = command [-- length]; if (kar != ' ' && kar != '\t') break; command [length] = '\0'; chopped ++; }*/
			lines [lineNumber] = command;
			if (str32nequ (command, U"label ", 6)) {
				int ilabel;
				for (ilabel = 1; ilabel <= my numberOfLabels; ilabel ++)
					if (str32equ (command + 6, my labelNames [ilabel]))
						Melder_throw ("Duplicate label \"", command + 6, "\".");
				if (my numberOfLabels >= Interpreter_MAXNUM_LABELS)
					Melder_throw ("Too many labels.");
				str32ncpy (my labelNames [++ my numberOfLabels], command + 6, 1+Interpreter_MAX_LABEL_LENGTH);
				my labelNames [my numberOfLabels] [Interpreter_MAX_LABEL_LENGTH] = U'\0';
				my labelLines [my numberOfLabels] = lineNumber;
			}
		}
		/*
		 * Connect continuation lines.
		 */
		trace ("connect continuation lines");
		for (lineNumber = numberOfLines; lineNumber >= 2; lineNumber --) {
			char32 *line = lines [lineNumber];
			if (line [0] == U'.' && line [1] == U'.' && line [2] == U'.') {
				char32 *previous = lines [lineNumber - 1];
				MelderString32_copy (& command2, line + 3);
				MelderString32_get (& command2, previous + str32len (previous));
				static char32 emptyLine [] = { U'\0' };
				lines [lineNumber] = emptyLine;
			}
		}
		/*
		 * Copy the parameter names and argument values into the array of variables.
		 */
		forget (my variables);
		my variables = SortedSetOfString32_create ();
		for (ipar = 1; ipar <= my numberOfParameters; ipar ++) {
			char32 parameter [200];
			/*
			 * Create variable names as-are and variable names without capitals.
			 */
			str32cpy (parameter, my parameters [ipar]);
			parameterToVariable (me, my types [ipar], parameter, ipar);
			if (parameter [0] >= U'A' && parameter [0] <= U'Z') {
				parameter [0] = (char32) tolower ((int) parameter [0]);
				parameterToVariable (me, my types [ipar], parameter, ipar);
			}
		}
		/*
		 * Initialize some variables.
		 */
		Interpreter_addStringVariable (me, U"newline$", U"\n");
		Interpreter_addStringVariable (me, U"tab$", U"\t");
		Interpreter_addStringVariable (me, U"shellDirectory$", Melder_peekWcsToStr32 (Melder_getShellDirectory ()));
		structMelderDir dir = { { 0 } }; Melder_getDefaultDir (& dir);
		Interpreter_addStringVariable (me, U"defaultDirectory$", Melder_peekWcsToStr32 (Melder_dirToPath (& dir)));
		Interpreter_addStringVariable (me, U"preferencesDirectory$", Melder_peekWcsToStr32 (Melder_dirToPath (& praatDir)));
		Melder_getHomeDir (& dir);
		Interpreter_addStringVariable (me, U"homeDirectory$", Melder_peekWcsToStr32 (Melder_dirToPath (& dir)));
		Melder_getTempDir (& dir);
		Interpreter_addStringVariable (me, U"temporaryDirectory$", Melder_peekWcsToStr32 (Melder_dirToPath (& dir)));
		#if defined (macintosh)
			Interpreter_addNumericVariable (me, U"macintosh", 1);
			Interpreter_addNumericVariable (me, U"windows", 0);
			Interpreter_addNumericVariable (me, U"unix", 0);
		#elif defined (_WIN32)
			Interpreter_addNumericVariable (me, U"macintosh", 0);
			Interpreter_addNumericVariable (me, U"windows", 1);
			Interpreter_addNumericVariable (me, U"unix", 0);
		#elif defined (UNIX)
			Interpreter_addNumericVariable (me, U"macintosh", 0);
			Interpreter_addNumericVariable (me, U"windows", 0);
			Interpreter_addNumericVariable (me, U"unix", 1);
		#else
			Interpreter_addNumericVariable (me, U"macintosh", 0);
			Interpreter_addNumericVariable (me, U"windows", 0);
			Interpreter_addNumericVariable (me, U"unix", 0);
		#endif
		Interpreter_addNumericVariable (me, U"left", 1);   // to accommodate scripts from before Praat 5.2.06
		Interpreter_addNumericVariable (me, U"right", 2);   // to accommodate scripts from before Praat 5.2.06
		Interpreter_addNumericVariable (me, U"mono", 1);   // to accommodate scripts from before Praat 5.2.06
		Interpreter_addNumericVariable (me, U"stereo", 2);   // to accommodate scripts from before Praat 5.2.06
		Interpreter_addNumericVariable (me, U"all", 0);   // to accommodate scripts from before Praat 5.2.06
		Interpreter_addNumericVariable (me, U"average", 0);   // to accommodate scripts from before Praat 5.2.06
		#define xstr(s) str(s)
		#define str(s) #s
		Interpreter_addStringVariable (me, U"praatVersion$", U"" xstr(PRAAT_VERSION_STR));
		Interpreter_addNumericVariable (me, U"praatVersion", PRAAT_VERSION_NUM);
		/*
		 * Execute commands.
		 */
		#define wordEnd(c)  (c == U'\0' || c == U' ' || c == U'\t')
		trace ("going to handle %ld lines", numberOfLines);
		//for (lineNumber = 1; lineNumber <= numberOfLines; lineNumber ++) {
			//trace ("line %d: %s", (int) lineNumber, Melder_peekStr32ToUtf8 (lines [lineNumber]));
		//}
		for (lineNumber = 1; lineNumber <= numberOfLines; lineNumber ++) {
			if (my stopped) break;
			//trace ("now at line %d: %s", (int) lineNumber, Melder_peekStr32ToUtf8 (lines [lineNumber]));
			//for (int lineNumber2 = 1; lineNumber2 <= numberOfLines; lineNumber2 ++) {
				//trace ("  line %d: %s", (int) lineNumber2, Melder_peekStr32ToUtf8 (lines [lineNumber2]));
			//}
			try {
				char32 c0;
				bool fail = false;
				MelderString32_copy (& command2, lines [lineNumber]);
				c0 = command2. string [0];
				if (c0 == U'\0') continue;
				/*
				 * Substitute variables.
				 */
				trace ("substituting variables");
				for (char32 *p = & command2. string [0]; *p != U'\0'; p ++) if (*p == U'\'') {
					/*
					 * Found a left quote. Search for a matching right quote.
					 */
					char32 *q = p + 1, varName [300], *r, *s, *colon;
					int precision = -1, percent = FALSE;
					while (*q != U'\0' && *q != U'\'' && q - p < 299) q ++;
					if (*q == U'\0') break;   // no matching right quote? done with this line!
					if (q - p == 1 || q - p >= 299) continue;   // ignore empty and too long variable names
					trace ("found %ld", (long) (q - p - 1));
					/*
					 * Found a right quote. Get potential variable name.
					 */
					for (r = p + 1, s = varName; q - r > 0; r ++, s ++) *s = *r;
					*s = U'\0';   /* Trailing null byte. */
					colon = str32chr (varName, U':');
					if (colon) {
						precision = a32tol (colon + 1);
						if (str32chr (colon + 1, U'%')) percent = TRUE;
						*colon = '\0';
					}
					InterpreterVariable var = Interpreter_hasVariable (me, varName);
					if (var) {
						/*
						 * Found a variable (p points to the left quote, q to the right quote). Substitute.
						 */
						int headlen = p - command2.string;
						const char32 *string = var -> stringValue ? var -> stringValue :
							percent ? Melder32_percent (var -> numericValue, precision) :
							precision >= 0 ?  Melder32_fixed (var -> numericValue, precision) :
							Melder32_double (var -> numericValue);
						int arglen = str32len (string);
						MelderString32_ncopy (& buffer, command2.string, headlen);
						MelderString32_append (& buffer, string, q + 1);
						MelderString32_copy (& command2, buffer.string);   // This invalidates p!! (really bad bug 20070203)
						p = command2.string + headlen + arglen - 1;
					} else {
						p = q - 1;   /* Go to before next quote. */
					}
				}
				trace ("resume");
				c0 = command2.string [0];   /* Resume in order to allow things like 'c$' = 5 */
				if ((c0 < U'a' || c0 > U'z') && c0 != U'@' && ! (c0 == U'.' && command2.string [1] >= U'a' && command2.string [1] <= U'z')) {
					praat_executeCommand (me, command2.string);
				/*
				 * Interpret control flow and variables.
				 */
				} else switch (c0) {
					case U'.':
						fail = true;
						break;
					case U'@':
					{
						/*
						 * This is a function call.
						 * Look for a function name.
						 */
						char32 *p = command2.string + 1;
						while (*p == U' ' || *p == U'\t') p ++;   // skip whitespace
						char32 *callName = p;
						while (*p != U'\0' && *p != U' ' && *p != U'\t' && *p != U'(' && *p != U':') p ++;
						if (p == callName) Melder_throw ("Missing procedure name after \"@\".");
						bool hasArguments = ( *p != U'\0' );
						if (hasArguments) {
							bool parenthesisOrColonFound = ( *p == U'(' || *p == U':' );
							*p = U'\0';   // close procedure name
							if (! parenthesisOrColonFound) {
								p ++;   // step over first white space
								while (*p != U'\0' && (*p == U' ' || *p == U'\t')) p ++;   // skip more whitespace
								hasArguments = ( *p != U'\0' );
								parenthesisOrColonFound = ( *p == U'(' || *p == U':' );
								if (hasArguments && ! parenthesisOrColonFound)
									Melder_throw ("Missing parenthesis or colon after procedure name \"", callName, "\".");
							}
							p ++;   // step over parenthesis or colon
						}
						int64 callLength = str32len (callName);
						long iline = 1;
						for (; iline <= numberOfLines; iline ++) {
							char32 *linei = lines [iline], *q;
							if (linei [0] != U'p' || linei [1] != U'r' || linei [2] != U'o' || linei [3] != U'c' ||
								linei [4] != U'e' || linei [5] != U'd' || linei [6] != U'u' || linei [7] != U'r' ||
								linei [8] != U'e' || linei [9] != U' ') continue;
							q = lines [iline] + 10;
							while (*q == U' ' || *q == U'\t') q ++;   // skip whitespace before procedure name
							char32 *procName = q;
							while (*q != U'\0' && *q != U' ' && *q != U'\t' && *q != U'(' && *q != U':') q ++;
							if (q == procName) Melder_throw ("Missing procedure name after 'procedure'.");
							if (q - procName == callLength && str32nequ (procName, callName, callLength)) {
								/*
								 * We found the procedure definition.
								 */
								if (++ my callDepth > Interpreter_MAX_CALL_DEPTH)
									Melder_throw ("Call depth greater than ", Interpreter_MAX_CALL_DEPTH, ".");
								str32cpy (my procedureNames [my callDepth], callName);
								bool parenthesisOrColonFound = ( *q == U'(' || *q == U':' );
								if (*q) q ++;   // step over parenthesis or colon or first white space
								if (! parenthesisOrColonFound) {
									while (*q == U' ' || *q == U'\t') q ++;   // skip more whitespace
									if (*q == U'(' || *q == U':') q ++;   // step over parenthesis or colon
								}
								while (*q && *q != ')') {
									static MelderString32 argument = { 0 };
									MelderString32_empty (& argument);
									while (*p == U' ' || *p == U'\t') p ++;
									while (*q == U' ' || *q == U'\t') q ++;
									char32 *parameterName = q;
									while (*q != U'\0' && *q != U' ' && *q != U'\t' && *q != U',' && *q != U')') q ++;   // collect parameter name
									int expressionDepth = 0;
									for (; *p; p ++) {
										if (*p == U',') {
											if (expressionDepth == 0) break;   // depth-0 comma ends expression
											MelderString32_appendCharacter (& argument, U',');
										} else if (*p == U')') {
											if (expressionDepth == 0) break;   // depth-0 closing parenthesis ends expression
											expressionDepth --;
											MelderString32_appendCharacter (& argument, U')');
										} else if (*p == U'(') {
											expressionDepth ++;
											MelderString32_appendCharacter (& argument, U'(');
										} else if (*p == U'\"') {
											/*
											 * Enter a string literal.
											 */
											MelderString32_appendCharacter (& argument, U'\"');
											p ++;
											for (;; p ++) {
												if (*p == U'\0') {
													Melder_throw (L"Incomplete string literal: the quotes don't match.");
												} else if (*p == U'\"') {
													MelderString32_appendCharacter (& argument, U'\"');
													if (p [1] == '\"') {
														p ++;   // stay in the string literal
														MelderString32_appendCharacter (& argument, U'\"');
													} else {
														break;
													}
												} else {
													MelderString32_appendCharacter (& argument, *p);
												}
											}
										} else {
											MelderString32_appendCharacter (& argument, *p);
										}
									}
									if (q == parameterName) break;
									if (*p) { *p = U'\0'; p ++; }
									if (q [-1] == U'$') {
										char32 *value;
										my callDepth --;
										Interpreter_stringExpression (me, argument.string, & value);
										my callDepth ++;
										char32 save = *q; *q = U'\0';
										InterpreterVariable var = Interpreter_lookUpVariable (me, parameterName); *q = save;
										Melder_free (var -> stringValue);
										var -> stringValue = value;
									} else {
										double value;
										my callDepth --;
										Interpreter_numericExpression (me, argument.string, & value);
										my callDepth ++;
										char32 save = *q; *q = U'\0';
										InterpreterVariable var = Interpreter_lookUpVariable (me, parameterName); *q = save;
										var -> numericValue = value;
									}
									if (*q) q ++;   // skip comma
								}
								if (callDepth == Interpreter_MAX_CALL_DEPTH)
									Melder_throw ("Call depth greater than ", Interpreter_MAX_CALL_DEPTH, ".");
								callStack [++ callDepth] = lineNumber;
								lineNumber = iline;
								break;
							}
						}
						if (iline > numberOfLines) Melder_throw ("Procedure \"", callName, "\" not found.");
						break;
					}
					case U'a':
						if (str32nequ (command2.string, U"assert ", 7)) {
							double value;
							Interpreter_numericExpression (me, command2.string + 7, & value);
							if (value == 0.0 || value == NUMundefined) {
								assertionFailed = TRUE;
								Melder_throw ("Script assertion fails in line ", lineNumber,
									" (", value == 0.0 ? "false" : "undefined", "):\n   ", command2.string + 7);
							}
						} else if (str32nequ (command2.string, U"asserterror ", 12)) {
							MelderString32_copy (& assertErrorString, command2.string + 12);
							assertErrorLineNumber = lineNumber;
						} else fail = true;
						break;
					case U'b':
						fail = true;
						break;
					case U'c':
						if (str32nequ (command2.string, U"call ", 5)) {
							char32 *p = command2.string + 5, *callName, *procName;
							long iline;
							bool hasArguments;
							int64 callLength;
							while (*p == U' ' || *p == U'\t') p ++;   // skip whitespace
							callName = p;
							while (*p != U'\0' && *p != U' ' && *p != U'\t' && *p != U'(' && *p != U':') p ++;
							if (p == callName) Melder_throw ("Missing procedure name after 'call'.");
							hasArguments = *p != U'\0';
							*p = U'\0';   // close procedure name
							callLength = str32len (callName);
							for (iline = 1; iline <= numberOfLines; iline ++) {
								char32 *linei = lines [iline], *q;
								int hasParameters;
								if (linei [0] != U'p' || linei [1] != U'r' || linei [2] != U'o' || linei [3] != U'c' ||
									linei [4] != U'e' || linei [5] != U'd' || linei [6] != U'u' || linei [7] != U'r' ||
									linei [8] != U'e' || linei [9] != U' ') continue;
								q = lines [iline] + 10;
								while (*q == U' ' || *q == U'\t') q ++;
								procName = q;
								while (*q != U'\0' && *q != U' ' && *q != U'\t' && *q != U'(' && *q != U':') q ++;
								if (q == procName) Melder_throw ("Missing procedure name after 'procedure'.");
								hasParameters = *q != U'\0';
								if (q - procName == callLength && str32nequ (procName, callName, callLength)) {
									if (hasArguments && ! hasParameters)
										Melder_throw ("Call to procedure \"", callName, "\" has too many arguments.");
									if (hasParameters && ! hasArguments)
										Melder_throw ("Call to procedure \"", callName, "\" has too few arguments.");
									if (++ my callDepth > Interpreter_MAX_CALL_DEPTH)
										Melder_throw ("Call depth greater than ", Interpreter_MAX_CALL_DEPTH, ".");
									str32cpy (my procedureNames [my callDepth], callName);
									if (hasParameters) {
										bool parenthesisOrColonFound = ( *q == U'(' || *q == U':' );
										q ++;   // step over parenthesis or colon or first white space
										if (! parenthesisOrColonFound) {
											while (*q == U' ' || *q == U'\t') q ++;   // skip more whitespace
											if (*q == U'(' || *q == U':') q ++;   // step over parenthesis or colon
										}
										++ p;   // first argument
										while (*q && *q != ')') {
											char32 *par, save;
											static MelderString32 arg = { 0 };
											MelderString32_empty (& arg);
											while (*p == U' ' || *p == U'\t') p ++;
											while (*q == U' ' || *q == U'\t' || *q == U',' || *q == U')') q ++;
											par = q;
											while (*q != U'\0' && *q != U' ' && *q != U'\t' && *q != U',' && *q != U')') q ++;   // collect parameter name
											if (*q) {   // does anything follow the parameter name?
												if (*p == U'\"') {
													p ++;   // skip initial quote
													while (*p != U'\0') {
														if (*p == U'\"') {   // quote signals end-of-string or string-internal quote
															if (p [1] == U'\"') {   // double quote signals string-internal quote
																MelderString32_appendCharacter (& arg, U'\"');
																p += 2;   // skip second quote
															} else {   // single quote signals end-of-string
																break;
															}
														} else {
															MelderString32_appendCharacter (& arg, *p ++);
														}
													}
												} else {
													while (*p != U'\0' && *p != U' ' && *p != U'\t')
														MelderString32_appendCharacter (& arg, *p ++);   // white space separates
												}
												if (*p) { *p = U'\0'; p ++; }
											} else {   // else rest of line
												while (*p != '\0')
													MelderString32_appendCharacter (& arg, *p ++);
											}
											if (q [-1] == '$') {
												save = *q; *q = U'\0';
												InterpreterVariable var = Interpreter_lookUpVariable (me, par); *q = save;
												Melder_free (var -> stringValue);
												var -> stringValue = Melder_str32dup_f (arg.string);
											} else {
												double value;
												my callDepth --;
												Interpreter_numericExpression (me, arg.string, & value);
												my callDepth ++;
												save = *q; *q = U'\0';
												InterpreterVariable var = Interpreter_lookUpVariable (me, par); *q = save;
												var -> numericValue = value;
											}
										}
									}
									if (callDepth == Interpreter_MAX_CALL_DEPTH)
										Melder_throw ("Call depth greater than ", Interpreter_MAX_CALL_DEPTH, ".");
									callStack [++ callDepth] = lineNumber;
									lineNumber = iline;
									break;
								}
							}
							if (iline > numberOfLines) Melder_throw ("Procedure \"", callName, "\" not found.");
						} else fail = true;
						break;
					case U'd':
						if (str32nequ (command2.string, U"dec ", 4)) {
							InterpreterVariable var = Interpreter_lookUpVariable (me, command2.string + 4);
							var -> numericValue -= 1.0;
						} else fail = true;
						break;
					case U'e':
						if (command2.string [1] == 'n' && command2.string [2] == 'd') {
							if (str32nequ (command2.string, U"endif", 5) && wordEnd (command2.string [5])) {
								/* Ignore. */
							} else if (str32nequ (command2.string, U"endfor", 6) && wordEnd (command2.string [6])) {
								int depth = 0;
								long iline;
								for (iline = lineNumber - 1; iline > 0; iline --) {
									char32 *line = lines [iline];
									if (line [0] == U'f' && line [1] == U'o' && line [2] == U'r' && line [3] == U' ') {
										if (depth == 0) { lineNumber = iline - 1; fromendfor = TRUE; break; }   // go before 'for'
										else depth --;
									} else if (str32nequ (lines [iline], U"endfor", 6) && wordEnd (lines [iline] [6])) {
										depth ++;
									}
								}
								if (iline <= 0) Melder_throw ("Unmatched 'endfor'.");
							} else if (str32nequ (command2.string, U"endwhile", 8) && wordEnd (command2.string [8])) {
								int depth = 0;
								long iline;
								for (iline = lineNumber - 1; iline > 0; iline --) {
									if (str32nequ (lines [iline], U"while ", 6)) {
										if (depth == 0) { lineNumber = iline - 1; break; }   // go before 'while'
										else depth --;
									} else if (str32nequ (lines [iline], U"endwhile", 8) && wordEnd (lines [iline] [8])) {
										depth ++;
									}
								}
								if (iline <= 0) Melder_throw ("Unmatched 'endwhile'.");
							} else if (str32nequ (command2.string, U"endproc", 7) && wordEnd (command2.string [7])) {
								if (callDepth == 0) Melder_throw ("Unmatched 'endproc'.");
								lineNumber = callStack [callDepth --];
								-- my callDepth;
							} else fail = true;
						} else if (str32nequ (command2.string, U"else", 4) && wordEnd (command2.string [4])) {
							int depth = 0;
							long iline;
							for (iline = lineNumber + 1; iline <= numberOfLines; iline ++) {
								if (str32nequ (lines [iline], U"endif", 5) && wordEnd (lines [iline] [5])) {
									if (depth == 0) { lineNumber = iline; break; }   /* Go after 'endif'. */
									else depth --;
								} else if (str32nequ (lines [iline], U"if ", 3)) {
									depth ++;
								}
							}
							if (iline > numberOfLines) Melder_throw ("Unmatched 'else'.");
						} else if (str32nequ (command2.string, U"elsif ", 6) || str32nequ (command2.string, U"elif ", 5)) {
							if (fromif) {
								double value;
								fromif = FALSE;
								Interpreter_numericExpression (me, command2.string + 5, & value);
								if (value == 0.0) {
									int depth = 0;
									long iline;
									for (iline = lineNumber + 1; iline <= numberOfLines; iline ++) {
										if (str32nequ (lines [iline], U"endif", 5) && wordEnd (lines [iline] [5])) {
											if (depth == 0) { lineNumber = iline; break; }   // go after 'endif'
											else depth --;
										} else if (str32nequ (lines [iline], U"else", 4) && wordEnd (lines [iline] [4])) {
											if (depth == 0) { lineNumber = iline; break; }   // go after 'else'
										} else if ((str32nequ (lines [iline], U"elsif", 5) && wordEnd (lines [iline] [5]))
											|| (str32nequ (lines [iline], U"elif", 4) && wordEnd (lines [iline] [4]))) {
											if (depth == 0) { lineNumber = iline - 1; fromif = TRUE; break; }   // go at next 'elsif' or 'elif'
										} else if (str32nequ (lines [iline], U"if ", 3)) {
											depth ++;
										}
									}
									if (iline > numberOfLines) Melder_throw ("Unmatched 'elsif'.");
								}
							} else {
								int depth = 0;
								long iline;
								for (iline = lineNumber + 1; iline <= numberOfLines; iline ++) {
									if (str32nequ (lines [iline], U"endif", 5) && wordEnd (lines [iline] [5])) {
										if (depth == 0) { lineNumber = iline; break; }   /* Go after 'endif'. */
										else depth --;
									} else if (str32nequ (lines [iline], U"if ", 3)) {
										depth ++;
									}
								}
								if (iline > numberOfLines) Melder_throw ("'elsif' not matched with 'endif'.");
							}
						} else if (str32nequ (command2.string, U"exit", 4)) {
							if (command2.string [4] == U'\0') {
								lineNumber = numberOfLines;   /* Go after end. */
							} else if (command2.string [4] == U' ') {
								Melder_throw (command2.string + 5);
							} else fail = true;
						} else if (str32nequ (command2.string, U"echo ", 5)) {
							/*
							 * Make sure that lines like "echo = 3" will not be regarded as assignments.
							 */
							praat_executeCommand (me, command2.string);
						} else fail = true;
						break;
					case U'f':
						if (command2.string [1] == U'o' && command2.string [2] == U'r' && command2.string [3] == U' ') {   // for_
							double toValue, loopVariable;
							char32 *frompos = str32str (command2.string, U" from "), *topos = str32str (command2.string, U" to ");
							char32 *varpos = command2.string + 4, *endvar = frompos;
							if (! topos) Melder_throw ("Missing \'to\' in \'for\' loop.");
							if (! endvar) endvar = topos;
							while (*endvar == U' ') { *endvar = '\0'; endvar --; }
							while (*varpos == U' ') varpos ++;
							if (endvar - varpos < 0) Melder_throw ("Missing loop variable after \'for\'.");
							InterpreterVariable var = Interpreter_lookUpVariable (me, varpos);
							Interpreter_numericExpression (me, topos + 4, & toValue);
							if (fromendfor) {
								fromendfor = FALSE;
								loopVariable = var -> numericValue + 1.0;
							} else if (frompos) {
								*topos = '\0';
								Interpreter_numericExpression (me, frompos + 6, & loopVariable);
							} else {
								loopVariable = 1.0;
							}
							var -> numericValue = loopVariable;
							if (loopVariable > toValue) {
								int depth = 0;
								long iline;
								for (iline = lineNumber + 1; iline <= numberOfLines; iline ++) {
									if (str32nequ (lines [iline], U"endfor", 6)) {
										if (depth == 0) { lineNumber = iline; break; }   // go after 'endfor'
										else depth --;
									} else if (str32nequ (lines [iline], U"for ", 4)) {
										depth ++;
									}
								}
								if (iline > numberOfLines) Melder_throw ("Unmatched 'for'.");
							}
						} else if (str32nequ (command2.string, U"form ", 5)) {
							long iline;
							for (iline = lineNumber + 1; iline <= numberOfLines; iline ++)
								if (str32nequ (lines [iline], U"endform", 7))
									{ lineNumber = iline; break; }   // go after 'endform'
							if (iline > numberOfLines) Melder_throw ("Unmatched 'form'.");
						} else fail = true;
						break;
					case U'g':
						if (str32nequ (command2.string, U"goto ", 5)) {
							char32 labelName [1+Interpreter_MAX_LABEL_LENGTH], *space;
							int dojump = TRUE, ilabel;
							str32ncpy (labelName, command2.string + 5, 1+Interpreter_MAX_LABEL_LENGTH);
							labelName [Interpreter_MAX_LABEL_LENGTH] = U'\0';
							space = str32chr (labelName, U' ');
							if (space == labelName) Melder_throw ("Missing label name after 'goto'.");
							if (space) {
								double value;
								*space = '\0';
								Interpreter_numericExpression (me, command2.string + 6 + str32len (labelName), & value);
								if (value == 0.0) dojump = FALSE;
							}
							if (dojump) {
								ilabel = lookupLabel (me, labelName);
								lineNumber = my labelLines [ilabel];   // loop will add 1
							}
						} else fail = true;
						break;
					case U'h':
						fail = true;
						break;
					case U'i':
						if (command2.string [1] == U'f' && command2.string [2] == U' ') {   // if_
							double value;
							Interpreter_numericExpression (me, command2.string + 3, & value);
							if (value == 0.0) {
								int depth = 0;
								long iline;
								for (iline = lineNumber + 1; iline <= numberOfLines; iline ++) {
									if (str32nequ (lines [iline], U"endif", 5)) {
										if (depth == 0) { lineNumber = iline; break; }   // go after 'endif'
										else depth --;
									} else if (str32nequ (lines [iline], U"else", 4)) {
										if (depth == 0) { lineNumber = iline; break; }   // go after 'else'
									} else if (str32nequ (lines [iline], U"elsif ", 6) || str32nequ (lines [iline], U"elif ", 5)) {
										if (depth == 0) { lineNumber = iline - 1; fromif = TRUE; break; }   // go at 'elsif'
									} else if (str32nequ (lines [iline], U"if ", 3)) {
										depth ++;
									}
								}
								if (iline > numberOfLines) Melder_throw ("Unmatched 'if'.");
							} else if (value == NUMundefined) {
								Melder_throw ("The value of the 'if' condition is undefined.");
							}
						} else if (str32nequ (command2.string, U"inc ", 4)) {
							InterpreterVariable var = Interpreter_lookUpVariable (me, command2.string + 4);
							var -> numericValue += 1.0;
						} else fail = true;
						break;
					case U'j':
						fail = true;
						break;
					case U'k':
						fail = true;
						break;
					case U'l':
						if (str32nequ (command2.string, U"label ", 6)) {
							;   /* Ignore labels. */
						} else fail = true;
						break;
					case U'm':
						fail = true;
						break;
					case U'n':
						fail = true;
						break;
					case U'o':
						fail = true;
						break;
					case U'p':
						if (str32nequ (command2.string, U"procedure ", 10)) {
							long iline = lineNumber + 1;
							for (; iline <= numberOfLines; iline ++) {
								if (str32nequ (lines [iline], U"endproc", 7) && wordEnd (lines [iline] [7])) {
									lineNumber = iline;
									break;
								}   // go after 'endproc'
							}
							if (iline > numberOfLines) Melder_throw ("Unmatched 'proc'.");
						} else if (str32nequ (command2.string, U"print", 5)) {
							/*
							 * Make sure that lines like "print = 3" will not be regarded as assignments.
							 */
							if (command2.string [5] == U' ' || (str32nequ (command2.string + 5, U"line", 4) && (command2.string [9] == U' ' || command2.string [9] == U'\0'))) {
								praat_executeCommand (me, command2.string);
							} else fail = true;
						} else fail = true;
						break;
					case U'q':
						fail = true;
						break;
					case U'r':
						if (str32nequ (command2.string, U"repeat", 6) && wordEnd (command2.string [6])) {
							/* Ignore. */
						} else fail = true;
						break;
					case U's':
						if (str32nequ (command2.string, U"stopwatch", 9) && wordEnd (command2.string [9])) {
							(void) Melder_stopwatch ();   /* Reset stopwatch. */
						} else fail = true;
						break;
					case U't':
						fail = true;
						break;
					case U'u':
						if (str32nequ (command2.string, U"until ", 6)) {
							double value;
							Interpreter_numericExpression (me, command2.string + 6, & value);
							if (value == 0.0) {
								int depth = 0;
								long iline;
								for (iline = lineNumber - 1; iline > 0; iline --) {
									if (str32nequ (lines [iline], U"repeat", 6) && wordEnd (lines [iline] [6])) {
										if (depth == 0) { lineNumber = iline; break; }   // go after 'repeat'
										else depth --;
									} else if (str32nequ (lines [iline], U"until ", 6)) {
										depth ++;
									}
								}
								if (iline <= 0) Melder_throw ("Unmatched 'until'.");
							}
						} else fail = true;
						break;
					case U'v':
						fail = true;
						break;
					case U'w':
						if (str32nequ (command2.string, U"while ", 6)) {
							double value;
							Interpreter_numericExpression (me, command2.string + 6, & value);
							if (value == 0.0) {
								int depth = 0;
								long iline;
								for (iline = lineNumber + 1; iline <= numberOfLines; iline ++) {
									if (str32nequ (lines [iline], U"endwhile", 8) && wordEnd (lines [iline] [8])) {
										if (depth == 0) { lineNumber = iline; break; }   // go after 'endwhile'
										else depth --;
									} else if (str32nequ (lines [iline], U"while ", 6)) {
										depth ++;
									}
								}
								if (iline > numberOfLines) Melder_throw ("Unmatched 'while'.");
							}
						} else fail = true;
						break;
					case U'x':
						fail = true;
						break;
					case U'y':
						fail = true;
						break;
					case U'z':
						fail = true;
						break;
					default: break;
				}
				if (fail) {
					/*
					 * Found an unknown word starting with a lower-case letter, optionally preceded by a period.
					 * See whether the word is a variable name.
					 */
					trace ("found an unknown word starting with a lower-case letter, optionally preceded by a period");
					char32 *p = & command2.string [0];
					/*
					 * Variable names consist of a sequence of letters, digits, and underscores,
					 * optionally preceded by a period and optionally followed by a $ and/or #.
					 */
					if (*p == U'.') p ++;
					while (isalnum ((int) *p) || *p == U'_' || *p == U'.')  p ++;
					if (*p == U'$') {
						/*
						 * Assign to a string variable.
						 */
						trace ("detected an assignment to a string variable");
						char32 *endOfVariable = ++ p;
						char32 *variableName = command2.string;
						int withFile;
						while (*p == U' ' || *p == U'\t') p ++;   // go to first token after variable name
						if (*p == U'[') {
							/*
							 * This must be an assignment to an indexed string variable.
							 */
							*endOfVariable = U'\0';
							static MelderString32 indexedVariableName = { 0 };
							MelderString32_copy (& indexedVariableName, command2.string);
							MelderString32_appendCharacter (& indexedVariableName, U'[');
							for (;;) {
								p ++;   // skip opening bracket or comma
								static MelderString32 index = { 0 };
								MelderString32_empty (& index);
								int depth = 0;
								while ((depth > 0 || (*p != U',' && *p != U']')) && *p != U'\n' && *p != U'\0') {
									MelderString32_appendCharacter (& index, *p);
									if (*p == U'[') depth ++;
									else if (*p == U']') depth --;
									p ++;
								}
								if (*p == U'\n' || *p == U'\0')
									Melder_throw ("Missing closing bracket (]) in indexed variable.");
								double numericIndexValue;
								Interpreter_numericExpression (me, index.string, & numericIndexValue);
								MelderString32_append (& indexedVariableName, Melder32_double (numericIndexValue));
								MelderString32_appendCharacter (& indexedVariableName, *p);
								if (*p == U']') {
									break;
								}
							}
							variableName = indexedVariableName.string;
							p ++;   // skip closing bracket
						}
						while (*p == U' ' || *p == U'\t') p ++;   // go to first token after (perhaps indexed) variable name
						if (*p == U'=') {
							withFile = 0;   // assignment
						} else if (*p == U'<') {
							withFile = 1;   // read from file
						} else if (*p == U'>') {
							if (p [1] == U'>')
								withFile = 2, p ++;   // append to file
							else
								withFile = 3;   /* Save to file. */
						} else Melder_throw ("Missing '=', '<', or '>' after variable ", variableName, ".");
						*endOfVariable = U'\0';
						p ++;
						while (*p == U' ' || *p == U'\t') p ++;   /* Go to first token after assignment or I/O symbol. */
						if (*p == U'\0') {
							if (withFile != 0)
								Melder_throw ("Missing file name after variable ", variableName, ".");
							else
								Melder_throw ("Missing expression after variable ", variableName, ".");
						}
						if (withFile) {
							structMelderFile file = { 0 };
							Melder_relativePathToFile (p, & file);
							if (withFile == 1) {
								char32 *stringValue = MelderFile_readText32 (& file);
								InterpreterVariable var = Interpreter_lookUpVariable (me, variableName);
								Melder_free (var -> stringValue);
								var -> stringValue = stringValue;   /* var becomes owner */
							} else if (withFile == 2) {
								if (theCurrentPraatObjects != & theForegroundPraatObjects) Melder_throw ("Commands that write to a file are not available inside pictures.");
								InterpreterVariable var = Interpreter_hasVariable (me, variableName);
								if (! var) Melder_throw ("Variable ", variableName, " undefined.");
								MelderFile_appendText32 (& file, var -> stringValue);
							} else {
								if (theCurrentPraatObjects != & theForegroundPraatObjects) Melder_throw ("Commands that write to a file are not available inside pictures.");
								InterpreterVariable var = Interpreter_hasVariable (me, variableName);
								if (! var) Melder_throw ("Variable ", variableName, " undefined.");
								MelderFile_writeText32 (& file, var -> stringValue, Melder_getOutputEncoding ());
							}
						} else if (isCommand (p)) {
							/*
							 * Example: name$ = Get name
							 */
							MelderString_empty (& valueString);   // empty because command may print nothing; also makes sure that valueString.string exists
							autoMelderDivertInfo divert (& valueString);
							int status = praat_executeCommand (me, p);
							InterpreterVariable var = Interpreter_lookUpVariable (me, variableName);
							Melder_free (var -> stringValue);
							var -> stringValue = Melder_wcsToStr32 (status ? valueString.string : L"");
						} else {
							/*
							 * Evaluate a string expression and assign the result to the variable.
							 * Examples:
							 *    sentence$ = subject$ + verb$ + object$
							 *    extension$ = if index (file$, ".") <> 0
							 *       ... then right$ (file$, length (file$) - rindex (file$, "."))
							 *       ... else "" fi
							 */
							char32 *stringValue;
							trace ("evaluating string expression");
							Interpreter_stringExpression (me, p, & stringValue);
							trace ("assigning to string variable %ls", variableName);
							InterpreterVariable var = Interpreter_lookUpVariable (me, variableName);
							Melder_free (var -> stringValue);
							var -> stringValue = stringValue;   // var becomes owner
						}
					} else if (*p == U'#') {
						/*
						 * Assign to a numeric array variable.
						 */
						char32 *endOfVariable = ++ p;
						while (*p == U' ' || *p == U'\t') p ++;   // go to first token after variable name
						if (*p == U'=') {
							;
						} else Melder_throw ("Missing '=' after variable ", command2.string, ".");
						*endOfVariable = U'\0';
						p ++;
						while (*p == U' ' || *p == U'\t') p ++;   // go to first token after assignment or I/O symbol
						if (*p == U'\0') {
							Melder_throw ("Missing expression after variable ", command2.string, ".");
						}
						struct Formula_NumericArray value;
						Interpreter_numericArrayExpression (me, p, & value);
						InterpreterVariable var = Interpreter_lookUpVariable (me, command2.string);
						NUMmatrix_free (var -> numericArrayValue. data, 1, 1);
						var -> numericArrayValue = value;
					} else {
						/*
						 * Try to assign to a numeric variable.
						 */
						double value;
						char32 *variableName = command2.string;
						int typeOfAssignment = 0;   /* Plain assignment. */
						if (*p == U'\0') {
							/*
							 * Command ends here: it may be a PraatShell command.
							 */
							praat_executeCommand (me, command2.string);
							continue;   // next line
						}
						char32 *endOfVariable = p;
						while (*p == U' ' || *p == U'\t') p ++;
						if (*p == U'=' || ((*p == U'+' || *p == U'-' || *p == U'*' || *p == U'/') && p [1] == U'=')) {
							/*
							 * This must be an assignment (though: "echo = ..." ???)
							 */
							typeOfAssignment = *p == U'+' ? 1 : *p == U'-' ? 2 : *p == U'*' ? 3 : *p == U'/' ? 4 : 0;
							*endOfVariable = U'\0';   // Close variable name. FIXME: this can be any weird character, e.g. hallo&
						} else if (*p == U'[') {
							/*
							 * This must be an assignment to an indexed numeric variable.
							 */
							*endOfVariable = U'\0';
							static MelderString32 indexedVariableName = { 0 };
							MelderString32_copy (& indexedVariableName, command2.string);
							MelderString32_appendCharacter (& indexedVariableName, U'[');
							for (;;) {
								p ++;   // skip opening bracket or comma
								static MelderString32 index = { 0 };
								MelderString32_empty (& index);
								int depth = 0;
								while ((depth > 0 || (*p != U',' && *p != U']')) && *p != U'\n' && *p != U'\0') {
									MelderString32_appendCharacter (& index, *p);
									if (*p == U'[') depth ++;
									else if (*p == U']') depth --;
									p ++;
								}
								if (*p == '\n' || *p == '\0')
									Melder_throw ("Missing closing bracket (]) in indexed variable.");
								Interpreter_numericExpression (me, index.string, & value);
								MelderString32_append (& indexedVariableName, Melder32_double (value));
								MelderString32_appendCharacter (& indexedVariableName, *p);
								if (*p == ']') {
									break;
								}
							}
							variableName = indexedVariableName.string;
							p ++;   // skip closing bracket
							while (*p == U' ' || *p == U'\t') p ++;
							if (*p == U'=' || ((*p == U'+' || *p == U'-' || *p == U'*' || *p == U'/') && p [1] == U'=')) {
								typeOfAssignment = *p == U'+' ? 1 : *p == U'-' ? 2 : *p == U'*' ? 3 : *p == U'/' ? 4 : 0;
							}
						} else {
							/*
							 * Not an assignment: perhaps a PraatShell command (select, echo, execute, pause ...).
							 */
							praat_executeCommand (me, variableName);
							continue;   // next line
						}
						p += typeOfAssignment == 0 ? 1 : 2;
						while (*p == U' ' || *p == U'\t') p ++;
						if (*p == U'\0') Melder_throw ("Missing expression after variable ", variableName, ".");
						/*
						 * Three classes of assignments:
						 *    var = formula
						 *    var = Query
						 *    var = Object creation
						 */
						if (isCommand (p)) {
							/*
							 * Get the value of the query.
							 */
							MelderString_empty (& valueString);
							autoMelderDivertInfo divert (& valueString);
							MelderString_appendCharacter (& valueString, 1);
							int status = praat_executeCommand (me, p);
							if (status == 0) {
								value = NUMundefined;
							} else if (valueString.string [0] == 1) {
								int IOBJECT, result = 0, found = 0;
								WHERE (SELECTED) { result = IOBJECT; found += 1; }
								if (found > 1) {
									Melder_throw ("Multiple objects selected. Cannot assign ID to variable.");
								} else if (found == 0) {
									Melder_throw ("No objects selected. Cannot assign ID to variable.");
								} else {
									value = theCurrentPraatObjects -> list [result]. id;
								}
							} else {
								value = Melder_atof (valueString.string);   // including --undefined--
							}
						} else {
							/*
							 * Get the value of the formula.
							 */
							Interpreter_numericExpression (me, p, & value);
						}
						/*
						 * Assign the value to a variable.
						 */
						if (typeOfAssignment == 0) {
							/*
							 * Use an existing variable, or create a new one.
							 */
							//Melder_casual ("looking up variable %ls", variableName);
							InterpreterVariable var = Interpreter_lookUpVariable (me, variableName);
							var -> numericValue = value;
						} else {
							/*
							 * Modify an existing variable.
							 */
							InterpreterVariable var = Interpreter_hasVariable (me, variableName);
							if (var == NULL) Melder_throw ("Unknown variable ", variableName, ".");
							if (var -> numericValue == NUMundefined) {
								/* Keep it that way. */
							} else {
								if (typeOfAssignment == 1) {
									var -> numericValue += value;
								} else if (typeOfAssignment == 2) {
									var -> numericValue -= value;
								} else if (typeOfAssignment == 3) {
									var -> numericValue *= value;
								} else if (value == 0) {
									var -> numericValue = NUMundefined;
								} else {
									var -> numericValue /= value;
								}
							}
						}
					}
				} // endif fail
				if (assertErrorLineNumber != 0 && assertErrorLineNumber != lineNumber) {
					long save_assertErrorLineNumber = assertErrorLineNumber;
					assertErrorLineNumber = 0;
					Melder_throw ("Script assertion fails in line ", save_assertErrorLineNumber,
							L": error « ", assertErrorString.string, L" » not raised. Instead: no error.");
					
				}
			} catch (MelderError) {
				//Melder_casual ("Error: << %ls >>\nassertErrorLineNumber: %ld\nlineNumber: %ld\nAssert error string: << %ls >>\n",
				//	Melder_getError(), assertErrorLineNumber, lineNumber, assertErrorString.string);
				if (assertErrorLineNumber == 0) {
					throw;
				} else if (assertErrorLineNumber != lineNumber) {
					if (wcsstr (Melder_getError (), Melder_peekStr32ToWcs (assertErrorString.string))) {
						Melder_clearError ();
						assertErrorLineNumber = 0;
					} else {
						wchar_t *errorCopy_nothrow = Melder_wcsdup_f (Melder_getError ());   // UGLY but necessary (1)
						Melder_clearError ();
						autostring errorCopy = errorCopy_nothrow;   // UGLY but necessary (2)
						Melder_throw ("Script assertion fails in line ", assertErrorLineNumber,
							L": error « ", assertErrorString.string, L" » not raised. Instead:\n",
							errorCopy.peek());
					}
				}
			}
		} // endfor lineNumber
		my numberOfLabels = 0;
		my running = false;
		my stopped = false;
	} catch (MelderError) {
		if (lineNumber > 0) {
			bool normalExplicitExit = str32nequ (lines [lineNumber], U"exit ", 5) || Melder_hasError (L"Script exited.");
			if (! normalExplicitExit && ! assertionFailed) {   // don't show the message twice!
				while (lines [lineNumber] [0] == U'\0') {   // did this use to be a continuation line?
					lineNumber --;
					Melder_assert (lineNumber > 0);   // originally empty lines that stayed empty should not generate errors
				}
				Melder_error_ ("Script line ", lineNumber, L" not performed or completed:\n« ", lines [lineNumber], L" »");
			}
		}
		my numberOfLabels = 0;
		my running = false;
		my stopped = false;
		if (wcsequ (Melder_getError (), L"\nScript exited.\n")) {
			Melder_clearError ();
		} else {
			throw;
		}
	}
}

void Interpreter_stop (Interpreter me) {
//Melder_casual ("Interpreter_stop in: %ld", me);
	my stopped = true;
//Melder_casual ("Interpreter_stop out: %ld", me);
}

void Interpreter_voidExpression (Interpreter me, const char32 *expression) {
	Formula_compile (me, NULL, expression, kFormula_EXPRESSION_TYPE_NUMERIC, FALSE);
	struct Formula_Result result;
	Formula_run (0, 0, & result);
}

void Interpreter_numericExpression (Interpreter me, const char32 *expression, double *value) {
	Melder_assert (value != NULL);
	if (str32str (expression, U"(=")) {
		*value = Melder_a32tof (expression);
	} else {
		Formula_compile (me, NULL, expression, kFormula_EXPRESSION_TYPE_NUMERIC, FALSE);
		struct Formula_Result result;
		Formula_run (0, 0, & result);
		*value = result. result.numericResult;
	}
}

void Interpreter_stringExpression (Interpreter me, const char32 *expression, char32 **value) {
	Formula_compile (me, NULL, expression, kFormula_EXPRESSION_TYPE_STRING, FALSE);
	struct Formula_Result result;
	Formula_run (0, 0, & result);
	*value = result. result.stringResult;
}

void Interpreter_numericArrayExpression (Interpreter me, const char32 *expression, struct Formula_NumericArray *value) {
	Formula_compile (me, NULL, expression, kFormula_EXPRESSION_TYPE_NUMERIC_ARRAY, FALSE);
	struct Formula_Result result;
	Formula_run (0, 0, & result);
	*value = result. result.numericArrayResult;
}

void Interpreter_anyExpression (Interpreter me, const char32 *expression, struct Formula_Result *result) {
	Formula_compile (me, NULL, expression, kFormula_EXPRESSION_TYPE_UNKNOWN, FALSE);
	Formula_run (0, 0, result);
}

/* End of file Interpreter.cpp */
