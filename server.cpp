#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <dirent.h>
#include <string>
#include <fstream>
#include <cstring>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <vector>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <iterator>
#include <stdlib.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>
#include <sys/sendfile.h>
#include <iostream>
#include <filesystem>
#include <cstdio>

using namespace std;

void * connection_thread(void* p_client_socket);

long GetFileSize(std::string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

int main(int argc, char *argv[])
{

  int sockfd, clientsocket, port;
  socklen_t clen;
  struct sockaddr_in serv_addr, clientInfo;

  // create a tcp (SOCK_STREAM) socket
  sockfd =  socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("error:: Could not open socket.");
    exit(1);
  }
  // clear address structure
  bzero((char *) &serv_addr, sizeof(serv_addr));

  port = 1003; // hard code port #
  // setup the host_addr structure
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port); // network byte order

  // bind socket to port
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    perror("error:: unable to bind");
  // listen for incoming connections - backlog queue set to 5.
  listen(sockfd,50);

  while (true) { // loop accepting connections
    clen = sizeof(clientInfo);
    printf("Ready for connections ...\n");
    // accept connection from client socket and put the client socket info in clientInfo
    clientsocket  = accept(sockfd,(struct sockaddr *) &clientInfo, &clen);
    if (clientsocket < 0)
      perror("error:: on accept");

    printf("Connected to %s on port %d\n", inet_ntoa(clientInfo.sin_addr), ntohs(clientInfo.sin_port));

    pthread_t t;
    int *pclient = (int *)malloc(sizeof(int));
    *pclient = clientsocket;
    pthread_create(&t, NULL, connection_thread, pclient);
  } // while
  return 0;
}

void * connection_thread(void* p_client_socket) {
  int clientsocket = *((int*)p_client_socket);
  free(p_client_socket);
  char buffer[256];
  char welcome[256] = "Welcome to fileserver.\n";

  // control flow bools
  int logout = 0;
  int isGuest = 0;
  int isUser = 0;

  // user info
  std::string currentUser;

  // welcome message
  send(clientsocket, welcome, sizeof(welcome), 0);

  // big while loop
  do{
    memset(buffer, 0, 256);

    // file io vars
    int filesize = 0, filehandle = 0;
    char filename[20];
cout << "waiting for client cmd\n";
    // wait for client input
    int n = read(clientsocket, buffer, 256);
    if (n < 0)
      perror("ERROR reading from socket");

    std::string cmd(buffer);

    printf("Command: %s\n", cmd.c_str());

    if(cmd.find("connect guest") != std::string::npos || isGuest == 1){
      if(isGuest == 0){
        printf("login guest\n");
        char welcome_guest[256] = "Welcome Guest User";
        send(clientsocket, welcome_guest, strlen(welcome_guest), 0);
        memset(welcome_guest, 0, 256);
      }

      isGuest = 1;

      n = read(clientsocket, buffer, 256);
      // show registered users
      if(strcmp(buffer, "show users") == 0){
        printf("show users\n");
        char users[256];
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir("users/")) != NULL) {
          while ((ent = readdir(dir)) != NULL) {
            if(strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0){
              strcat(users, ent->d_name);
              char space[2] = " ";
              strcat(users, space);
            }
          }
          send(clientsocket, users, strlen(users), 0);
          memset(users, 0, 256);
          closedir(dir);
        }
        else{
          printf("error opening user directory");
        }
      }
      //show active users
      else if(strcmp(buffer, "show active") == 0){
        printf("show active\n");
        std::ifstream active_users_file("active");
        char active_users[256];
        std::string line;

        if(active_users_file.is_open()){
          while(getline(active_users_file, line)){
            char temp[line.length() + 1];
            strcpy(temp, line.c_str());
            char space[2] = " ";
            strcat(active_users, temp);
            strcat(active_users, space);

          }
          active_users_file.close();
        }
        send(clientsocket, active_users, sizeof(active_users), 0);
        memset(active_users, 0, 256);
      }
      else if(strcmp(std::string(buffer).substr(0,11).c_str(), "create user") == 0){
        std::string create_user(buffer);
        create_user = create_user.substr(12);

        int already_created = 0;
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir("users/")) != NULL) {
          while ((ent = readdir(dir)) != NULL) {
              if(strcmp(ent->d_name, create_user.c_str()) == 0){
                already_created = 1;
              }
            }
            closedir(dir);
        }

        if(already_created == 1){
          char create[256] = "User is already created. Please try again.";
          send(clientsocket, create, sizeof(create), 0);
          memset(create, 0, 256);
        }
        else{
          system(("mkdir -p users/"+create_user).c_str());
          std::ofstream user_pub_file("users/"+create_user+"/txt.pub");
          std::ofstream user_enc_file("users/"+create_user+"/txt.encrypt");

          char req_pub[256] = "request public key";
          send(clientsocket, req_pub, sizeof(req_pub), 0);
          memset(req_pub, 0, 256);

          char pub_key[256];
          n = read(clientsocket,pub_key,256);

          user_pub_file << pub_key;
          memset(pub_key, 0, 256);


          char req_encrypt[256] = "request encrypted";
          send(clientsocket, req_encrypt, sizeof(req_encrypt), 0);
          memset(req_encrypt, 0, 256);

          char encrypt_data[256];
          n = read(clientsocket,encrypt_data,256);

          user_enc_file << encrypt_data;
          memset(encrypt_data, 0, 256);

          printf("user creation successful\n");

          char successful_creation[256] = "User creation successful. Please login.";
          send(clientsocket, successful_creation, sizeof(successful_creation), 0);
          memset(successful_creation, 0, 256);

          isGuest = 0;
          }
        }
        // logout guest user
        else if(strcmp(buffer, "logout") == 0){
          char bye[256] = "Goodbye\n";
          send(clientsocket, bye, sizeof(bye), 0);
          logout = 1;
        }
        //invalid guest user command
        else{
          char invalid_cmd[256] = "Invalid command. Try again.";
          printf("invalid command\n");
          send(clientsocket, invalid_cmd, sizeof(invalid_cmd), 0);
          memset(invalid_cmd, 0, 256);
        }
    }
    //login process
    else if(cmd.find("login") != std::string::npos){
      printf("login attempt\n");

      int created = 0;

      if(isUser == 0){
        //check if user is created
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir("users/")) != NULL) {
          while ((ent = readdir(dir)) != NULL) {
              if(strcmp(ent->d_name, cmd.substr(cmd.find(" ")+1).c_str()) == 0){
                created = 1;
                currentUser = cmd.substr(cmd.find(" ")+1);
              }
            }
            closedir(dir);
        }
        else{
            printf("error opening user directory");
        }

        //user is not created, error
        if(created == 0){
          char create[256] = "User is not created. Please try again.";
          send(clientsocket, create, sizeof(create), 0);
          memset(create, 0, 256);
          currentUser = "";
        }
        //user is created
        else{
          char request_encrypted[256] = "request encrypted data";
          send(clientsocket, request_encrypted, sizeof(request_encrypted), 0);
          memset(request_encrypted, 0, 256);

          // wait for response
          char encrypted[256];
          n = read(clientsocket,encrypted,256);

          // check if encrypted data = data stored in user file
          std::ifstream user_encrypt_file("users/"+currentUser+"/txt.encrypt");
          std::string line;

          if(user_encrypt_file.is_open()){
            while(getline(user_encrypt_file, line)){
              if(strcmp(line.c_str(), encrypted) == 0){
                printf("login successful\n");
                memset(encrypted, 0, 256);

                // request new encrypt data
                char request_new_encrypted[256] = "request new encrypted data";
                send(clientsocket, request_new_encrypted, sizeof(request_new_encrypted), 0);
                memset(request_new_encrypted, 0, 256);

                // wait for client
                char new_encrypted[256];
                n = read(clientsocket,new_encrypted,256);

                // clear encrypted file
                std::ofstream ofs;
                ofs.open("users/"+currentUser+"/txt.encrypt", std::ofstream::out | std::ofstream::trunc);
                ofs.close();

                // add new encrypt data to file
                std::ofstream user_new_enc_file("users/"+currentUser+"/txt.encrypt");
                user_new_enc_file << new_encrypted;
                memset(new_encrypted, 0, 256);

                char auth_success[256] = "Successful authentication";
                send(clientsocket, auth_success, sizeof(auth_success), 0);
                memset(auth_success, 0, 256);

                isUser = 1;

                printf("current user: \"%s\"\n", currentUser.c_str());

              }
              else{
                printf("login failed\n");
                char auth[256] = "Wrong authentication";
                send(clientsocket, auth, sizeof(auth), 0);
                memset(auth, 0, 256);
                created = 0;
                currentUser = "";
              }
            }
            user_encrypt_file.close();
          }
          else{
            printf("error finding user encrypted file\n");
            created = 0;
            currentUser = "";
          }

        }
      }
    }
    // client is user
    else if (isUser == 1)
    {
      if (cmd.substr(0, cmd.find(' ')) == "search"){

        // error checking
        int exists = 0;

        // file specified, search file
        if(cmd.find("-f") != std::string::npos){
          std::string searchFile = (cmd.substr(cmd.find("-f")+3)).substr(0, (cmd.substr(cmd.find("-f")+3)).find(' '));
          std::string text = (cmd.substr(cmd.find("-s")+3)).substr(0, (cmd.substr(cmd.find("-s")+3)).find(' '));
          if(text.length() == 0){
            text = (cmd.substr(cmd.find("-s")+3)).substr(0, (cmd.substr(cmd.find("-s")+3)).find('\n'));
          }

          // get currentUser directory
          const char* directory = ("users/" + currentUser).c_str();

          // directory vars
          DIR *dir;
          struct dirent *ent;

          // open dir
          if ((dir = opendir(directory)) != NULL){
            // cycle through dir
            while ((ent = readdir(dir)) != NULL){
              if (strcmp(ent->d_name, searchFile.c_str()) == 0){
                exists = 1;

                std::ifstream userfile("users/" + currentUser + "/" + searchFile);
                std::string line;
                std::string ret;

                // add file text
                if(userfile.is_open()){
                  int i = 1;
                  while(getline(userfile, line)){
                    if(line.find(text.c_str()) != std::string::npos){
                      string temp(ent->d_name);
                      ret += temp + " contains the text in line " + std::to_string(i) + ". Line: " + line + "\n";
                    }
                    i++;
                  } // end adding file text

                  // send sentences to client
                  send(clientsocket, ret.c_str(), strlen(ret.c_str()), 0);
                }
              } // end if
            } // end dir reading
          }// end dir
          else{
            char message[256] = "Error reading directory\n";
            send(clientsocket, message, sizeof(message), 0);
            memset(message, 0, 256);
          }
        }
        //file not specified, search all files
        else{
          exists = 1;
          std::string text = (cmd.substr(cmd.find("-s")+3)).substr(0, (cmd.substr(cmd.find("-s")+3)).find(' '));
          if(text.length() == 0){
            text = (cmd.substr(cmd.find("-s")+3)).substr(0, (cmd.substr(cmd.find("-s")+3)).find('\n'));
          }

          std::string ret;
          std::string fileText;

          // get currentUser directory
          const char* directory = ("users/" + currentUser).c_str();

          // directory vars
          DIR *dir;
          struct dirent *ent;

          // open dir
          if ((dir = opendir(directory)) != NULL){
            // cycle through dir
            while ((ent = readdir(dir)) != NULL){
              std::string fileText;

              if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0){
                exists = 1;

                std::ifstream userfile("users/" + currentUser + "/" + ent->d_name);
                std::string line;

                // add file text
                if(userfile.is_open()){
                  int i = 1;
                  while(getline(userfile, line)){
                    if(line.find(text.c_str()) != std::string::npos){
                      string temp(ent->d_name);
                      ret += temp + " contains the text in line " + std::to_string(i) + ". Line: " + line + "\n";
                    }
                    i++;
                  } // end adding file text
                } // end if
              }
            } // end while dir reading
          }// end dir
          else{
            char message[256] = "Error reading directory\n";
            send(clientsocket, message, sizeof(message), 0);
            memset(message, 0, 256);
          }

          send(clientsocket, ret.c_str(), strlen(ret.c_str()), 0);

        }

        if(exists == 0){
          char message[256] = "File does not exist\n";
          send(clientsocket, message, sizeof(message), 0);
          memset(message, 0, 256);
        }

      }
      else if (cmd.substr(0, cmd.find(' ')) == "write")
      {
        // THE FILENAME HAS TO BE MORE THAN 3 CHARACTERS ELSE IT IS APPENING RANDOM CHARACTERS, PLEASE SOLVE THIS ISSUE ALSO
        std::string fstr = cmd.substr(cmd.find(" ") + 1, cmd.length());
        const char *fn = fstr.c_str();
        std::string fstr1 = fstr.substr(fstr.find(" ") + 1, fstr.length());
        const char *fn1 = fstr1.c_str();

        //check if file is created
        DIR *dir;
        struct dirent *ent;

        std::string directory = "users/" + currentUser; // Since I can't check in the users file I created different dierctory to test, change it to users/

        const char *currentUsername = directory.c_str();

        // ALSO I CAN'T WRITE AT THE FILE DIRECTORY(NO IDEA) SO SAVE THE FILE AT THE USERS/USERNAME DIRECTORY

        if ((dir = opendir(currentUsername)) != NULL)
        {
          while ((ent = readdir(dir)) != NULL)
          {
            if (strcmp(ent->d_name, fstr1.substr(fstr1.find(" ") + 1).c_str()) == 0)
            {
              // To write at the beginning of the file
              if (fstr.substr(0, fstr.find(' ')) == "-f")
              {
                char message[256] = "Enter the text to write at the beginning of the file\n";
                send(clientsocket, message, sizeof(message), 0);
                memset(message, 0, 256);

                memset(buffer, 0, 256);

                // wait for client input
                int n = read(clientsocket, buffer, 256);
                if (n < 0)
                  perror("ERROR reading from socket");

                std::string textInput(buffer);

                // Data can't be inserted at the start of a file on disk. We need to read the entire file into
                // memory, insert data at the beginning, and write the entire thing back to disk.

                std::fstream processedFile(("users/" + currentUser + "/" + fstr1).c_str());
                std::stringstream fileData;

                fileData << textInput;
                fileData << " ";
                fileData << processedFile.rdbuf();
                processedFile.close();

                processedFile.open(("users/" + currentUser + "/" + fstr1).c_str(), std::fstream::out | std::fstream::trunc);
                processedFile << fileData.rdbuf();
                processedFile.close();

                char successful[256] = "Successful Write\n";
                send(clientsocket, successful, sizeof(successful), 0);
                memset(successful, 0, 256);
              }
              // Writing at the end of the file
              else if (fstr.substr(0, fstr.find(' ')) == "-a")
              {
                char message[256] = "Enter the text to write at the end of the file\n";
                send(clientsocket, message, sizeof(message), 0);
                memset(message, 0, 256);

                memset(buffer, 0, 256);

                // wait for client input
                int n = read(clientsocket, buffer, 256);
                if (n < 0)
                  perror("ERROR reading from socket");

                std::string textInput(buffer);

                printf("file to append to: %s\n", fstr1.c_str());

                std::ofstream fileOUT("users/" + currentUser + "/" + fstr1, std::ios::app); // open file in append mode

                fileOUT << textInput << std::endl; // append at the end of the file
                fileOUT.close();                   // close the file

                char successful[256] = "Successful Write\n";
                send(clientsocket, successful, sizeof(successful), 0);
                memset(successful, 0, 256);
              }
              // Removing all the data and writing again
              else if (fstr.substr(0, fstr.find(' ')) == "-n")
              {
                char message[256] = "Enter the text to Over-write the file\n";
                send(clientsocket, message, sizeof(message), 0);
                memset(message, 0, 256);

                memset(buffer, 0, 256);

                // wait for client input
                int n = read(clientsocket, buffer, 256);
                if (n < 0)
                  perror("ERROR reading from socket");

                std::string textInput(buffer);

                std::ofstream fileOUT("users/" + currentUser + "/" + fstr1, std::ios::trunc); // open file and truncate(remove) it

                fileOUT << textInput << std::endl; // append the file
                fileOUT.close();                   // close the file

                char successful[256] = "Successful Write\n";
                send(clientsocket, successful, sizeof(successful), 0);
                memset(successful, 0, 256);
              }
            }
          }
          closedir(dir);
        }
        else
        {
          printf("error opening user directory");
        }
      } // end file modification
      /****************************************
      * ls - list files
      ****************************************/
      else if(cmd.substr(0,cmd.find(' '))=="ls"){
          std::string ls_cmd = "ls >temps.txt";
          system(ls_cmd.c_str());

          filesize = GetFileSize("temps.txt");
          send(clientsocket, &filesize, sizeof(&filesize), 0);
          filehandle = open("temps.txt", O_RDONLY);
          int bytesSent = sendfile(clientsocket, filehandle, NULL, filesize);
    ls_cmd = "rm temps.txt";
    system(ls_cmd.c_str());

      }

      /***********************************
      * receive - server sends requested file to client
      *****************************************/
      else if(cmd.substr(0,cmd.find(' ')) == "receive"){
        std::string fstr = cmd.substr(cmd.find(" ")+1);
        const char *fn = ("users/" + currentUser + "/" + fstr).c_str();

        FILE * pFile;
        long lSize;
        int status;
        char * buffer;
        size_t result;
        pFile = fopen(fn, "rw");
        if (pFile == NULL) {
          fputs("File error!!! ", stderr);
        } else {
          // obtain file size:
          fseek (pFile, 0, SEEK_END);
          lSize = ftell (pFile);
          rewind (pFile);
          long filesize = lSize;
          cout << "filesize: " << filesize << "\n";
          int sendRes = send(clientsocket, &filesize, sizeof(&filesize), 0);

          // allocate memory to contain the whole file:
          buffer = (char*) malloc (sizeof(char)*lSize);
          if (buffer == NULL) {fputs ("Memory error", stderr);}

          // copy the file into the buffer:
          result = fread (buffer, 1, lSize, pFile);
          if (result != lSize) {fputs ("Reading error", stderr);}

          char buf[1]={' '};
          int from;
          from=open(fn,O_RDONLY);
          if(from<0){
            cout << "Error opening file\n";
            return 0;
          }
          int n=1;
          int s;

          while ((n=read(from, buf, sizeof(buf)))!=0) {
            s=send(clientsocket,buf,sizeof(buf),0);
            if(s<0) {cout << "error sending\n";}
          }
          // terminate
          fclose (pFile);
          free (buffer);
        }

      }

      else if(cmd.substr(0,cmd.find(' ')) == "send"){
        int c = 0;
        char *f;
        std::cout << "received cmdLine: " << cmd.c_str() << "\n size() of cmdLine: " << cmd.length() << "\n";
        std::string fstr = cmd.substr(cmd.find(" ")+1,cmd.length());
        std::cout << "fstr from: " << cmd.substr(cmd.find(" ")+1) << " to: " << cmd.length()<< "\n";
        const char* fn = ("users/" + currentUser + "/" + fstr).c_str();

        memset(&filesize, 0, sizeof(&filesize));
        recv(clientsocket, &filesize, sizeof(&filesize), 0); // get the size of the file in bytes

        std::cout << "filesize received: " << filesize << "\n";

        std::string input_trace_filename = string(("users/" + currentUser + "/" + fstr));
        int index = input_trace_filename.find_last_of("/\\");
        std::string filename2 = input_trace_filename.substr(index+1);

        int user_file=creat(string("users/" + currentUser + "/" + filename2).c_str(),0777);

        if(user_file<0){
          cout<<"Error creating destination file\n";
        } else {

          char contents[filesize];
          //n = read(clientsocket,contents,filesize);
          //cout<<"[LOG] : File Created.\n";
          int w;
          int rec;
          int cc=0;
          while(cc!=filesize){
            rec=recv(clientsocket,contents,sizeof(contents),0);
            if(rec<0){
              cout<<"Error receiving\n";
              break;
            }
            cc += rec;
  //          cc+=1;

            w=write(user_file,contents,rec);

          }
          close(user_file);
//          send(clientsocket, &w, sizeof(&w), 0);
        }
      }
      // logout
      else if(cmd.find("logout") != std::string::npos){
        char bye[256] = "Goodbye\n";
        send(clientsocket, bye, sizeof(bye), 0);
        memset(bye, 0, 256);
        logout = 1;
      } // end logout
      else{
        char invalid_cmd[256] = "Invalid command. Try again.";
        printf("invalid command\n");
        send(clientsocket, invalid_cmd, sizeof(invalid_cmd), 0);
        memset(invalid_cmd, 0, 256);
      } // end invalid command
    } // end user
    // logout rando user
    else if(cmd.find("logout") != std::string::npos){
      char bye[256] = "Goodbye\n";
      send(clientsocket, bye, sizeof(bye), 0);
      memset(bye, 0, 256);
      logout = 1;
    }
    //invalid command rando user
    else{
      char invalid_cmd[256] = "Invalid command. Try again.";
      printf("invalid command\n");
      send(clientsocket, invalid_cmd, sizeof(invalid_cmd), 0);
      memset(invalid_cmd, 0, 256);
    }


  } while(!logout);

  printf("closing connection\n");
  close(clientsocket);


  //printf("You sent: %s\n",buffer);
  //send(clientsocket, buffer, n, 0);
  //close(clientsocket);


  // AFTER AUTHENTICATION //
  /*
  int name_found = 0;
  for (const auto & entry : fs::directory_iterator(path)){
    if((std::string)entry.path() == client_name){
      name_found = 1;
      break;
    }
  }

  if(!name_found){
    if (fs::create_directory("users/" + client_name) == 0)
      printf("ERROR creating user directory");
  }*/
  return NULL;
}
