#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

/** @file
 * @brief Hello World code
 *
 * wxWidgets hello world code
 */

class MyApp : public wxApp
{
public:
	/**
	 * @brief Create new frame and display that
	 *
	 * This routine creates new frame and displays the same.
	 *
	 * @return true on successful initialization of "hello world" application
	 * @see wxTopLevelWindowMSW::Show()
	 */
	virtual bool OnInit();
};

class MyFrame : public wxFrame
{
public:
	/**
	 * @brief Frame setup
	 *
	 * Sets and populates menubar with File and Help menus. File contains Hello and \n
	 * Quit entries while Help contains About entry. A status bar at the bottom is \n
	 * also created. A relevant bind to the OnHello, OnExit, and OnAbout routines \n
	 * is done for desired callback.
	 *
	 * @see MyFrame::OnHello(wxCommandEvent&)
	 * @see MyFrame::OnExit()
	 * @see MyFrame::OnAbout(wxCommandEvent& event)
	 */
	MyFrame();

private:
	/**
	 * @brief Hello entry callback
	 *
	 * Opens up a modal message window with desired hello message.
	 *
	 * @see MyFrame::MyFrame()
	 */
	void OnHello(wxCommandEvent& event);

	/**
	 * @brief Exit entry callback
	 *
	 * Closes the application.
	 *
	 * @see MyFrame::MyFrame()
	 */
	void OnExit(wxCommandEvent& event);

	/**
	 * @brief About entry callback
	 *
	 * Opens up a modal message window with desired application information.
	 *
	 * @see MyFrame::MyFrame()
	 */
	void OnAbout(wxCommandEvent& event);
};

enum
{
	ID_Hello = 1
};

wxIMPLEMENT_APP_CONSOLE(MyApp);

bool MyApp::OnInit()
{
	MyFrame* frame = new MyFrame();
	frame->Show(true);
	return true;
}

MyFrame::MyFrame()
	: wxFrame(nullptr, wxID_ANY, "Hello World")
{
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
		"Help string shown in status bar for this menu item");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuHelp, "&Help");

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Welcome to wxWidgets!");

	Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
	Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
}

void MyFrame::OnExit(wxCommandEvent& event)
{
	Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
	wxMessageBox("This is a wxWidgets Hello World example",
		"About Hello World", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnHello(wxCommandEvent& event)
{
	wxLogMessage("Hello world from wxWidgets!");
}