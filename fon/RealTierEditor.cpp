/* RealTierEditor.cpp
 *
 * Copyright (C) 1992-2011,2012,2013,2014,2015 Paul Boersma
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

#include "RealTierEditor.h"
#include "EditorM.h"

Thing_implement (RealTierEditor, TimeSoundEditor, 0);

#define SOUND_HEIGHT  0.382

/********** MENU COMMANDS **********/

static void menu_cb_removePoints (EDITOR_ARGS) {
	EDITOR_IAM (RealTierEditor);
	Editor_save (me, L"Remove point(s)");
	if (my d_startSelection == my d_endSelection)
		AnyTier_removePointNear (my data, my d_startSelection);
	else
		AnyTier_removePointsBetween (my data, my d_startSelection, my d_endSelection);
	RealTierEditor_updateScaling (me);
	FunctionEditor_redraw (me);
	Editor_broadcastDataChanged (me);
}

static void menu_cb_addPointAtCursor (EDITOR_ARGS) {
	EDITOR_IAM (RealTierEditor);
	if (NUMdefined (my v_minimumLegalValue ()) && my ycursor < my v_minimumLegalValue ())
		Melder_throw ("Cannot add a point below ", my v_minimumLegalValue (), my v_rightTickUnits (), ".");
	if (NUMdefined (my v_maximumLegalValue ()) && my ycursor > my v_maximumLegalValue ())
		Melder_throw ("Cannot add a point above ", my v_maximumLegalValue (), my v_rightTickUnits (), ".");
	Editor_save (me, L"Add point");
	RealTier_addPoint ((RealTier) my data, 0.5 * (my d_startSelection + my d_endSelection), my ycursor);
	RealTierEditor_updateScaling (me);
	FunctionEditor_redraw (me);
	Editor_broadcastDataChanged (me);
}

static void menu_cb_addPointAt (EDITOR_ARGS) {
	EDITOR_IAM (RealTierEditor);
	EDITOR_FORM (L"Add point", 0)
		REAL (L"Time (s)", L"0.0")
		REAL (my v_quantityText (), L"0.0")
	EDITOR_OK
		SET_REAL (L"Time", 0.5 * (my d_startSelection + my d_endSelection))
		SET_REAL (my v_quantityKey (), my ycursor)
	EDITOR_DO
		double desiredValue = GET_REAL (my v_quantityKey ());
		if (NUMdefined (my v_minimumLegalValue ()) && desiredValue < my v_minimumLegalValue ())
			Melder_throw ("Cannot add a point below ", my v_minimumLegalValue (), my v_rightTickUnits (), ".");
		if (NUMdefined (my v_maximumLegalValue ()) && desiredValue > my v_maximumLegalValue ())
			Melder_throw ("Cannot add a point above ", my v_maximumLegalValue (), my v_rightTickUnits (), ".");
		Editor_save (me, L"Add point");
		RealTier_addPoint ((RealTier) my data, GET_REAL (L"Time"), desiredValue);
		RealTierEditor_updateScaling (me);
		FunctionEditor_redraw (me);
		Editor_broadcastDataChanged (me);
	EDITOR_END
}

static void menu_cb_setRange (EDITOR_ARGS) {
	EDITOR_IAM (RealTierEditor);
	EDITOR_FORM (my v_setRangeTitle (), 0)
		REAL (my v_yminText (), my v_defaultYminText ())
		REAL (my v_ymaxText (), my v_defaultYmaxText ())
	EDITOR_OK
		SET_REAL (my v_yminKey (), my ymin)
		SET_REAL (my v_ymaxKey (), my ymax)
	EDITOR_DO
		my ymin = GET_REAL (my v_yminKey ());
		my ymax = GET_REAL (my v_ymaxKey ());
		if (my ymax <= my ymin) RealTierEditor_updateScaling (me);
		FunctionEditor_redraw (me);
	EDITOR_END
}

void structRealTierEditor :: v_createMenuItems_view (EditorMenu menu) {
	RealTierEditor_Parent :: v_createMenuItems_view (menu);
	EditorMenu_addCommand (menu, L"-- view/realtier --", 0, 0);
	EditorMenu_addCommand (menu, v_setRangeTitle (), 0, menu_cb_setRange);
}

void structRealTierEditor :: v_createMenus () {
	RealTierEditor_Parent :: v_createMenus ();
	EditorMenu menu = Editor_addMenu (this, L"Point", 0);
	EditorMenu_addCommand (menu, L"Add point at cursor", 'T', menu_cb_addPointAtCursor);
	EditorMenu_addCommand (menu, L"Add point at...", 0, menu_cb_addPointAt);
	EditorMenu_addCommand (menu, L"-- remove point --", 0, NULL);
	EditorMenu_addCommand (menu, L"Remove point(s)", GuiMenu_OPTION + 'T', menu_cb_removePoints);
}

void RealTierEditor_updateScaling (RealTierEditor me) {
	RealTier data = (RealTier) my data;
	if (data -> points -> size == 0) {
		my ymin = my v_defaultYmin ();
		my ymax = my v_defaultYmax ();
	} else {
		double ymin = RealTier_getMinimumValue (data);
		double ymax = RealTier_getMaximumValue (data);
		double range = ymax - ymin;
		if (range == 0.0) ymin -= 1.0, ymax += 1.0;
		else ymin -= 0.2 * range, ymax += 0.2 * range;
		if (NUMdefined (my v_minimumLegalValue()) && ymin < my v_minimumLegalValue ())
			ymin = my v_minimumLegalValue ();
		if (NUMdefined (my v_maximumLegalValue ()) && ymin > my v_maximumLegalValue ())
			ymin = my v_maximumLegalValue ();
		if (NUMdefined (my v_minimumLegalValue ()) && ymax < my v_minimumLegalValue ())
			ymax = my v_minimumLegalValue ();
		if (NUMdefined (my v_maximumLegalValue ()) && ymax > my v_maximumLegalValue ())
			ymax = my v_maximumLegalValue ();
		if (ymin >= ymax) {
			if (NUMdefined (my v_minimumLegalValue ()) && NUMdefined (my v_maximumLegalValue ())) {
				ymin = my v_minimumLegalValue ();
				ymax = my v_maximumLegalValue ();
			} else if (NUMdefined (my v_minimumLegalValue ())) {
				ymin = my v_minimumLegalValue ();
				ymax = ymin + 1.0;
			} else {
				Melder_assert (NUMdefined (my v_maximumLegalValue ()));
				ymax = my v_maximumLegalValue ();
				ymin = ymax - 1.0;
			}
		}
		if (ymin < my ymin || my ymin < 0.0) my ymin = ymin;
		if (ymax > my ymax) my ymax = ymax;
		if (my ycursor <= my ymin || my ycursor >= my ymax)
			my ycursor = 0.382 * my ymin + 0.618 * my ymax;
	}
}

void structRealTierEditor :: v_dataChanged () {
	RealTierEditor_updateScaling (this);
	RealTierEditor_Parent :: v_dataChanged ();
}

/********** DRAWING AREA **********/

void structRealTierEditor :: v_draw () {
	RealTier data = (RealTier) our data;
	long ifirstSelected, ilastSelected, n = data -> points -> size, imin, imax, i;
	Graphics_Viewport viewport;
	if (our d_sound.data) {
		viewport = Graphics_insetViewport (our d_graphics, 0, 1, 1 - SOUND_HEIGHT, 1.0);
		Graphics_setColour (our d_graphics, Graphics_WHITE);
		Graphics_setWindow (our d_graphics, 0, 1, 0, 1);
		Graphics_fillRectangle (our d_graphics, 0, 1, 0, 1);
		TimeSoundEditor_drawSound (this, -1.0, 1.0);
		Graphics_resetViewport (our d_graphics, viewport);
		Graphics_insetViewport (our d_graphics, 0, 1, 0.0, 1 - SOUND_HEIGHT);
	}
	Graphics_setColour (our d_graphics, Graphics_WHITE);
	Graphics_setWindow (our d_graphics, 0, 1, 0, 1);
	Graphics_fillRectangle (our d_graphics, 0, 1, 0, 1);
	Graphics_setWindow (our d_graphics, our d_startWindow, our d_endWindow, our ymin, our ymax);
	Graphics_setColour (our d_graphics, Graphics_RED);
	Graphics_line (our d_graphics, our d_startWindow, ycursor, our d_endWindow, our ycursor);
	Graphics_setTextAlignment (our d_graphics, Graphics_RIGHT, Graphics_HALF);
	Graphics_text1 (our d_graphics, our d_startWindow, our ycursor, Melder_float (Melder_half (our ycursor)));
	Graphics_setColour (our d_graphics, Graphics_BLUE);
	Graphics_setTextAlignment (our d_graphics, Graphics_LEFT, Graphics_TOP);
	Graphics_text2 (our d_graphics, our d_endWindow, our ymax, Melder_float (Melder_half (ymax)), our v_rightTickUnits ());
	Graphics_setTextAlignment (our d_graphics, Graphics_LEFT, Graphics_HALF);
	Graphics_text2 (our d_graphics, our d_endWindow, our ymin, Melder_float (Melder_half (our ymin)), our v_rightTickUnits ());
	ifirstSelected = AnyTier_timeToHighIndex (data, our d_startSelection);
	ilastSelected = AnyTier_timeToLowIndex (data, our d_endSelection);
	imin = AnyTier_timeToHighIndex (data, our d_startWindow);
	imax = AnyTier_timeToLowIndex (data, our d_endWindow);
	Graphics_setLineWidth (our d_graphics, 2);
	if (n == 0) {
		Graphics_setTextAlignment (our d_graphics, Graphics_CENTRE, Graphics_HALF);
		Graphics_text (our d_graphics, 0.5 * (our d_startWindow + our d_endWindow),
			0.5 * (our ymin + our ymax), L"(no points)");
	} else if (imax < imin) {
		double yleft = RealTier_getValueAtTime (data, our d_startWindow);
		double yright = RealTier_getValueAtTime (data, our d_endWindow);
		Graphics_line (our d_graphics, our d_startWindow, yleft, our d_endWindow, yright);
	} else for (i = imin; i <= imax; i ++) {
		RealPoint point = data -> point (i);
		double t = point -> number, y = point -> value;
		if (i >= ifirstSelected && i <= ilastSelected)
			Graphics_setColour (our d_graphics, Graphics_RED);
		Graphics_fillCircle_mm (our d_graphics, t, y, 3);
		Graphics_setColour (our d_graphics, Graphics_BLUE);
		if (i == 1)
			Graphics_line (our d_graphics, our d_startWindow, y, t, y);
		else if (i == imin)
			Graphics_line (our d_graphics, t, y, our d_startWindow, RealTier_getValueAtTime (data, our d_startWindow));
		if (i == n)
			Graphics_line (our d_graphics, t, y, our d_endWindow, y);
		else if (i == imax)
			Graphics_line (our d_graphics, t, y, our d_endWindow, RealTier_getValueAtTime (data, our d_endWindow));
		else {
			RealPoint pointRight = data -> point (i + 1);
			Graphics_line (our d_graphics, t, y, pointRight -> number, pointRight -> value);
		}
	}
	Graphics_setLineWidth (our d_graphics, 1);
	Graphics_setColour (our d_graphics, Graphics_BLACK);
	our v_updateMenuItems_file ();
}

static void drawWhileDragging (RealTierEditor me, double xWC, double yWC, long first, long last, double dt, double dy) {
	RealTier data = (RealTier) my data;
	(void) xWC;
	(void) yWC;

	/*
	 * Draw all selected points as magenta empty circles, if inside the window.
	 */
	for (long i = first; i <= last; i ++) {
		RealPoint point = data -> point (i);
		double t = point -> number + dt, y = point -> value + dy;
		if (t >= my d_startWindow && t <= my d_endWindow)
			Graphics_circle_mm (my d_graphics, t, y, 3);
	}

	if (last == first) {
		/*
		 * Draw a crosshair with time and y.
		 */
		RealPoint point = data -> point (first);
		double t = point -> number + dt, y = point -> value + dy;
		Graphics_line (my d_graphics, t, my ymin, t, my ymax - Graphics_dyMMtoWC (my d_graphics, 4.0));
		Graphics_setTextAlignment (my d_graphics, kGraphics_horizontalAlignment_CENTRE, Graphics_TOP);
		Graphics_text1 (my d_graphics, t, my ymax, Melder_fixed (t, 6));
		Graphics_line (my d_graphics, my d_startWindow, y, my d_endWindow, y);
		Graphics_setTextAlignment (my d_graphics, Graphics_LEFT, Graphics_BOTTOM);
		Graphics_text1 (my d_graphics, my d_startWindow, y, Melder_fixed (y, 6));
	}
}

int structRealTierEditor :: v_click (double xWC, double yWC, bool shiftKeyPressed) {
	RealTier pitch = (RealTier) our data;
	long inearestPoint, ifirstSelected, ilastSelected;
	RealPoint nearestPoint;
	double dt = 0, df = 0;
	bool draggingSelection;
	Graphics_Viewport viewport;

	/*
	 * Perform the default action: move cursor.
	 */
	//our d_startSelection = our d_endSelection = xWC;
	if (our d_sound.data) {
		if (yWC < 1 - SOUND_HEIGHT) {   /* Clicked in tier area? */
			yWC /= 1 - SOUND_HEIGHT;
			our ycursor = (1.0 - yWC) * our ymin + yWC * our ymax;
			viewport = Graphics_insetViewport (our d_graphics, 0, 1, 0, 1 - SOUND_HEIGHT);
		} else {
			return our RealTierEditor_Parent :: v_click (xWC, yWC, shiftKeyPressed);
		}
	} else {
		our ycursor = (1.0 - yWC) * our ymin + yWC * our ymax;
	}
	Graphics_setWindow (our d_graphics, our d_startWindow, our d_endWindow, our ymin, our ymax);
	yWC = our ycursor;

	/*
	 * Clicked on a point?
	 */
	inearestPoint = AnyTier_timeToNearestIndex (pitch, xWC);
	if (inearestPoint == 0) return RealTierEditor_Parent :: v_click (xWC, yWC, shiftKeyPressed);
	nearestPoint = (RealPoint) pitch -> points -> item [inearestPoint];
	if (Graphics_distanceWCtoMM (d_graphics, xWC, yWC, nearestPoint -> number, nearestPoint -> value) > 1.5) {
		if (d_sound.data) Graphics_resetViewport (our d_graphics, viewport);
		return our RealTierEditor_Parent :: v_click (xWC, yWC, shiftKeyPressed);
	}

	/*
	 * Clicked on a selected point?
	 */
	draggingSelection = shiftKeyPressed &&
		nearestPoint -> number > d_startSelection && nearestPoint -> number < d_endSelection;
	if (draggingSelection) {
		ifirstSelected = AnyTier_timeToHighIndex (pitch, d_startSelection);
		ilastSelected = AnyTier_timeToLowIndex (pitch, d_endSelection);
		Editor_save (this, L"Drag points");
	} else {
		ifirstSelected = ilastSelected = inearestPoint;
		Editor_save (this, L"Drag point");
	}

	/*
	 * Drag.
	 */
	Graphics_xorOn (our d_graphics, Graphics_MAROON);
	drawWhileDragging (this, xWC, yWC, ifirstSelected, ilastSelected, dt, df);   // draw at old position
	while (Graphics_mouseStillDown (our d_graphics)) {
		double xWC_new, yWC_new;
		Graphics_getMouseLocation (our d_graphics, & xWC_new, & yWC_new);
		if (xWC_new != xWC || yWC_new != yWC) {
			drawWhileDragging (this, xWC, yWC, ifirstSelected, ilastSelected, dt, df);   // undraw at old position
			dt += xWC_new - xWC, df += yWC_new - yWC;
			xWC = xWC_new, yWC = yWC_new;
			drawWhileDragging (this, xWC, yWC, ifirstSelected, ilastSelected, dt, df);   // draw at new position
		}
	}
	Graphics_xorOff (d_graphics);

	/*
	 * Dragged inside window?
	 */
	if (xWC < d_startWindow || xWC > d_endWindow) return 1;

	/*
	 * Points not dragged past neighbours?
	 */
	{
		RealPoint *points = pitch -> peekPoints ();
		double newTime = points [ifirstSelected] -> number + dt;
		if (newTime < our tmin) return 1;   // outside domain
		if (ifirstSelected > 1 && newTime <= points [ifirstSelected - 1] -> number)
			return 1;   // past left neighbour
		newTime = points [ilastSelected] -> number + dt;
		if (newTime > our tmax) return 1;   // outside domain
		if (ilastSelected < pitch -> numberOfPoints() && newTime >= points [ilastSelected + 1] -> number)
			return 1;   // past right neighbour
	}

	/*
	 * Drop.
	 */
	for (int i = ifirstSelected; i <= ilastSelected; i ++) {
		RealPoint point = pitch -> point (i);
		point -> number += dt;
		point -> value += df;
		if (NUMdefined (v_minimumLegalValue ()) && point -> value < v_minimumLegalValue ())
			point -> value = v_minimumLegalValue ();
		if (NUMdefined (v_maximumLegalValue ()) && point -> value > v_maximumLegalValue ())
			point -> value = v_maximumLegalValue ();
	}

	/*
	 * Make sure that the same points are still selected (a problem with Undo...).
	 */

	if (draggingSelection) our d_startSelection += dt, our d_endSelection += dt;
	if (ifirstSelected == ilastSelected) {
		/*
		 * Move crosshair to only selected pitch point.
		 */
		RealPoint point = (RealPoint) pitch -> points -> item [ifirstSelected];
		our d_startSelection = our d_endSelection = point -> number;
		our ycursor = point -> value;
	} else {
		/*
		 * Move crosshair to mouse location.
		 */
		/*our cursor += dt;*/
		our ycursor += df;
		if (NUMdefined (v_minimumLegalValue ()) && our ycursor < v_minimumLegalValue ())
			our ycursor = v_minimumLegalValue ();
		if (NUMdefined (v_maximumLegalValue ()) && our ycursor > v_maximumLegalValue ())
			our ycursor = v_maximumLegalValue ();
	}

	Editor_broadcastDataChanged (this);
	RealTierEditor_updateScaling (this);
	return 1;   // update needed
}

void structRealTierEditor :: v_play (double a_tmin, double a_tmax) {
	if (our d_sound.data)
		Sound_playPart (our d_sound.data, a_tmin, a_tmax, theFunctionEditor_playCallback, this);
}

void RealTierEditor_init (RealTierEditor me, const wchar_t *title, RealTier data, Sound sound, bool ownSound) {
	Melder_assert (data != NULL);
	Melder_assert (Thing_member (data, classRealTier));
	TimeSoundEditor_init (me, title, data, sound, ownSound);
	my ymin = -1.0;
	RealTierEditor_updateScaling (me);
	my ycursor = 0.382 * my ymin + 0.618 * my ymax;
}

/* End of file RealTierEditor.cpp */
