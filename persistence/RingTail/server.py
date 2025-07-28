import socket

def cmd_check(message):
    words = message.split()
    if not words:
        return False

    if words[0] in command_reg:
        return True
    else:
        return False

command_reg = {
    "exit",
    "ls",
}

# Create a socket object
s = socket.socket()
print("Socket successfully created")
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Reserve a port
port = 4444

# Bind to the port
s.bind(('', port))
print(f"Socket bound to port {port}")

# Put the socket into listening mode
s.listen(5)
print("Socket is listening...")

logo = r"""
                    ⠀⠀⠀⠀⢀⣠⠤⠒⠒⠒⠒⠒⠒⠤⠤⣀⣀⣀⣀⣀⡤⠤⠤⠄⠤⡤⣲⣽⡇⠀
                    ⠀⠀⡠⠖⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠁⠉⠁⠀⠀⠚⠛⠿⠰⠟⣿⡇⠀
                    ⠀⡴⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡏⠀⠀⠀⠀⠙⡄⠀
                    ⢰⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣇⠀⢸⣿⣷⣦⡇⠀
                    ⣼⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡿⠷⣄⠉⠛⠈⠀⠀
                    ⣇⠀⠀⠀⠀⣧⠀⠀⠀⠀⠀⠈⢦⠀⠀⠀⠀⠀⠀⠀⠀⣼⣆⠀⠈⢂⠀⣀⣡⠀
                    ⠹⣴⣿⣶⣶⣿⣆⠀⠀⠀⠀⠀⠈⣷⡀⠀⠀⠀⠀⠀⠰⠿⠿⠦⠤⠶⣷⣿⣧⣄
                    ⠀⢻⣩⣶⣿⣾⣏⠢⠀⠀⠀⠀⠀⢻⠈⠑⢄⠀⠀⢀⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀
                    ⠀⠈⠿⢋⣥⣶⣿⣆⠑⡀⠀⠀⠀⡞⠀⠀⠈⢶⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
                    ⠀⠀⠈⢼⡿⠋⢀⣠⣵⡘⢦⣤⣬⣤⣴⣤⣄⠘⠿⠶⠄⠀⠀⠀⠀⠀⠀⠀⠀⠀
                    ⠀⠀⠀⠀⠁⢴⣿⡟⠉⢑⡌⠉⠉⠉⠛⠳⠄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
                    ⠀⠀⠀⠀⠀⠀⠙⢄⣾⡿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
   (`-')   _     <-. (`-')_               (`-')     (`-')  _   _              
<-.(OO )  (_)       \( OO) )    .->       ( OO).->  (OO ).-/  (_)      <-.    
,------,) ,-(`-'),--./ ,--/  ,---(`-')    /    '._  / ,---.   ,-(`-'),--. )   
|   /`. ' | ( OO)|   \ |  | '  .-(OO )    |'--...__)| \ /`.\  | ( OO)|  (`-') 
|  |_.' | |  |  )|  . '|  |)|  | .-, \    `--.  .--''-'|_.' | |  |  )|  |OO ) 
|  .   .'(|  |_/ |  |\    | |  | '.(_/       |  |  (|  .-.  |(|  |_/(|  '__ | 
|  |\  \  |  |'->|  | \   | |  '-'  |        |  |   |  | |  | |  |'->|     |' 
`--' '--' `--'   `--'  `--'  `-----'         `--'   `--' `--' `--'   `-----'  
"""

print(logo)
print("========================= Listening for Connections =========================")

# A forever loop to accept client connections
while True:
    c = None
    try:
        c, addr = s.accept()
        print("\nGot connection from", addr)

        while True:
            # Get input from the server user
            message_to_send = input("RingTail > ")
            if not message_to_send.strip():
                continue
            
            if cmd_check(message_to_send) == True:
                c.send(message_to_send.encode('utf-8'))
                if message_to_send == "exit":
                    break
            else:
                command = message_to_send.split()
                print(f"ERROR: Invalid command: {command[0]}")
                continue

            response = c.recv(4096).decode('utf-8')
            print(response)
    except socket.error:
        print(f"Connection with {addr} lost.")
        print(socket.error)
        break
    except KeyboardInterrupt:
        print("\nServer shutting down.")
        break
    finally:
        if c:
            c.close()

s.close()