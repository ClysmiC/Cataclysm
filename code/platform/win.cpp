#include <Windows.h>
#include "win.h"

using std::string;
using std::vector;

void _getAllFileNames(const string& directoryPath, vector<string>& result, bool recursive, const string& extension, int rootFolderNameLength)
{
    string searchPath = directoryPath + "*.*";
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &fd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            string fileName = fd.cFileName;
            if (fileName == "." || fileName == "..") continue;

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (recursive)
                {
                    _getAllFileNames(directoryPath + fileName + '/', result, recursive, extension, rootFolderNameLength);
                }
            }
            else
            {
                if (extension == "" || fileName.compare(fileName.length() - extension.length(), extension.length(), extension) == 0)
                {
                    result.push_back((directoryPath + fileName).substr(rootFolderNameLength));
                }
            }
        } while (FindNextFile(hFind, &fd));
        
        FindClose(hFind);
    }
}

// https://stackoverflow.com/a/20847429/9635414
vector<string> getAllFileNames(const string& directoryPath, bool recursive, const string& extension)
{
    // NOTE: this returns their names MINUS the folder prefix that gets passed in. For example, if the folder parameter is
    // C:/foo/bar and there is a file named C:/foo/bar/baz.txt, the result entry will just say baz.txt

    // If we want different behavior in the future, we can just modify this funciton with some flags to control that kind of stuff

    string fixedDirectoryPath = directoryPath;
    char lastChar = fixedDirectoryPath[fixedDirectoryPath.length() - 1];
    if (lastChar != '\\' && lastChar != '/') fixedDirectoryPath += '/';

    vector<string> result;
    _getAllFileNames(fixedDirectoryPath, result, recursive, extension, fixedDirectoryPath.length());
    return result;
}
