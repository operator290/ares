Ares is a lightweight rat written by operator290 that targets windows

Functionality:
 + Full shell access
 + File upload / download
 + Create message box
 + Persistence using registry run keys

Compile instructions:<br>
 YOU MUST SET THE SERVER IP AND PORT IN THE `client.c` CODE BEFORE COMPILING <br>
 To compile the client either run the compile.sh script or use the command: <br> `x86_64-w64-mingw32-gcc -o client.exe client.c -lws2_32 -lmswsock` <br>
 To compile the server for windows: <br>
  Run the compile.sh script or use the command: <br> `GOOS=windows GOARCH=amd64 go build -o main.exe main.go`
 To compile the server for linux:<br>
  Run the command: `go build main.go`

Known issues:<br>
 + Message box feature causes stub to stop running
 + Using changedir with arguments in a shell session will cause the client to hang and spike RAM usage on target PC. (FOUND A FIX BUT STILL TESTING, HOPEFULLY OUT SOON)

All known issues are being worked on and will be fixed soon

Usage:
 1. Open the server CLI by running `./main`
 2. Configure server listener options
 3. Start the server and wait for the victim to connect

Coming soon:
 + Loading RAT to memory so it doesn't touch disk. 
