# fileserver_cpp
Client-server File server with authentication and send, receive, modify capabilities.

Sample execution:
$sudo ./server (first)
$sudo ./client (in another directory)
connect guest
create user myusername
login myusername
ls
send myfile.txt
ls
write -f myfile.txt
SERVER> Enter the text to write at the beginning of the file
new text
search -s new
receive yourfile.txt
quit
