/* TableOfReal_def.h
 *
 * Copyright (C) 1992-2011,2015 Paul Boersma
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


#define ooSTRUCT TableOfReal
oo_DEFINE_CLASS (TableOfReal, Data)

	oo_LONG (numberOfRows)
	oo_LONG (numberOfColumns)
	oo_STRING_VECTOR (rowLabels, numberOfRows)
	oo_STRING_VECTOR (columnLabels, numberOfColumns)
	oo_DOUBLE_MATRIX (data, numberOfRows, numberOfColumns)

	#if oo_DECLARING
		void v_info ()
			override;
		bool v_hasGetNrow ()
			override { return true; }
		double v_getNrow ()
			override { return numberOfRows; }
		bool v_hasGetNcol ()
			override { return true; }
		double v_getNcol ()
			override { return numberOfColumns; }
		bool v_hasGetRowStr ()
			override { return true; }
		const char32 * v_getRowStr (long irow)
			override;
		bool v_hasGetColStr ()
			override { return true; }
		const char32 * v_getColStr (long icol)
			override;
		bool v_hasGetMatrix ()
			override { return true; }
		double v_getMatrix (long irow, long icol)
			override;
		bool v_hasGetRowIndex ()
			override { return true; }
		double v_getRowIndex (const char32 *rowLabel)
			override;
		bool v_hasGetColIndex ()
			override { return true; }
		double v_getColIndex (const char32 *columnLabel)
			override;
	#endif

oo_END_CLASS (TableOfReal)
#undef ooSTRUCT


/* End of file TableOfReal_def.h */
