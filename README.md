# fileserver_cpp
Client-server File server with authentication and send, receive, modify capabilities.

Sample execution:<br>
$sudo ./server (first)<br>
$sudo ./client (in another directory)<br>
connect guest<br>
create user myusername<br>
login myusername<br>
ls<br>
send myfile.txt<br>
ls<br>
write -f myfile.txt<br>
SERVER> Enter the text to write at the beginning of the file<br>
new text<br>
search -s new<br>
receive yourfile.txt<br>
quit<br>
