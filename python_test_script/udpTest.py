import socket, time

IP = "127.0.0.1"
PORT = 10000

sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
sock.bind(('',PORT))
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
msg = b'Hello ESP'

while True:
    print("Everything good so far")
    data, addr = sock.recvfrom(1024)
    print("received message: " + str(data))
    sock.sendto(msg,('255.255.255.255', 10000))
    time.sleep(3)