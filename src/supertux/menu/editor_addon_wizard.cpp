//  SuperTux
//  Copyright (C) 2018 christ2go <christian@hagemeier.ch>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "supertux/menu/editor_addon_wizard.hpp"

#include "gui/menu.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "editor/editor.hpp"
#include "editor/input_center.hpp"
#include "editor/scroller.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/options_menu.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/world.hpp"
#include "util/gettext.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include <physfs.h>
#include "physfs/physfs_file_system.hpp"
#include "util/file_system.hpp"
#include "util/writer.hpp"
#include "gui/dialog.hpp"
#ifdef HAVE_LIBZIP
  #include <zip.h>
#endif
static PHYSFS_EnumerateCallbackResult printDir(void *data, const char *origdir, const char *fname)
{
        PHYSFS_Stat sr;
        std::vector<std::string>* liste = (std::vector<std::string>*)data;
        std::string realf = FileSystem::join(origdir,fname);
        PHYSFS_stat(realf.c_str(),&sr);
        if (sr.filetype == PHYSFS_FILETYPE_DIRECTORY)
        {
                PHYSFS_enumerate(realf.c_str(),printDir, data);
        } else {
                liste->push_back(realf);
        }
        return PHYSFS_ENUM_OK; // give me more data, please.
}

EditorAddonWizard::EditorAddonWizard(const std::string& dir) :
        name(),
        author(),
        license(),
        m_dir(dir),
        id()
{
  #ifdef HAVE_LIBZIP
        add_label("Export Add-Om");
        add_hl();
        add_textfield(_("Id"), &id);
        add_textfield(_("Name"), &name);
        add_textfield(_("Author"), &author);
        add_textfield(_("License"), &license);
        add_intfield(_("Version"), &vers);
        add_entry(MNID_PACK,_("Pack Addon"));
  #else
        add_inactive("This version of supertux has not been compiled with Libzip support.");
        add_inactive("You can however package your add-on manually.");
  #endif
        add_back(_("Back"));
}

void EditorAddonWizard::menu_action(MenuItem* item)
{
  #if HAVE_LIBZIP
        if(item->id == MNID_PACK)
        {
                // Check that no field is empty
                if(id.empty() || author.empty() || title.empty() || license.empty())
                {
                  Dialog::show_message(_("Please fill out all fields."));
                  return;
                }
                // First create the NFO-File
                std::stringstream nfostream;
                Writer w(&nfostream);
                w.start_list("supertux-addoninfo");
                w.write("id",id);
                w.write("version",vers);
                w.write("author",author);
                w.write("title",name);
                w.write("license",license);
                w.end_list("supertux-addoninfo");
                log_debug << "Starting to create the zip file " << m_dir << std::endl;
                // Collect all filenames
                PhysFSFileSystem pfs;
                std::vector<std::string> f;
                PHYSFS_enumerate(m_dir.c_str(),printDir, &f);
                int error = 0;
                zip_error_t er;
                std::string filename = id+".zip";
                zip * arch = zip_open(filename.c_str(),ZIP_CREATE,&error);
                if (error != 0)
                {
                        zip_error_t e;
                        zip_error_init_with_code(&e,error);
                        log_debug << "Error creating zip file" << std::endl << zip_error_strerror(&e) << std::endl;
                }

                // Create nfo file
                std::string nfo = nfostream.str();
                std::string adf = id+".nfo";
                zip_source_t * zipl  = zip_source_buffer_create(nfo.c_str(),nfo.size(),0,&er);
                zip_file_add(arch,adf.c_str(),zipl,ZIP_FL_ENC_UTF_8);

                for (auto file:f)
                {
                        PHYSFS_Stat stat;
                        auto fhandle = PHYSFS_openRead(file.c_str());
                        PHYSFS_stat(file.c_str(),&stat);
                        // Create a source buffer with corresponding length
                        void* buffer = malloc(stat.filesize);
                        if(!buffer)
                        {
                                // malloc failed, exit with error.
                                Dialog::show_message("There has been an error creating the zip file. (Malloc failed)");
                                return;
                        }
                        PHYSFS_readBytes(fhandle,buffer,stat.filesize);
                        PHYSFS_close(fhandle);
                        zip_source_t * zips = zip_source_buffer_create(buffer,stat.filesize,1,&er);
                        zip_file_add(arch,file.c_str(),zips,ZIP_FL_ENC_UTF_8);
                }

                int e = zip_close(arch);
                if (e != 0)
                {
                        log_debug << "Error saving" << zip_error_strerror(zip_get_error(arch)) << std::endl;
                        Dialog::show_message("There has been an error creating the zip file.");
                }else{
                        Dialog::show_message("Zip created and saved in your user directory.");
                }
        }
  #endif
}
