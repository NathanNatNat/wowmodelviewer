/*----------------------------------------------------------------------*\
| This file is part of WoW Model Viewer                                  |
|                                                                        |
| WoW Model Viewer is free software: you can redistribute it and/or      |
| modify it under the terms of the GNU General Public License as         |
| published by the Free Software Foundation, either version 3 of the     |
| License, or (at your option) any later version.                        |
|                                                                        |
| WoW Model Viewer is distributed in the hope that it will be useful,    |
| but WITHOUT ANY WARRANTY; without even the implied warranty of         |
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          |
| GNU General Public License for more details.                           |
|                                                                        |
| You should have received a copy of the GNU General Public License      |
| along with WoW Model Viewer.                                           |
| If not, see <http://www.gnu.org/licenses/>.                            |
\*----------------------------------------------------------------------*/

/*
 * AnimationExportChoiceDialog.h
 *
 *  Created on: 3 jul. 2015
 *   Copyright: 2015, WoW Model Viewer (http://wowmodelviewer.net)
 */

#pragma once

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/choicdlg.h>

class wxButton;

class AnimationExportChoiceDialog : public wxMultiChoiceDialog
{
public :
	AnimationExportChoiceDialog(wxWindow* parent, const wxString& message, const wxString& caption,
	                            const wxArrayString& choices);

	~AnimationExportChoiceDialog() override
	{
	}

private :
	void updateButtons(wxCommandEvent& event);
	void OnSelectAll(wxCommandEvent& event);
	void OnUnselectAll(wxCommandEvent& event);

	wxButton* m_selectall;
	wxButton* m_unselectall;

	DECLARE_EVENT_TABLE();
};
