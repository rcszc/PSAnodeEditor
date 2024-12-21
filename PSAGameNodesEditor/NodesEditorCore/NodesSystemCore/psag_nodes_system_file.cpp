// psag_nodes_system_file.
#include <filesystem>
#include "psag_nodes_system.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSAnodesFiletool {

    bool ForEachDirectoryFile(
        const string& directory_path, const string& file_filter,
        const function<void(const string&, const string&)>& callback
    ) {
        if (!filesystem::exists(directory_path) || !filesystem::is_directory(directory_path)) {
            PushLogger(LogError, LABLogNodesFILETOOL, "directory path invalid.");
            return PSAG_SYSTEM_FAILED;
        }
        for (const auto& Entry : filesystem::directory_iterator(directory_path)) {
            if (filesystem::is_regular_file(Entry.status())) {
                // filesystem path params. 
                const string PathFull     = Entry.path().string();
                const string PathFileName = Entry.path().stem().string();

                // check file extension filter.
                if (file_filter.empty() || Entry.path().extension() == file_filter)
                    callback(PathFull, PathFileName);
            }
        }
        return PSAG_SYSTEM_SUCCESS;
    }

    string FilepathAssemble(
        const string& directory, const string& name, const string& extension
    ) {
        // check directory name empty ?
        if (directory.empty() || name.empty())
            return string();

        filesystem::path DirectoryPath(directory);
        filesystem::path FileName(name);
        filesystem::path ExtensionFile(extension);

        // normalize extension.
        if (!extension.empty() && extension.front() != '.')
            ExtensionFile = "." + ExtensionFile.string();
        // path: folder + name + exten.
        return (DirectoryPath / (FileName += ExtensionFile)).string();
    }

    bool DeleteFile(const string& filepath) {
        if (!filesystem::exists(filepath)) return PSAG_SYSTEM_FAILED;
        // delete file.
        filesystem::remove(filepath);
        return PSAG_SYSTEM_SUCCESS;
    }
}