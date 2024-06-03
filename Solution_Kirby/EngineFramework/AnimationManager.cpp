#include "pch.h"
#include "AnimationRender.h"
bool AnimationManager::IsBitmapFile(const string& filename)
{
    size_t dotIndex = filename.find_last_of('.');
    if (dotIndex != string::npos)
    {
        string extension = filename.substr(dotIndex + 1);
        transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        return extension == "bmp";
    }
    return false;
}

Animation AnimationManager::LoadAnimation(const string& folderName, float time)
{
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer);
    string currentDirectory = buffer;

    vector<HBITMAP> bitmapHandles;
    string searchPath = currentDirectory + "\\" + folderName + "\\*.*";
    WIN32_FIND_DATAA fileData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fileData);

    if (hFind != INVALID_HANDLE_VALUE) 
    {
        do {
            string fileName = fileData.cFileName;
            if (IsBitmapFile(fileName))
            {
                string filePath = folderName + "\\" + fileName;
                HBITMAP hBitmap = (HBITMAP)LoadImageA(NULL, filePath.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
                if (hBitmap != NULL)
                {
                    bitmapHandles.push_back(hBitmap);
                }
                else 
                {
                    cout << "Failed to load bitmap: " << filePath << endl;
                }
            }
        } while (FindNextFileA(hFind, &fileData) != 0);
        FindClose(hFind);
    }
    else
    {
        cout << "Failed to find files in folder: " << folderName << endl;
    }

    Animation newAnim;
    newAnim.bitmaps = bitmapHandles;
    newAnim.time = time;

    static int identity = 1000;
    newAnim.identity = identity;
    identity++;
    return newAnim;
}

void AnimationManager::ReleaseAnimation(Animation& anim)
{
    for (HBITMAP hBitmap : anim.bitmaps)
    {
        DeleteObject(hBitmap);
    }
}

HBITMAP AnimationManager::LoadHBitmap(const string& path)
{
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer);
    string currentDirectory = buffer;
    string searchPath = currentDirectory + "\\" + path + ".bmp";
    HBITMAP hBitmap = (HBITMAP)LoadImageA(NULL, searchPath.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (hBitmap != NULL)
    {
        return hBitmap;
    }
    else
    {
        return NULL;
    }
}

void AnimationManager::ReleaseHBitmap(HBITMAP hbit)
{
    DeleteObject(hbit);
}
