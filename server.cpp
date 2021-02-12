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


void * connection_thread(void* p_client_socket);

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

  port = 1000; // hard code port #
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


    // wait for client input
    int n = read(clientsocket, buffer, 256);
    if (n < 0)
      perror("ERROR reading from socket");

    std::string cmd(buffer);

    if(cmd.find("connect guest") != std::string::npos || isGuest == 1){
      if(isGuest == 0){
        printf("in here\n");
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
          std::ofstream user_pub_file("users/"+create_user+"/.pub");
          std::ofstream user_enc_file("users/"+create_user+"/.encrypt");

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
          std::ifstream user_encrypt_file("users/"+currentUser+"/.encrypt");
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
                ofs.open("users/"+currentUser+"/.encrypt", std::ofstream::out | std::ofstream::trunc);
                ofs.close();

                // add new encrypt data to file
                std::ofstream user_new_enc_file("users/"+currentUser+"/.encrypt");
                user_new_enc_file << new_encrypted;
                memset(new_encrypted, 0, 256);

                char auth_success[256] = "Successful authentication";
                send(clientsocket, auth_success, sizeof(auth_success), 0);
                memset(auth_success, 0, 256);

                isUser = 1;
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
    //user is logged in
    else if(isUser == 1){
      if(cmd.find("logout") != std::string::npos){
        char bye[256] = "Goodbye\n";
        send(clientsocket, bye, sizeof(bye), 0);
        memset(bye, 0, 256);
        logout = 1;
      }
    }
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
