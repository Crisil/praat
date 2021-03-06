#ifndef _SpectrumEditor_h_
#define _SpectrumEditor_h_
/* SpectrumEditor.h
 *
 * Copyright (C) 1992-2011,2012,2013,2015 Paul Boersma
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

#include "FunctionEditor.h"
#include "Spectrum.h"

Thing_define (SpectrumEditor, FunctionEditor) {
	double minimum, maximum, cursorHeight;
	GuiMenuItem publishBandButton, publishSoundButton;

	void v_createMenus ()
		override;
	void v_createHelpMenuItems (EditorMenu menu)
		override;
	void v_dataChanged ()
		override;
	void v_draw ()
		override;
	int v_click (double xWC, double yWC, bool shiftKeyPressed)
		override;
	void v_play (double tmin, double tmax)
		override;
	void v_createMenuItems_view (EditorMenu menu)
		override;
	const wchar_t * v_format_domain ()
		override { return L"Frequency domain:"; }
	const wchar_t * v_format_short ()
		override { return L"%.0f"; }
	const wchar_t * v_format_long ()
		override { return L"%.2f"; }
	int v_fixedPrecision_long ()
		override { return 2; }
	const wchar_t * v_format_units ()
		override { return L"hertz"; }
	const wchar_t * v_format_totalDuration ()
		override { return L"Total bandwidth %.2f hertz"; }
	const wchar_t * v_format_window ()
		override { return L"Visible part %.2f hertz"; }
	const wchar_t * v_format_selection ()
		override { return L"%.2f Hz"; }

	#include "SpectrumEditor_prefs.h"
};

SpectrumEditor SpectrumEditor_create (const wchar_t *title, Spectrum data);

/* End of file SpectrumEditor.h */
#endif
