/*
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "Core.h"
#include <windows.h>
#include <shlobj.h>

namespace modulith {

    /**
     * This class contains utility methods for showing file / folder selection dialog windows
     */
    class FileDialog {
    public:
        /**
         * Opens a popup where the user can select a folder from their local machine
         * @param infoText The text shown as the header of the popup
         * @return A path to a folder if one was selected or {@link std::nullopt} if the operation was cancelled
         * @remark Code taken from https://www.codeproject.com/Articles/2604/Browse-Folder-dialog-search-folder-and-all-sub-fol
         */
        static std::optional <std::string> BrowseFolder(const std::string& infoText) {
            TCHAR path[MAX_PATH];
            BROWSEINFO bi = {nullptr};
            bi.lpszTitle = (infoText.c_str());
            LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

            if (pidl != nullptr) {
                // get the name of the folder and put it in path
                SHGetPathFromIDList(pidl, path);

                // free memory used
                IMalloc* imalloc = nullptr;
                if (SUCCEEDED(SHGetMalloc(&imalloc))) {
                    imalloc->Free(pidl);
                    imalloc->Release();
                }

                return path;
            } else {
                return {};
            }
        }
    };
}
