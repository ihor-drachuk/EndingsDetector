#include <Windows.h>
#include <cstdint>

//#define ENABLE_SECONDARY

#ifdef ENABLE_SECONDARY
#include <memory>

template<typename T, typename D>
std::unique_ptr<T, D> wrap_with_unique_ptr(T* ptr, const D& deleter)
{
    return std::unique_ptr<T, D>(ptr, deleter);
}
#endif

constexpr DWORD BufferSize = 4 * 1024 * 1024; // 4 Mb

template <DWORD N>
inline void print(const char (&msg)[N])
{
    HANDLE const hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    WriteFile(hStdOut, msg, N - 1, nullptr, nullptr);
}

static inline int logic(int argc, WCHAR* argv[])
{
    if (!argv || argc != 2)
        return 1;

    // Open file for reading
    HANDLE hFile = CreateFileW(argv[1],               // File path from argument
                               GENERIC_READ,          // Open for reading
                               FILE_SHARE_READ,       // Share for reading
                               nullptr,               // Default security
                               OPEN_EXISTING,         // Open only if exists
                               FILE_ATTRIBUTE_NORMAL, // Normal file
                               nullptr                // No template
                               );

    if (hFile == INVALID_HANDLE_VALUE)
        return 2;

#ifdef ENABLE_SECONDARY
    auto hFileGuard = wrap_with_unique_ptr(hFile, [](HANDLE x){ CloseHandle(x); });
#endif

    // Read file content
    char buffer[BufferSize];
    DWORD bytesRead;
    if (!ReadFile(hFile, buffer, BufferSize, &bytesRead, nullptr))
        return 5;

    // Find CRLF, CR, LF endings
    bool hasCrlf {false};
    bool hasCr {false};
    bool hasLf {false};

    auto ptr = buffer;
    const auto ptrEnd = buffer + bytesRead - 1; // Reserve 1 additional byte for algorithm optimization
    while (ptr < ptrEnd) {
        if (*reinterpret_cast<uint16_t*>(ptr) == 2573) {
            hasCrlf = true;
            ptr += 2;

        } else {
            if (*ptr == '\r') {
                hasCr = true;
            } else if (*ptr == '\n') {
                hasLf = true;
            }

            ++ptr;
        }
    }

    // Detect and print result
    auto count = hasCrlf + hasCr + hasLf;
    if (count == 0) {
        print("None\n");

    } else if (count == 1) {
        hasCrlf ? print("CRLF\n") :
        hasCr   ? print("CR\n") :
                  print("LF\n") ;
    } else {
        print("Mixed\n");
    }

    return 0;
}

//extern "C" int WinMain(
//    HINSTANCE /*hInstance*/,
//    HINSTANCE /*hPrevInstance*/,
//    LPSTR     /*lpCmdLine*/,
//    int       /*nCmdShow*/
//    )

//extern "C" int main()

extern "C" [[noreturn]] void mainCRTStartup()
{
    // Get parameters
    LPWSTR const cmdLine = GetCommandLineW();
    int numArgs;
    LPWSTR* argv = CommandLineToArgvW(cmdLine, &numArgs);

    // Do everything
    const auto status = logic(numArgs, argv);

#ifdef ENABLE_SECONDARY
    LocalFree(argv);
#endif

    ExitProcess(status);
}
