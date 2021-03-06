/* PointProcess_def.h
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


#define ooSTRUCT PointProcess
oo_DEFINE_CLASS (PointProcess, Function)

	#if oo_DECLARING || oo_COPYING
		oo_LONG (maxnt)
	#endif

	oo_LONG (nt)

	#if oo_COPYING
		oo_DOUBLE_VECTOR (t, maxnt)
	#elif oo_READING
		if (our nt) {
			our maxnt = our nt;
			oo_DOUBLE_VECTOR (t, nt)
		} else {
			our maxnt = 1;
			our t = NUMvector <double> (1, 1);
		}
	#else
		oo_DOUBLE_VECTOR (t, nt)
	#endif

	#if oo_DECLARING
		void v_info ()
			override;
		int v_domainQuantity ()
			override { return MelderQuantity_TIME_SECONDS; }
		void v_shiftX (double xfrom, double xto)
			override;
		void v_scaleX (double xminfrom, double xmaxfrom, double xminto, double xmaxto)
			override;
	#endif

oo_END_CLASS (PointProcess)
#undef ooSTRUCT


/* End of file PointProcess_def.h */
