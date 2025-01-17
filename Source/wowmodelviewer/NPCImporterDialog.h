/*
 * NPCimporterDialog.h
 *
 *  Created on: 5 nov. 2013
 *
 */

#pragma once

#include <wx/dialog.h>
class wxButton;
class wxStaticText;
class wxTextCtrl;

#include <QString>

class NPCimporterDialog : public wxDialog
{
public:
	NPCimporterDialog(wxWindow* parent = nullptr, wxWindowID id = -1, const wxString& title = _("Import from URL"),
	                  const wxPoint& position = wxDefaultPosition, const wxSize& size = wxSize(300, 300));

	int getImportedId();
	QString getNPCLine();

private:
	wxTextCtrl* m_URLname;
	wxButton* m_importBtn;
	wxButton* m_displayBtn;
	wxStaticText* m_nameResult;
	wxStaticText* m_typeResult;
	wxStaticText* m_idResult;
	wxStaticText* m_displayIdResult;

	static const int ID_BTN_IMPORT;

	void OnImportButtonClicked(wxCommandEvent& event);
	void OnDisplayButtonClicked(wxCommandEvent& event);

	DECLARE_EVENT_TABLE();
};
