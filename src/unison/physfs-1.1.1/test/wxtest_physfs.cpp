/**
 * Test program for PhysicsFS, using wxWidgets. May only work on Unix.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#if ( (defined(__MACH__)) && (defined(__APPLE__)) )
#define PLATFORM_MACOSX 1
#include <Carbon/Carbon.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <wx/wx.h>
#include <wx/treectrl.h>

#include "physfs.h"

#define TEST_VER_MAJOR  1
#define TEST_VER_MINOR  1
#define TEST_VER_PATCH  1

//static PHYSFS_uint32 do_buffer_size = 0;

enum WxTestPhysfsMenuCommands
{
    // start with standard menu items, since using the wxIDs will map them
    //  to sane things in the platform's UI (gnome icons in GTK+, moves the
    //  about and quit items to the Apple menu on Mac OS X, etc).
    MENUCMD_About = wxID_ABOUT,
    MENUCMD_Quit = wxID_EXIT,

    // non-standard menu items go here.
    MENUCMD_Init = wxID_HIGHEST,
    MENUCMD_Deinit,
    MENUCMD_AddArchive,
    MENUCMD_Mount,
    MENUCMD_Remove,
    MENUCMD_GetCDs,
    MENUCMD_SetWriteDir,
    MENUCMD_PermitSymLinks,
    MENUCMD_SetSaneConfig,
    MENUCMD_MkDir,
    MENUCMD_Delete,
    MENUCMD_Cat,
    MENUCMD_SetBuffer,
    MENUCMD_StressBuffer,
    MENUCMD_Append,
    MENUCMD_Write,
    MENUCMD_GetLastError,

/*
    { "getdirsep",      cmd_getdirsep,      0, NULL                         },
    { "getsearchpath",  cmd_getsearchpath,  0, NULL                         },
    { "getbasedir",     cmd_getbasedir,     0, NULL                         },
    { "getuserdir",     cmd_getuserdir,     0, NULL                         },
    { "getwritedir",    cmd_getwritedir,    0, NULL                         },
    { "getrealdir",     cmd_getrealdir,     1, "<fileToFind>"               },
    { "exists",         cmd_exists,         1, "<fileToCheck>"              },
    { "isdir",          cmd_isdir,          1, "<fileToCheck>"              },
    { "issymlink",      cmd_issymlink,      1, "<fileToCheck>"              },
    { "filelength",     cmd_filelength,     1, "<fileToCheck>"              },
    { "getlastmodtime", cmd_getlastmodtime, 1, "<fileToExamine>"            },
*/
};


class WxTestPhysfsFrame : public wxFrame
{
public:
    WxTestPhysfsFrame(const wxChar *argv0);

    void rebuildTree();

    void onMenuInit(wxCommandEvent &evt);
    void onMenuDeinit(wxCommandEvent &evt);
    void onMenuAddArchive(wxCommandEvent &evt);
    void onMenuGetCDs(wxCommandEvent &evt);
    void onMenuPermitSymLinks(wxCommandEvent &evt);

private:
    wxTreeCtrl *fileTree;
    wxTreeItemId stateItem;
    wxTreeItemId fsItem;

    int err(int success);
    void fillFileSystemTree(const char *path, const wxTreeItemId &item);
    void doInit(const char *argv0);
    void doDeinit();

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WxTestPhysfsFrame, wxFrame)
    EVT_MENU(MENUCMD_Init, WxTestPhysfsFrame::onMenuInit)
    EVT_MENU(MENUCMD_Deinit, WxTestPhysfsFrame::onMenuDeinit)
    EVT_MENU(MENUCMD_AddArchive, WxTestPhysfsFrame::onMenuAddArchive)
    EVT_MENU(MENUCMD_GetCDs, WxTestPhysfsFrame::onMenuGetCDs)
    EVT_MENU(MENUCMD_PermitSymLinks, WxTestPhysfsFrame::onMenuPermitSymLinks)
END_EVENT_TABLE()



// This is the the Application itself.
class WxTestPhysfsApp : public wxApp
{
public:
    WxTestPhysfsApp() : mainWindow(NULL) { /* no-op. */ }
    virtual bool OnInit();

private:
    WxTestPhysfsFrame *mainWindow;
};

DECLARE_APP(WxTestPhysfsApp)


static inline char *newstr(const char *str)
{
    char *retval = NULL;
    if (str != NULL)
    {
        retval = new char[strlen(str) + 1];
        strcpy(retval, str);
    } // if
    return retval;
} // newstr

static char *newutf8(const wxString &wxstr)
{
    #if wxUSE_UNICODE
    size_t len = wxstr.Len() + 1;
    char *utf8text = new char[len * 6];
    wxConvUTF8.WC2MB(utf8text, wxstr, len);
    return utf8text;
    #else
    return newstr(wxstr);
    #endif
} // newutf8


WxTestPhysfsFrame::WxTestPhysfsFrame(const wxChar *argv0)
    : wxFrame(NULL, -1, wxT("WxTestPhysfs"))
{
    this->CreateStatusBar();

    wxMenuBar *menuBar = new wxMenuBar;

    wxMenu *stuffMenu = new wxMenu;
    stuffMenu->Append(MENUCMD_Init, wxT("&Init"));
    stuffMenu->Append(MENUCMD_Deinit, wxT("&Deinit"));
    stuffMenu->Append(MENUCMD_AddArchive, wxT("&Add Archive"));
    stuffMenu->Append(MENUCMD_Mount, wxT("&Mount Archive"));
    stuffMenu->Append(MENUCMD_Remove, wxT("&Remove Archive"));
    stuffMenu->Append(MENUCMD_GetCDs, wxT("&Get CD-ROM drives"));
    stuffMenu->Append(MENUCMD_SetWriteDir, wxT("&Set Write Dir"));
    stuffMenu->Append(MENUCMD_SetSaneConfig, wxT("Set Sane &Config"));
    stuffMenu->Append(MENUCMD_MkDir, wxT("M&kDir"));
    stuffMenu->Append(MENUCMD_Delete, wxT("D&elete"));
    stuffMenu->Append(MENUCMD_Cat, wxT("&Cat"));
    stuffMenu->Append(MENUCMD_SetBuffer, wxT("Set &Buffer"));
    stuffMenu->Append(MENUCMD_StressBuffer, wxT("Stress &Test Buffer"));
    stuffMenu->Append(MENUCMD_Append, wxT("&Append"));
    stuffMenu->Append(MENUCMD_Write, wxT("&Write"));
    stuffMenu->Append(MENUCMD_Write, wxT("&Update getLastError"));
    stuffMenu->AppendCheckItem(MENUCMD_PermitSymLinks, wxT("&Permit symlinks"));
    menuBar->Append(stuffMenu, wxT("&Stuff"));

    //wxMenu *helpMenu = new wxMenu;
    //helpMenu->Append(MENUCMD_About, wxT("&About\tF1"));
    //menuBar->Append(helpMenu, wxT("&Help"));

    this->SetMenuBar(menuBar);

    this->fileTree = new wxTreeCtrl(this, -1);

    // The sizer just makes sure that fileTree owns whole client area.
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(this->fileTree, 1, wxALL | wxEXPAND | wxALIGN_CENTRE);
    sizer->SetItemMinSize(this->fileTree, 1, 1);
    this->SetSizer(sizer);

    char *utf8argv0 = newutf8(wxString(argv0));
    this->doInit(utf8argv0);
    delete[] utf8argv0;
} // WxTestPhysfsFrame::WxTestPhysfsFrame


int WxTestPhysfsFrame::err(int success)
{
    if (success)
        this->SetStatusText(wxT(""));
    else
        this->SetStatusText(wxString(PHYSFS_getLastError(), wxConvUTF8));
    return success;
} // WxTestPhysfsFrame::err


void WxTestPhysfsFrame::fillFileSystemTree(const char *path,
                                           const wxTreeItemId &item)
{
    char **rc = PHYSFS_enumerateFiles(path);
    char **i;
    wxTreeItemId id;

    if (rc == NULL)
    {
        const wxString quote(wxT("'"));
        wxString str(wxT("Enumeration error: "));
        str << quote << wxString(PHYSFS_getLastError(), wxConvUTF8) << quote;
        id = this->fileTree->AppendItem(item, str);
        this->fileTree->SetItemTextColour(id, wxColour(255, 0, 0));
    } // if
    else
    {
        for (i = rc; *i != NULL; i++)
        {
            id = this->fileTree->AppendItem(item, wxString(*i, wxConvUTF8));
            const int len = strlen(path) + strlen(*i) + 2;
            char *fname = new char[len];
            const char *origdir = path;
            if (strcmp(origdir, "/") == 0)
                origdir = "";
            snprintf(fname, len, "%s/%s", origdir, *i);

            if (PHYSFS_isDirectory(fname))
            {
                this->fileTree->SetItemTextColour(id, wxColour(0, 0, 255));
                this->fillFileSystemTree(fname, id);
            } // if

            else if (PHYSFS_isSymbolicLink(fname))
            {
                this->fileTree->SetItemTextColour(id, wxColour(0, 255, 0));
            } // else if

            else  // ...file.
            {
            } // else

            delete[] fname;
        } // for

        PHYSFS_freeList(rc);
    } // else
} // fillFileSystemTree


void WxTestPhysfsFrame::rebuildTree()
{
    const wxString dot(wxT("."));
    const wxString quote(wxT("'"));
    wxTreeItemId item;
    wxString str;
    const char *cstr = NULL;
    const bool wasInit = PHYSFS_isInit() ? true : false;

    this->fileTree->DeleteAllItems();
    wxTreeItemId root = this->fileTree->AddRoot(wxT("PhysicsFS"));
    this->stateItem = this->fileTree->AppendItem(root, wxT("Library state"));

    str = wxT("Initialized: ");
    str << ((wasInit) ? wxT("true") : wxT("false"));
    this->fileTree->AppendItem(this->stateItem, str);

    this->fileTree->Expand(this->stateItem);
    this->fileTree->Expand(root);

    // Fill in version information...

    PHYSFS_Version ver;
    item = this->stateItem;
    str = wxT("wxtest_physfs version: ");
    str << TEST_VER_MAJOR << dot << TEST_VER_MINOR << dot << TEST_VER_PATCH;
    this->fileTree->AppendItem(item, str);
    PHYSFS_VERSION(&ver);
    str = wxT("Compiled against PhysicsFS version: ");
    str << (int) ver.major << dot << (int) ver.minor << dot << ver.patch;
    this->fileTree->AppendItem(item, str);
    PHYSFS_getLinkedVersion(&ver);
    str = wxT("Linked against PhysicsFS version: ");
    str << (int) ver.major << dot << (int) ver.minor << dot << ver.patch;
    this->fileTree->AppendItem(item, str);

    if (!wasInit)
        return;   // nothing else to do before initialization...

    str = wxT("Symbolic links permitted: ");
    str << ((PHYSFS_symbolicLinksPermitted()) ? wxT("true") : wxT("false"));
    this->fileTree->AppendItem(this->stateItem, str);

    str = wxT("Native directory separator: ");
    str << quote << wxString(PHYSFS_getDirSeparator(), wxConvUTF8) << quote;
    this->fileTree->AppendItem(this->stateItem, str);

    // Fill in supported archives...

    item = this->fileTree->AppendItem(this->stateItem, wxT("Archivers"));
    const PHYSFS_ArchiveInfo **arcs = PHYSFS_supportedArchiveTypes();
    if (*arcs == NULL)
        this->fileTree->AppendItem(item, wxT("(none)"));
    else
    {
        const PHYSFS_ArchiveInfo **i;
        for (i = arcs; *i != NULL; i++)
        {
            const wxString ext((*i)->extension, wxConvUTF8);
            const wxString desc((*i)->description, wxConvUTF8);
            const wxString auth((*i)->author, wxConvUTF8);
            const wxString url((*i)->url, wxConvUTF8);
            wxTreeItemId arcitem = this->fileTree->AppendItem(item, ext);
            this->fileTree->AppendItem(arcitem, desc);
            this->fileTree->AppendItem(arcitem, auth);
            this->fileTree->AppendItem(arcitem, url);
        } // for
    } // else


    // Fill in the standard paths...

    item = this->fileTree->AppendItem(this->stateItem, wxT("Paths"));
    str = wxT("Base directory: ");
    str << quote << wxString(PHYSFS_getBaseDir(), wxConvUTF8) << quote;
    this->fileTree->AppendItem(item, str);
    str = wxT("User directory: ");
    str << quote << wxString(PHYSFS_getUserDir(), wxConvUTF8) << quote;
    this->fileTree->AppendItem(item, str);
    str = wxT("Write directory: ");
    if ((cstr = PHYSFS_getWriteDir()) == NULL)
        str << wxT("(NULL)");
    else
        str << quote << wxString(cstr ? cstr : "(NULL)", wxConvUTF8) << quote;
    this->fileTree->AppendItem(item, str);
    //str = wxT("Preference directory: ");
    //str << wxString(PHYSFS_getUserDir(), wxConvUTF8);
    //this->fileTree->AppendItem(item, str);

    // Fill in the CD-ROMs...

    item = this->fileTree->AppendItem(this->stateItem, wxT("CD-ROMs"));
    char **cds = PHYSFS_getCdRomDirs();
    if (cds == NULL)
    {
        str = wxT("Error: ");
        str << quote << wxString(PHYSFS_getLastError(), wxConvUTF8) << quote;
        wxTreeItemId id = this->fileTree->AppendItem(item, str);
        this->fileTree->SetItemTextColour(id, wxColour(255, 0, 0));
    } // if
    else
    {
        if (*cds == NULL)
            this->fileTree->AppendItem(item, wxT("(none)"));
        else
        {
            char **i;
            for (i = cds; *i != NULL; i++)
                this->fileTree->AppendItem(item, wxString(*i, wxConvUTF8));
        } // else
        PHYSFS_freeList(cds);
    } // else

    // Fill in search path...

    item = this->fileTree->AppendItem(this->stateItem, wxT("Search path"));
    char **sp = PHYSFS_getSearchPath();
    if (sp == NULL)
    {
        str = wxT("Error: ");
        str << quote << wxString(PHYSFS_getLastError(), wxConvUTF8) << quote;
        wxTreeItemId id = this->fileTree->AppendItem(item, str);
        this->fileTree->SetItemTextColour(id, wxColour(255, 0, 0));
    } // if
    else
    {
        if (*sp == NULL)
            this->fileTree->AppendItem(item, wxT("(none)"));
        else
        {
            char **i;
            for (i = sp; *i != NULL; i++)
                this->fileTree->AppendItem(item, wxString(*i, wxConvUTF8));
        } // else
        PHYSFS_freeList(sp);
    } // else

    // Now fill in the filesystem...

    this->fsItem = this->fileTree->AppendItem(root, wxT("Filesystem"));
    this->fillFileSystemTree("/", this->fsItem);
    this->fileTree->Expand(this->fsItem);
} // WxTestPhysfsFrame::rebuildTree


void WxTestPhysfsFrame::doInit(const char *argv0)
{
    if (!this->err(PHYSFS_init(argv0)))
        ::wxMessageBox(wxT("PHYSFS_init() failed!"), wxT("wxTestPhysfs"));
    this->rebuildTree();
} // WxTestPhysfsFrame::doInit


void WxTestPhysfsFrame::doDeinit()
{
    if (!this->err(PHYSFS_deinit()))
        ::wxMessageBox(wxT("PHYSFS_deinit() failed!"), wxT("wxTestPhysfs"));
    this->rebuildTree();
} // WxTestPhysfsFrame::doDeinit


void WxTestPhysfsFrame::onMenuInit(wxCommandEvent &evt)
{
    wxString argv0(wxGetApp().argv[0] == NULL ? wxT("") : wxGetApp().argv[0]);
    wxString str(wxGetTextFromUser(wxT("PHYSFS_init"),
                 wxT("argv[0]? (cancel for NULL)"), argv0));
    char *cstr = str.IsEmpty() ? NULL : newutf8(str);
    this->doInit(cstr);
    delete[] cstr;
} // WxTestPhysfsFrame::onMenuInit


void WxTestPhysfsFrame::onMenuDeinit(wxCommandEvent &evt)
{
    this->doDeinit();
} // WxTestPhysfsFrame::onMenuDeinit


void WxTestPhysfsFrame::onMenuAddArchive(wxCommandEvent &evt)
{
    wxString arc = wxFileSelector(wxT("Choose archive to add"));
    if (!arc.IsEmpty())
    {
        char *cstr = newutf8(arc);
        // !!! FIXME: add to start/end?
        if (!this->err(PHYSFS_addToSearchPath(cstr, 1)))
            ::wxMessageBox(wxT("PHYSFS_addToSearchPath() failed!"), wxT("wxTestPhysfs"));
        delete[] cstr;
        this->rebuildTree();
    } // if
} // WxTestPhysfsFrame::onMenuAddArchive


void WxTestPhysfsFrame::onMenuGetCDs(wxCommandEvent &evt)
{
    this->rebuildTree();  // This will call PHYSFS_getCdRomDirs()...
} // WxTestPhysfsFrame::onMenuGetCDs


void WxTestPhysfsFrame::onMenuPermitSymLinks(wxCommandEvent &evt)
{
    PHYSFS_permitSymbolicLinks(evt.IsChecked() ? 1 : 0);
    this->rebuildTree();
} // WxTestPhysfsFrame::onMenuPermitSymLinks



IMPLEMENT_APP(WxTestPhysfsApp)

bool WxTestPhysfsApp::OnInit()
{
    #if PLATFORM_MACOSX
    // This lets a stdio app become a GUI app. Otherwise, you won't get
    //  GUI events from the system and other things will fail to work.
    // Putting the app in an application bundle does the same thing.
    //  TransformProcessType() is a 10.3+ API. SetFrontProcess() is 10.0+.
    if (TransformProcessType != NULL)  // check it as a weak symbol.
    {
        ProcessSerialNumber psn = { 0, kCurrentProcess };
        TransformProcessType(&psn, kProcessTransformToForegroundApplication);
        SetFrontProcess(&psn);
    } // if
    #endif

    this->mainWindow = new WxTestPhysfsFrame(this->argv[0]);
    this->mainWindow->Show(true);
    SetTopWindow(this->mainWindow);
    return true;
} // WxTestPhysfsApp::OnInit

// end of wxtest_physfs.cpp ...

