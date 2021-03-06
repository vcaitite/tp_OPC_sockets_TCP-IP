/*
 * Servidor com WinSock 2
 *
 * Autor: V�tor Gabriel Reis Caitit� &
 *        Joao Pedro Antunes Ferreira
 *
 * NOTA: compilado com cl.exe (compilador do Visual Studio)
 */

#include "socket.h"

void __cdecl socketServer(void)
{
    mtx.lock();
    HANDLE handle;
    // Obt�m handle para a sa�da da console
    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE) {
        printf("\n\tError getting the identifier for the console\n");
    }
    SetConsoleTextAttribute(handle, WHITE);
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    SetConsoleTextAttribute(handle, HLRED);
    printf("\n\t* CONFIGURING SOCKET SERVER: \n");
    SetConsoleTextAttribute(handle, WHITE);

    printf("\n\t# Initializing the Winsock library...");
    /*
    *    Inicializando a biblioteca de Winsock. Se a biblioteca n�o for inicializada
    *    o programa encerra retornando 1.
    */
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("\t\t[-] WSASturtup failed! Error: %d\n\n", iResult);
        return;
    }
    printf("\t\t[+] Library successfully initialized!\n");


    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    printf("\t# Resolving server address and port...");
    // Resolvendo o endereco e porta do servidor
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return;
    }printf("\t\t[+] Success!\n");


    printf("\t# Creating SOCKET for connection...");
    // Criando Socket
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("\t\t\t[-] Socket failed!Error: %ld\n\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return;
    }
    printf("\t\t[+] Socket successfully created!\n");

    printf("\n\t# Setting TCP listening socket:\n");
    printf("\t# bind()...");
    // Setup TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("\t\t\t\t\t\t[-] bind() failed!Error: %d\n\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }
    printf("\t\t\t\t\t[+] Success!\n");

    freeaddrinfo(result);

    printf("\t# listen()...");
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("\t\t\t\t\t\t[-] listen() failed! Error: %d\n\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }
    printf("\t\t\t\t\t[+] Success!\n");

    printf("\t# Waiting for connections!accept().\n\n");
    mtx.unlock();
    
    // Aceitando um cliente socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("\t\t\t[-] accept() failed! Error: %d\n\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }
    printf("\t\t\t[+] Success!\n");

    //closesocket(ListenSocket);

    printf("\n\t[+] Server ready to receive data !!!\n\n");
    
    // Receive until client ends the connection
    char* sendMsg;
    char received[50];
    do {
        
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            if (!check_sequencial_number(recvbuf)) {
                SetConsoleTextAttribute(handle, HLRED);
                printf("\t\t\t\t\t[-] Invalid sequential number\n");
                SetConsoleTextAttribute(handle, WHITE);
                closesocket(ClientSocket);
                WSACleanup();
            }
            sendMsg = socketMsgTreatment(recvbuf, recvbuflen);
            if (sendMsg != NULL) {
                printf("\t[+] Bytes received: %d\n", iResult);
                memcpy(&received[0], &recvbuf[0], iResult);
                if(iResult<50)
                    received[iResult] = '\0';
                printf("\t # Received: %s\n\n", received);
                SetConsoleTextAttribute(handle, WHITE);

                if (iResult == 8) {
                    SetConsoleTextAttribute(handle, HLGREEN);
                    printf("\t[SOCKETSERVER] <- Sending wagon positioning message...\t");
                    //SetConsoleTextAttribute(handle, WHITE);
                    iSendResult = send(ClientSocket, &sendMsg[0], (POSITION_MSG_LENGHT - 1), 0);
                }
                else if (iResult == 22) {
                    SetConsoleTextAttribute(handle, HLGREEN);
                    printf("\t[SOCKETSERVER] <- Sending ACK msg to TCP / IP client... ");
                    //SetConsoleTextAttribute(handle, WHITE);
                    iSendResult = send(ClientSocket, &sendMsg[0], (ACK_MSG_LENGHT - 1), 0);
                }
                if (iSendResult == SOCKET_ERROR) {
                    SetConsoleTextAttribute(handle, HLRED);
                    printf("\t[-] send() failed!Error: %d\n", WSAGetLastError());
                    SetConsoleTextAttribute(handle, WHITE);
                    closesocket(ClientSocket);
                    WSACleanup();
                    return;
                }
                else{
                    printf("\t[+] Success!Bytes sent %d\n", iSendResult);
                    printf("\t # Message sent: %s\n\n", sendMsg);
                    SetConsoleTextAttribute(handle, WHITE);
                }

            }
            else {
                SetConsoleTextAttribute(handle, HLRED);
                printf("\t\t[-] Invalid message!\n");
                SetConsoleTextAttribute(handle, WHITE);
            }
        }
        else if (iResult == 0) {
            SetConsoleTextAttribute(handle, HLRED);
            printf("\t\t\t\t\t[-] Connection closed by the client!\n");
            SetConsoleTextAttribute(handle, WHITE);
        }
        else {
            SetConsoleTextAttribute(handle, HLRED);
            printf("\t\t\t\t\t[-] recv() failed! Error: %d\n", WSAGetLastError());
            SetConsoleTextAttribute(handle, WHITE);
            countMessages = 0;
            //closesocket(ClientSocket);
            //WSACleanup();
            printf("\t-> listen()...");
            iResult = listen(ListenSocket, SOMAXCONN);
            if (iResult == SOCKET_ERROR) {
                printf("\t\t\t\t\t\t[-] listen() failed! Error: %d\n\n", WSAGetLastError());
                closesocket(ListenSocket);
                WSACleanup();
                return;
            }
            printf("\t\t\t\t\t\t[+] Success!\n");

            printf("\t-> Waiting for connections! accept()...");
            // Aceitando um cliente socket
            ClientSocket = accept(ListenSocket, NULL, NULL);
            if (ClientSocket == INVALID_SOCKET) {
                printf("\t\t\t[-] accept() failed! Error: %d\n\n", WSAGetLastError());
                closesocket(ListenSocket);
                WSACleanup();
                return;
            }
            printf("\t\t\t[+] Success!\n");
            printf("\n\t[+] Server ready to receive data!!!\n\n");
            //return;
        }

    } while (1);

    printf("\t-> Shutting down...");
    closesocket(ListenSocket);
    // Fexando a conexao quando terminarmos
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("\t\t\t\t\t[-] shutdown() failed! Error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return;
    }
    printf("\t\t\t\t\t[+] Closed!\n");
    printf("\n\n\t### Bye!!! ###\n");
    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return;

}