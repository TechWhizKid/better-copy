#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <thread>
#include <chrono>
#include <cstdio>
#include <direct.h>   // For _getcwd
#include <cstring>    // For std::memcmp
#include <sys/stat.h> // For checking if destination is a directory
#include <filesystem> // For filesystem operations (C++17)

namespace fs = std::filesystem;

void showHelp()
{
    std::cout << "\nUsage: bcopy.exe [/?] [/v] [/y] [/n] [source] [destination]\n"
              << "Options:\n"
              << "  /v          Verify the copy after completion.\n"
              << "  /y          Overwrite the destination file without asking.\n"
              << "  /n          Skip copying if the destination file exists.\n"
              << "  /?          Show this help menu.\n"
              << "If destination is not provided, the file will be copied to the current directory and "
              << "if destination is a directory it should end in double slash like \"E:\\example\\New folder\\\\\""
              << "\n";
}

void showProgress(size_t bytesRead, size_t totalSize, double speed)
{
    double mbRead = bytesRead / (1024.0 * 1024.0);
    double mbTotal = totalSize / (1024.0 * 1024.0);
    std::cout << ""
              << std::fixed << std::setprecision(2)
              << "Copied: " << mbRead << " MB of " << mbTotal << " MB | "
              << "Speed: " << speed << " MB/s\r";
    std::cout.flush();
}

void copyFile(const std::string &source, const std::string &destination)
{
    std::ifstream src(source, std::ios::binary);
    std::ofstream dst(destination, std::ios::binary);

    if (!src.is_open() || !dst.is_open())
    {
        std::cerr << "\nError opening files!" << std::endl;
        return;
    }

    // Get the total size of the source file
    src.seekg(0, std::ios::end);
    size_t totalSize = src.tellg();
    src.seekg(0, std::ios::beg);

    char buffer[4096];
    size_t bytesRead = 0;
    auto startTime = std::chrono::high_resolution_clock::now();

    while (src.read(buffer, sizeof(buffer)))
    {
        dst.write(buffer, src.gcount());
        bytesRead += src.gcount();

        // Calculate elapsed time and speed
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = currentTime - startTime;
        double speed = (bytesRead / (1024.0 * 1024.0)) / elapsed.count(); // MB/s

        showProgress(bytesRead, totalSize, speed);
    }

    // Copy any remaining bytes
    dst.write(buffer, src.gcount());
    bytesRead += src.gcount();

    // Final update for the last chunk
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = currentTime - startTime;
    double speed = (bytesRead / (1024.0 * 1024.0)) / elapsed.count(); // MB/s
    showProgress(bytesRead, totalSize, speed);

    std::cout << std::endl
              << "\nCopy completed!" << std::endl;
}

bool verifyFiles(const std::string &source, const std::string &destination)
{
    std::ifstream src(source, std::ios::binary);
    std::ifstream dst(destination, std::ios::binary);

    if (!src.is_open() || !dst.is_open())
    {
        std::cerr << "\nError opening files for verification!" << std::endl;
        return false;
    }

    char srcBuffer[4096];
    char dstBuffer[4096];

    while (src.read(srcBuffer, sizeof(srcBuffer)) && dst.read(dstBuffer, sizeof(dstBuffer)))
    {
        if (src.gcount() != dst.gcount() || ::memcmp(srcBuffer, dstBuffer, src.gcount()) != 0)
        {
            return false; // Files differ
        }
    }

    // Check for any remaining bytes
    if (src.gcount() != dst.gcount())
    {
        return false; // Different sizes
    }

    return true; // Files are identical
}

std::string getCurrentDirectory()
{
    char buffer[FILENAME_MAX];
    if (_getcwd(buffer, FILENAME_MAX))
    {
        return std::string(buffer);
    }
    return "";
}

std::string getFileName(const std::string &path)
{
    return fs::path(path).filename().string();
}

bool fileExists(const std::string &path)
{
    return fs::exists(path);
}

bool isDirectory(const std::string &path)
{
    return fs::is_directory(path);
}

bool askForOverwrite()
{
    char response;
    std::cout << "\nFile already exists. Overwrite? (y/n): ";
    std::cin >> response;
    return (response == 'y' || response == 'Y');
}

int main(int argc, char *argv[])
{
    bool verify = false;
    bool overwrite = false;
    bool skip = false;
    std::string source;
    std::string destination;

    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "/v")
        {
            verify = true;
        }
        else if (std::string(argv[i]) == "/y")
        {
            overwrite = true;
        }
        else if (std::string(argv[i]) == "/n")
        {
            skip = true;
        }
        else if (std::string(argv[i]) == "/?")
        {
            showHelp();
            return 0;
        }
        else if (source.empty())
        {
            source = argv[i];
        }
        else
        {
            destination = argv[i];
        }
    }

    if (source.empty())
    {
        std::cerr << "\nUsage: bcopy.exe [/?] [/v] [/y] [/n] [source] [destination]" << std::endl;
        return 1;
    }

    if (destination.empty())
    {
        // If destination is not provided, copy to the current directory
        destination = getCurrentDirectory() + "\\" + getFileName(source);
    }
    else if (isDirectory(destination))
    {
        // If destination is a directory, append the filename to it
        destination = (fs::path(destination) / getFileName(source)).string();
    }

    bool fileExistsFlag = fileExists(destination);

    if (fileExistsFlag)
    {
        if (skip)
        {
            std::cout << "\nFile already exists. Skipping copy." << std::endl;
            // Still verify if /v is provided
            if (verify)
            {
                if (verifyFiles(source, destination))
                {
                    std::cout << "Verification successful: Files are identical." << std::endl;
                }
                else
                {
                    std::cerr << "Verification failed: Files differ!" << std::endl;
                }
            }
            return 0;
        }
        else if (!overwrite)
        {
            if (!askForOverwrite())
            {
                std::cout << "Copy operation cancelled." << std::endl;
                return 0;
            }
        }
    }

    if (FILE *file = fopen(source.c_str(), "r"))
    {
        fclose(file);
    }
    else
    {
        std::cerr << "\nSource file does not exist!" << std::endl;
        return 1;
    }

    copyFile(source, destination);

    if (verify)
    {
        if (verifyFiles(source, destination))
        {
            std::cout << "Verification successful: Files are identical." << std::endl;
        }
        else
        {
            std::cerr << "Verification failed: Files differ!" << std::endl;
        }
    }

    return 0;
}
