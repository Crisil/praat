#ifndef _ManipulationEditor_h_
#define _ManipulationEditor_h_
/* ManipulationEditor.h
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
#include "Manipulation.h"

#include "ManipulationEditor_enums.h"

Thing_define (ManipulationEditor, FunctionEditor) {
	PointProcess previousPulses;
	PitchTier previousPitch;
	DurationTier previousDuration;
	double soundmin, soundmax;
	int synthesisMethod;
	GuiMenuItem synthPulsesButton, synthPulsesHumButton;
	GuiMenuItem synthPulsesLpcButton;
	GuiMenuItem synthPitchButton, synthPitchHumButton;
	GuiMenuItem synthPulsesPitchButton, synthPulsesPitchHumButton;
	GuiMenuItem synthOverlapAddNodurButton, synthOverlapAddButton;
	GuiMenuItem synthPitchLpcButton;
	struct { double minPeriodic, cursor; } pitchTier;
	struct { double cursor;  } duration;
	Graphics_Viewport inset;

	void v_destroy ()
		override;
	void v_createMenus ()
		override;
	void v_createHelpMenuItems (EditorMenu menu)
		override;
	void v_saveData ()
		override;
	void v_restoreData ()
		override;
	void v_draw ()
		override;
	int v_click (double xWC, double yWC, bool shiftKeyPressed)
		override;
	void v_play (double tmin, double tmax)
		override;

	#include "ManipulationEditor_prefs.h"
};

ManipulationEditor ManipulationEditor_create (const wchar_t *title, Manipulation ana);

/* End of file ManipulationEditor.h */
#endif
