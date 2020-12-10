#include <sys/select.h>

#include "SocketUtil.h"
#include "UDPSocket.h"

UDPSocket::UDPSocket() { FD_ZERO(&read_fds); }

int UDPSocket::Bind(const SocketAddress& inBindAddress)
{
    int error = bind(mSocket, &inBindAddress.mSockAddr, inBindAddress.GetSize());
    if (error != 0)
    {
        SocketUtil::ReportError("UDPSocket::Bind");
        return SocketUtil::GetLastError();
    }

    FD_SET(mSocket, &read_fds);

    if (pipe(close_pipe))
    {
        std::cout << "Failed to bind pipe" << std::endl;
    }

    // add the read pipe to the fd set
    FD_SET(close_pipe[0], &read_fds);

    return NO_ERROR;
}

int UDPSocket::SendTo(const void* inToSend, uint32_t inLength, const SocketAddress& inToAddress)
{
    int byteSentCount = (int)sendto(mSocket, static_cast<const char*>(inToSend), inLength, 0, &inToAddress.mSockAddr,
                                    inToAddress.GetSize());
    if (byteSentCount <= 0)
    {
        // we'll return error as negative number to indicate less than requested
        // amount of bytes sent...
        SocketUtil::ReportError("UDPSocket::SendTo");
        return -SocketUtil::GetLastError();
    }
    else
    {
        return byteSentCount;
    }
}

int UDPSocket::ReceiveFrom(void* inToReceive, int inMaxLength, SocketAddress& outFromAddress)
{
    socklen_t fromLength = outFromAddress.GetSize();

    int activity = select(FD_SETSIZE, &read_fds, NULL, NULL, NULL);

    if (FD_ISSET(mSocket, &read_fds))
    {
        int readByteCount = (int)recvfrom(mSocket, static_cast<char*>(inToReceive), inMaxLength, 0,
                                          &outFromAddress.mSockAddr, &fromLength);

        if (readByteCount >= 0)
        {
            return readByteCount;
        }
        else
        {
            int error = SocketUtil::GetLastError();

            if (error == WSAEWOULDBLOCK)
            {
                return 0;
            }
            else if (error == WSAECONNRESET)
            {
                // this can happen if a client closed and we haven't DC'd yet.
                // this is the ICMP message being sent back saying the port on that
                // computer is closed
                //          LOG( "Connection reset from %s",
                // outFromAddress.ToString().c_str() );
                return -WSAECONNRESET;
            }
            else
            {
                SocketUtil::ReportError("UDPSocket::ReceiveFrom");
                return -error;
            }
        }
    }

    if (FD_ISSET(close_pipe[0], &read_fds))
    {
        return -1;
    }

    return 0;
}

void UDPSocket::Shutdown()
{
    // TODO: Something different for windows
    // Shutdown socket to stop any more data from coming in
    shutdown(mSocket, SHUT_RDWR);
    // Pipe something to the close pipe to unstick the select
    FILE* stream;
    stream = fdopen(close_pipe[1], "w");
    fprintf(stream, "CLOSE");
    fclose(stream);
}

UDPSocket::~UDPSocket()
{
#if _WIN32
    closesocket(mSocket);
#else
    close(mSocket);
#endif
    close(close_pipe[0]);
    close(close_pipe[1]);
}

int UDPSocket::SetNonBlockingMode(bool inShouldBeNonBlocking)
{
#if _WIN32
    u_long arg = inShouldBeNonBlocking ? 1 : 0;
    int result = ioctlsocket(mSocket, FIONBIO, &arg);
#else
    int flags = fcntl(mSocket, F_GETFL, 0);
    flags = inShouldBeNonBlocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    int result = fcntl(mSocket, F_SETFL, flags);
#endif

    if (result == SOCKET_ERROR)
    {
        SocketUtil::ReportError("UDPSocket::SetNonBlockingMode");
        return SocketUtil::GetLastError();
    }
    else
    {
        return NO_ERROR;
    }
}
