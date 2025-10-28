#include <iostream>
#include <string>
#include <vector>
#include <windows.h> // For playing sound on Windows
#include <fstream>   // CHANGE 1: Include fstream for file checking

#pragma comment(lib, "winmm.lib") // Link the Windows Multi-Media library

// This structure holds information about a single song
struct Song
{
    std::string title;
    std::string filepath;
};

// --- Global variables for our player's state ---
std::vector<Song> library;
int currentTrackIndex = -1;

// --- Functions to control the music ---
void playSong(int trackIndex)
{
    if (trackIndex < 0 || trackIndex >= library.size())
    {
        std::cout << "Invalid track number.\n";
        return;
    }

    currentTrackIndex = trackIndex;
    std::string songPath = library[currentTrackIndex].filepath;

    // --- CHANGE 2: Check if the file exists BEFORE trying to play it ---
    std::ifstream fileCheck(songPath);
    if (!fileCheck.good())
    {
        std::cout << "Error: Cannot find or access the song file at path: " << songPath << std::endl;
        return;
    }
    fileCheck.close();
    // --- End of file check ---

    // Close any previously opened music to prevent conflicts
    mciSendStringA("close music_player", NULL, 0, NULL);

    // Command to open the audio file
    std::string openCommand = "open \"" + songPath + "\" type waveaudio alias music_player";

    // Try to open the file, but with a simplified error message
    if (mciSendStringA(openCommand.c_str(), NULL, 0, NULL) != 0)
    {
        // CHANGE 3: Simplified, user-friendly error message
        std::cout << "Error: Could not open the song. The file might be corrupt or in an unsupported format." << std::endl;
        return;
    }

    // Try to play the file
    if (mciSendStringA("play music_player", NULL, 0, NULL) != 0)
    {
        std::cout << "Error: Could not play the song." << std::endl;
        mciSendStringA("close music_player", NULL, 0, NULL); // Clean up
        return;
    }

    std::cout << "Now playing: " << library[currentTrackIndex].title << std::endl;
}

void stopSong()
{
    mciSendStringA("close music_player", NULL, 0, NULL);
    std::cout << "Playback stopped.\n";
    currentTrackIndex = -1;
}

void playNext()
{
    if (library.empty())
    {
        std::cout << "Library is empty.\n";
        return;
    }
    // Simple logic to go to the next song, wrapping around to the beginning
    int nextTrack = (currentTrackIndex + 1) % library.size();
    playSong(nextTrack);
}

void playPrevious()
{
    if (library.empty())
    {
        std::cout << "Library is empty.\n";
        return;
    }
    int prevTrack;
    if (currentTrackIndex < 0)
    {
        prevTrack = (int)library.size() - 1;
    }
    else
    {
        prevTrack = (currentTrackIndex - 1 + (int)library.size()) % (int)library.size();
    }
    playSong(prevTrack);
}

// --- GUI (Win32) ---

// Control IDs
#define IDC_LISTBOX 1001
#define IDC_BTN_PLAY 1002
#define IDC_BTN_NEXT 1003
#define IDC_BTN_STOP 1004
#define IDC_BTN_PREV 1005

HWND g_mainWindow = NULL;
HWND g_listBox = NULL;
HWND g_btnPlay = NULL;
HWND g_btnNext = NULL;
HWND g_btnStop = NULL;
HWND g_btnPrev = NULL;

static void initializeLibrary()
{
    if (!library.empty())
    {
        return;
    }
    library.push_back({"Lover", "C:\\Users\\Aastha\\OneDrive\\Desktop\\MusicPlayer\\assests\\music\\song1.wav"});
    library.push_back({"Ok J title track", "C:\\Users\\Aastha\\OneDrive\\Desktop\\MusicPlayer\\assests\\music\\song2.wav"});
    library.push_back({"Sunflower", "C:\\Users\\Aastha\\OneDrive\\Desktop\\MusicPlayer\\assests\\music\\song3.wav"});
    library.push_back({"sky full of stars", "C:\\Users\\Aastha\\OneDrive\\Desktop\\MusicPlayer\\assests\\music\\song4.wav"});
    library.push_back({"Shape of you", "C:\\Users\\Aastha\\OneDrive\\Desktop\\MusicPlayer\\assests\\music\\song5.wav"});
    library.push_back({"Let me love you", "C:\\Users\\Aastha\\OneDrive\\Desktop\\MusicPlayer\\assests\\music\\song6.wav"});
    library.push_back({"Despacito", "C:\\Users\\Aastha\\OneDrive\\Desktop\\MusicPlayer\\assests\\music\\song7.wav"});
    library.push_back({"Perfect", "C:\\Users\\Aastha\\OneDrive\\Desktop\\MusicPlayer\\assests\\music\\song8.wav"});
    library.push_back({"Attention", "C:\\Users\\Aastha\\OneDrive\\Desktop\\MusicPlayer\\assests\\music\\song9.wav"});
    library.push_back({"Cheap Thrills", "C:\\Users\\Aastha\\OneDrive\\Desktop\\MusicPlayer\\assests\\music\\song10.wav"});
}

static void populateListBox()
{
    if (!g_listBox)
    {
        return;
    }
    SendMessageA(g_listBox, LB_RESETCONTENT, 0, 0);
    for (size_t i = 0; i < library.size(); ++i)
    {
        SendMessageA(g_listBox, LB_ADDSTRING, 0, (LPARAM)library[i].title.c_str());
    }
}

static void layoutControls(int width, int height)
{
    const int padding = 10;
    const int buttonHeight = 32;
    const int buttonWidth = 80;
    const int listTop = padding;
    const int listLeft = padding;
    const int listRight = padding;
    const int listBottom = padding + buttonHeight + padding;

    int listWidth = (width - listLeft - listRight);
    int listHeight = (height - listTop - listBottom);

    MoveWindow(g_listBox, listLeft, listTop, listWidth, listHeight, TRUE);

    int btnTop = listTop + listHeight + padding;
    int btnLeft = listLeft;
    MoveWindow(g_btnPlay, btnLeft, btnTop, buttonWidth, buttonHeight, TRUE);
    btnLeft += buttonWidth + padding;
    MoveWindow(g_btnPrev, btnLeft, btnTop, buttonWidth, buttonHeight, TRUE);
    btnLeft += buttonWidth + padding;
    MoveWindow(g_btnNext, btnLeft, btnTop, buttonWidth, buttonHeight, TRUE);
    btnLeft += buttonWidth + padding;
    MoveWindow(g_btnStop, btnLeft, btnTop, buttonWidth, buttonHeight, TRUE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        g_mainWindow = hWnd;
        initializeLibrary();

        g_listBox = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "",
                                    WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | WS_BORDER,
                                    0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)IDC_LISTBOX, GetModuleHandleA(NULL), NULL);

        g_btnPlay = CreateWindowExA(0, "BUTTON", "Play",
                                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)IDC_BTN_PLAY, GetModuleHandleA(NULL), NULL);

        g_btnPrev = CreateWindowExA(0, "BUTTON", "Prev",
                                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)IDC_BTN_PREV, GetModuleHandleA(NULL), NULL);

        g_btnNext = CreateWindowExA(0, "BUTTON", "Next",
                                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)IDC_BTN_NEXT, GetModuleHandleA(NULL), NULL);

        g_btnStop = CreateWindowExA(0, "BUTTON", "Stop",
                                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)IDC_BTN_STOP, GetModuleHandleA(NULL), NULL);

        populateListBox();

        RECT rc;
        GetClientRect(hWnd, &rc);
        layoutControls(rc.right - rc.left, rc.bottom - rc.top);
        return 0;
    }
    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        layoutControls(width, height);
        return 0;
    }
    case WM_COMMAND:
    {
        int controlId = LOWORD(wParam);
        int notifyCode = HIWORD(wParam);

        if (controlId == IDC_LISTBOX && notifyCode == LBN_DBLCLK)
        {
            LRESULT sel = SendMessageA(g_listBox, LB_GETCURSEL, 0, 0);
            if (sel != LB_ERR)
            {
                playSong((int)sel);
            }
            return 0;
        }

        if (notifyCode == BN_CLICKED)
        {
            switch (controlId)
            {
            case IDC_BTN_PREV:
                playPrevious();
                if (!library.empty())
                {
                    int index = currentTrackIndex >= 0 ? currentTrackIndex : (int)library.size() - 1;
                    SendMessageA(g_listBox, LB_SETCURSEL, (WPARAM)index, 0);
                }
                return 0;
            case IDC_BTN_PLAY:
            {
                LRESULT sel = SendMessageA(g_listBox, LB_GETCURSEL, 0, 0);
                if (sel == LB_ERR)
                {
                    // If nothing selected, play first
                    if (!library.empty())
                    {
                        SendMessageA(g_listBox, LB_SETCURSEL, 0, 0);
                        playSong(0);
                    }
                }
                else
                {
                    playSong((int)sel);
                }
                return 0;
            }
            case IDC_BTN_NEXT:
                playNext();
                if (!library.empty())
                {
                    int index = currentTrackIndex >= 0 ? currentTrackIndex : 0;
                    SendMessageA(g_listBox, LB_SETCURSEL, (WPARAM)index, 0);
                }
                return 0;
            case IDC_BTN_STOP:
                stopSong();
                return 0;
            default:
                break;
            }
        }
        break;
    }
    case WM_DESTROY:
        stopSong();
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcA(hWnd, message, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    const char *kClassName = "MusicPlayerWindow";

    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = kClassName;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExA(&wc))
    {
        MessageBoxA(NULL, "Failed to register window class", "Error", MB_ICONERROR | MB_OK);
        return 0;
    }

    HWND hWnd = CreateWindowExA(
        0,
        kClassName,
        "C++ Music Player",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 400,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!hWnd)
    {
        MessageBoxA(NULL, "Failed to create window", "Error", MB_ICONERROR | MB_OK);
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}
