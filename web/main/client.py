import socket
import re


BUFFSIZE = 8192
PORT_NUM = 13002
HOSTNAME = 'localhost'

re_successful_header = re.compile(r'201: Expect packets: ([\d]+)')

class ClientResponseSignal:
    Ack = "ACK".encode('utf-8')
    Stop = "STOP".encode('utf-8')

class ErrorRetrievingFromServer(Exception):
    pass


def request(command, host=HOSTNAME, port=PORT_NUM):
    """Returns full string output of the command to the user (regardless of how
    many packets are sent from the server; they're all concatenated here).

    If unsuccessful, returns None.
    """
    s = socket.socket()
    s.connect((host, port))
    s.send(command.encode('utf-8'))

    # Learn how many packets are needed
    header = s.recv(BUFFSIZE).decode('utf-8')
    match = re_successful_header.match(header)

    if match:
        num_expected = int(match.group(1))

        # Let the server know that we're going to proceed
        s.send(ClientResponseSignal.Ack)

        # Receive as many packets as indicated
        output = ""
        for i in range(num_expected):
            output += s.recv(BUFFSIZE).decode('utf-8')
            # Signal that we have received the packet
            s.send(ClientResponseSignal.Ack)

        return output

    elif header.startswith('500'):
        raise ErrorRetrievingFromServer()
