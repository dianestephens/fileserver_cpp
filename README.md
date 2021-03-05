Client-server File server with authentication and send, receive, modify capabilities.

File server application that:
1. authenticates clients using a private/public key, 
2. sends and receives files, 
3. writes to remote files and 
4. searches text in remote files.

Authentication:
A client can connect to the server as a guest user by sending the following message: "connect guest".  
As a guest user, the client can:
- create user [USERNAME]
- show users - lists the users created so far
- show active - lists users who are currently logged in to the server

As an authenticated user:
- send [FILE] - send a file from the client machine to the remote server
- ls - list files on the remote server of the users directory
- receive [FILE] - sends file from the server user directory to the client
- write [OPTION] [FILE]
-   The options are: -f to write at the beginning of the file, -s to write at the end of the file, -n to overwrite the file content.
-   The server will prompt fo rthe text to write to the file.
-   search -f [FILE] -s [TEXT] - list the context from the remote file that contains the text
-   search -s [TEXT] - list the context that contains the text in all files of that user

Releases:
0.1: server init, client, authenticate users
0.2: send, receive and modify files
1.0: search text
