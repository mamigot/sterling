"""Defines workflow through which requests are made to the server"""
import socket
import re
import os


# Qualities of the server we're connecting to
DATASERVER_HOST = os.environ['DATASERVER_HOST']
DATASERVER_PORT = int(os.environ['DATASERVER_PORT'])
DATASERVER_BUFFSIZE = int(os.environ['DATASERVER_BUFFSIZE'])

re_successful_header = re.compile(r'201: Expect packets: ([\d]+)')

class ClientResponseSignal:
    Ack = "ACK".encode('utf-8')
    Stop = "STOP".encode('utf-8')

class ErrorRetrievingFromServer(Exception):
    pass


def request(command, host=DATASERVER_HOST, port=DATASERVER_PORT):
    """Returns full string output of the command to the user (regardless of how
    many packets are sent from the server; they're all concatenated here).

    If unsuccessful, returns None.
    """
    s = socket.socket()
    s.connect((host, port))
    s.send(command.encode('utf-8'))

    # Learn how many packets are needed
    header = parse_as_utf8(s.recv(DATASERVER_BUFFSIZE))
    match = re_successful_header.match(header)

    if match:
        num_expected = int(match.group(1))

        # Let the server know that we're going to proceed
        s.send(ClientResponseSignal.Ack)

        # Receive as many packets as indicated
        output = ""
        for i in range(num_expected + 1):
            output += parse_as_utf8(s.recv(DATASERVER_BUFFSIZE))
            # Signal that we have received the packet
            s.send(ClientResponseSignal.Ack)

        return output

    elif header.startswith('500'):
        raise ErrorRetrievingFromServer()

def parse_as_utf8(buffer_bytes):
    """Read until we encounter the '\x00' (control character) in UTF-8. Assumes
    that read bytes will be terminated with this. Then, decode accordingly."""

    end_idx = buffer_bytes.find(b'\x00')
    return (buffer_bytes[:end_idx]).decode('utf-8')
