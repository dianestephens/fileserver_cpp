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
#include <sys/sendfile.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include <ctime>


using namespace std;


std::vector<std::string> f(string userInput)
{

  std::istringstream buf(userInput);
  std::istream_iterator<std::string> beg(buf), end;
  std::vector<std::string> tokens(beg, end); // done!

  return tokens;
}

void encryption(char* encryptedText,int& privateKey,int& publicKey)
{

  int modulo=64;
  privateKey = 1 + (rand() % (modulo-1));
  publicKey= modulo-privateKey;
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];
  time (&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer,sizeof(buffer),"%d-%m-%Y-%H-%M-%S",timeinfo);
  //cout << "time> " << buffer<< "\r\n";

  int i;
  for(i = 0; (i < strlen(buffer) && buffer[i] != '\0'); i++)
        encryptedText[i] = (buffer[i]+publicKey)%modulo;
 //cout << "encryptedText> " << encryptedText[2] << "\r\n";
  //char decryptedText[80];
  //for(i = 0; (i < strlen(encryptedText) && encryptedText[i] != '\0'); i++)
        //decryptedText[i] = (encryptedText[i]+privateKey)% modulo;
  //cout << "is equals? > " << strcmp( decryptedText, buffer )<< "\r\n";
  //cout << "decryptedText> " << decryptedText << "\r\n";
  //cout << "Encrypted time> " << encryptedText<< "\r\n";
  //cout << "decrypted time> " << decryptedText<< "\r\n";
  //cout << "SERVER> " << publicKey<< "\r\n";
}

void encryptedTextt(char* encryptedText,int privateKey,int publicKey)
{

  int modulo=64;
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];
  time (&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer,sizeof(buffer),"%d-%m-%Y-%H-%M-%S",timeinfo);
  //cout << "time> " << buffer<< "\r\n";

  int i;
  for(i = 0; (i < strlen(buffer) && buffer[i] != '\0'); i++)
        encryptedText[i] = (buffer[i]+publicKey)%modulo;


}


long GetFileSize(std::string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}


int main()
{
    //	Create a socket

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        return 1;
    }

    //	Create a hint structure for the server we're connecting with
    int port = 1003, filesize, filehandle, status;
    char filename[20], *myfile;
    string ipAddress = "127.0.0.1";

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    //	Connect to the server on the socket
    int connectRes = connect(sock, (sockaddr *)&hint, sizeof(hint));
    if (connectRes == -1)
    {
        return 1;
    }
    char buf[4096];

    int bytesReceived = recv(sock, buf, 4096, 0);
    if (bytesReceived == -1){
            cout << "There was an error getting response from server\r\n";
    }
    else{
            //		Display response
            string serverResponse=string(buf, bytesReceived);
            cout << "SERVER> " << serverResponse << "\r\n";
    }
    memset(buf, 0, 4096);

    string userInput;
      // control flow bools
    int logout = 0;
    int isGuest = 0;
    int isUser = 0;


    int* logo = &logout;
    int* guest = &isGuest;
    int* user = &isGuest;



 do
    {
        //	Enter lines of text
        cout << "> ";
        getline(cin, userInput);
        if(userInput.length()<80){

         if(userInput.find("create user")==0){

            std::vector<std::string> tokens=f(userInput);
            int i=0;
            for(string& s: tokens){
                if(tokens.size()>=4)
                {
                    cout << "USERNAME must not contain space\r\n";
                    cout << "USERNAME must be one word length\r\n";
                    break;
                }else if(i==2){

                    if(s.length()>20){
                        cout << "USERNAME must not be longer than 20 characters\r\n";
                        break;
                    }

                        //std::cout<< ">>>> " << userInput.c_str()<< "\r\n";
                        int sendRes = send(sock, userInput.c_str(), userInput.size(), 0);
                        if (sendRes == -1){
                            cout << "Could not send to server! Whoops!\r\n";
                            break;
                        }else{

                           memset(buf, 0, 4096);
                           int bytesReceived = recv(sock, buf, 4096, 0);
                           if (bytesReceived == -1)
                            {
                              cout << "There was an error getting response from server\r\n";
                            }
                            else{

                                int privateKey;
                                int publicKey;
                                char encryptedText[80];
                                //Display response
                                string serverResponse=string(buf, bytesReceived);
                                //cout << "mine server " << serverResponse << "\r\n";
                                //cout << "mine server " << serverResponse.c_str() << "\r\n";
                                if(strcmp(serverResponse.c_str(),"User is already created. Please try again.")==0){
                                    cout << "> " << serverResponse << "\r\n";
                                }

                                else if(strcmp(serverResponse.c_str(),"request public key")==0){
                                    //cout << "yessss!\r\n";
                                    cout << "SERVER> " << serverResponse << "\r\n";
                                    system(("mkdir -p usersClient/"+s).c_str());
                                    encryption(encryptedText,privateKey,publicKey);

                                    std::ofstream user_pub_file("usersClient/"+s+"/key.pub");
                                    std::ofstream user_enc_file("usersClient/"+s+"/enc.encrypt");
                                    std::ofstream user_pri_file("usersClient/"+s+"/key.pri");

                                    //cout << "encryptedText> " << encryptedText << "\r\n";
                                    //cout << "privateKey> " << privateKey << "\r\n";
                                    //cout << "publicKey> " << publicKey << "\r\n";

                                    int sendRes = send(sock, &publicKey, sizeof(publicKey), 0);
                                    //serverResponse=string(buf, sendRes);
                                    if (sendRes == -1){
                                        cout << "Could not send to server! Whoops!\r\n";
                                        break;
                                     }else
                                        memset(buf, 0, 4096);
                                        int bytesReceived = recv(sock, buf, 4096, 0);
                                         if (bytesReceived == -1){
                                            cout << "There was an error getting response from server\r\n";
                                        }
                                        serverResponse=string(buf, bytesReceived);

                                        if(strcmp(serverResponse.c_str(),"request encrypted")==0){
                                          cout << "SERVERR> " << serverResponse << "\r\n";
                                          //cout << "encry> " << encryptedText << "\r\n";

                                          int sendRes = send(sock, encryptedText, strlen(encryptedText), 0);
                                          serverResponse=string(buf, sendRes);

                                          if (sendRes == -1){
                                            cout << "Could not send to server! Whoops!\r\n";
                                            break;

                                          }
                                          memset(buf, 0, 4096);
                                          int bytesReceived = recv(sock, buf, 4096, 0);
                                          if (bytesReceived == -1){
                                            cout << "There was an error getting response from server\r\n";
                                        }
                                        serverResponse=string(buf, bytesReceived);
                                          if(strcmp(serverResponse.c_str(),"User creation successful. Please login.")==0){

                                              cout << "SERVERR> " << serverResponse << "\r\n";
                                              user_pub_file << publicKey;
                                              user_pri_file << privateKey;
                                              user_enc_file << encryptedText;

                                           }
                                           else{
                                               cout << "SERVERR> " << serverResponse << "\r\n";
                                           }
                                            //system(("mkdir -p usersClient/"+s).c_str());


                                     }


                                }
                                else
                                   cout << "error case!\r\n";

                            }


                          break;
                        }



                }
                 i++;
                }



            }


            else if(userInput.substr(0,userInput.find(' '))=="login"){
            std::vector<std::string> tokens=f(userInput);

            int i=0;
            for(string& s: tokens){
               if(tokens.size()>=3)
                {
                    cout << "USERNAME must not contain space\r\n";
                    cout << "USERNAME must be one word length\r\n";
                    break;
                }else if(i==1){
                //cout << ">>>> " << s<< "\r\n";

                    if(s.length()>20){
                        cout << "USERNAME must not be longer than 20 characters\r\n";
                        break;
                    }
                   int sendRes = send(sock, userInput.c_str(), userInput.size(), 0);
                   if (sendRes == -1){
                        cout << "Could not send to server! Whoops!\r\n";
                        break;
                    }else{

                        memset(buf, 0, 4096);
                        int bytesReceived = recv(sock, buf, 4096, 0);
                        if (bytesReceived == -1){
                              cout << "There was an error getting response from server\r\n";
                            }
                        else{
                        string serverResponse=string(buf, bytesReceived);

                        if(strcmp(serverResponse.c_str(),"User is not created. Please try again.")==0){
                            cout << "SERVERR> " << serverResponse << "\r\n";
                            break;
                        }
                        else if(strcmp(serverResponse.c_str(),"request encrypted data")==0){
                             cout << "SERVERR> " << serverResponse << "\r\n";
                             char oldencryptedText[80];
                             int publicKey;
                             int privateKey;
                             try{

                                std::ifstream user_enc_file("usersClient/"+s+"/enc.encrypt");
                                std::ifstream user_pub_file("usersClient/"+s+"/key.pub");
                                std::ifstream user_pri_file("usersClient/"+s+"/key.pri");
                                user_enc_file >> oldencryptedText;
                                user_pub_file >> publicKey;
                                user_pri_file >> privateKey;

                             }catch(...){
                               cout << "errorThe user's old encrypted text  is not found in the filesystem !\n";
                               break;
                            }
                           int sendRes = send(sock, oldencryptedText, strlen(oldencryptedText), 0);
                           if (sendRes == -1){
                            cout << "Could not send to server! Whoops!\r\n";
                            break;
                           }else{
                              memset(buf, 0, 4096);
                              int bytesReceived = recv(sock, buf, 4096, 0);
                              if (bytesReceived == -1){
                                cout << "There was an error getting response from server\r\n";
                              }else{
                                string serverResponse=string(buf, bytesReceived);
                                if(strcmp(serverResponse.c_str(),"request new encrypted data")==0){
                                   cout << "SERVERR> " << serverResponse << "\r\n";
                                   int i;
                                   char encryptedText[80];
                                   encryptedTextt(encryptedText,privateKey,publicKey);
                                   int sendRes = send(sock, encryptedText, strlen(encryptedText), 0);
                                   if (sendRes == -1){
                                      cout << "Could not send to server! Whoops!\r\n";
                                      break;
                                   }else{
                                       memset(buf, 0, 4096);
                                       int bytesReceived = recv(sock, buf, 4096, 0);
                                       if (bytesReceived == -1){
                                           cout << "There was an error getting response from server\r\n";
                                        }else{
                                           string serverResponse=string(buf, bytesReceived);
                                          if(strcmp(serverResponse.c_str(),"Successful authentication")==0){
                                            cout << "SERVERR> " << serverResponse << "\r\n";


                                            try{

                                            std::ofstream user_enc_file("usersClient/"+s+"/enc.encrypt");
                                            user_enc_file << encryptedText;
                                            }catch(...){
                                               cout << "Error Cannot open the old encrypted file !\n";
                                               break;
                                           }


                                          break;
                                          }else{
                                              cout << "SERVERR> " << serverResponse << "\r\n";
                                              break;
                                          }


                                       }

                                }
                                }
                                else if(strcmp(serverResponse.c_str(),"Wrong authentication")==0){
                                    cout << "SERVERR> " << serverResponse << "\r\n";
                                }


                              }
                              }


                           }

                }
                }



            }
             i++;
               }
               /****************************************
               * ls - list files
               ****************************************/
              } else if(userInput.substr(0,userInput.find(' '))=="ls"){
                   std::vector<std::string> tokens=f(userInput);
                   std::string fstr = "ls";
                   const char *fn = fstr.c_str();

                   int sendRes = send(sock, userInput.c_str(), userInput.size(), 0);

                   if (sendRes == -1){
                     cout << "Could not send input to server! Whoops!\r\n";
                     break;
                   } else {
                     //  memset(&filesize, 0, sizeof(&filesize));
                     int bytesReceived = recv(sock, &filesize, sizeof(&filesize), 0);
                     if (bytesReceived == -1){
                       cout << "There was an error getting response from server\r\n";
                     }
                     else {

                       myfile = (char *)malloc(filesize);
                       memset(myfile, 0, filesize);
                       bytesReceived = recv(sock, myfile, filesize, 0);

                       if (bytesReceived == -1){
                         cout << "There was an error receiving ls from server\r\n";
                       } else {
                         filehandle = creat("temps.txt", O_WRONLY);
                         int bytesWritten =  write(filehandle, myfile, filesize);

                         close(filehandle);
                         cout << "Directory listing: \n";
                         //  system("cat temp.txt");
                         std::string cat_temps_cmd = "cat temps.txt";
                         system(cat_temps_cmd.c_str());
                         cat_temps_cmd = "rm temps.txt";
                         system(cat_temps_cmd.c_str());

                       }

                     }
                   }
                 } // end ls

                else if(userInput.substr(0,userInput.find(' '))=="receive"){
                    int c = 0;
                    char *f;

                    std::string fstr = userInput.substr(userInput.find(" ")+1);
                    const char *fn = fstr.c_str();
                    int sendRes = send(sock, userInput.c_str(), userInput.size(), 0);
                    //cout << "sent userInput to server: " << userInput.c_str() << "\n size() of cmdLine: " << userInput.size() << "\n";
                    if (sendRes == -1){
                      cout << "Could not send get request server! Whoops!\r\n";
                      break;
                    } else {
                      memset(&filesize, 0, sizeof(&filesize));
                      int bytesReceived = recv(sock, &filesize, sizeof(&filesize), 0);
                      if (bytesReceived == -1){
                        cout << "No such file found; filesize: " << filesize << "\r\n";
                      } else {
                        cout << "received filesize: " << filesize << " about to open: " << filename << "\n";
                        filehandle = creat(fn, 0777);
                        if (filehandle == -1) {
                          cout << "Cannot create file here\n";
                          break;
                        }
                        char contents[filesize];
                        int w;
                        int rec;
                        int cc = 0;
                        while(cc != filesize) {
                          cout << "waiting to receive file contents; cc: " << cc << " filesize: " << filesize << "\n";
                          rec=recv(sock,contents,sizeof(contents),0);
                          if(rec<0){
                            cout <<"Error receiving file\n";
                            break;
                          }
                          cc += rec;
                          //  cc+=1;

                          w=write(filehandle, contents, rec);
                          //cout << "received and wrote " << w << " bytes to file\n";
                        }

                        close(filehandle);
              //          send(sock, &w, sizeof(&w), 0);

                      }
                    } // else good send
                  } // end receive


                  else if(userInput.substr(0,userInput.find(' '))=="send"){

		                std::string fstr = userInput.substr(userInput.find(" ")+1);
                    const char *fn = fstr.c_str();
                    cout << "fstr.c_str() " << fstr.c_str() << "\n";
                    int sendRes = send(sock, userInput.c_str(), userInput.size(), 0);
                    cout << "sent userInput to server: " << userInput.c_str() << "\n size() of cmdLine: " << userInput.size() << "\n";

                    if (sendRes == -1){
                         cout << "Could not send to server! Whoops!\r\n";

                    } else {
                            FILE * pFile;
                            long lSize;
                            char * buffer;
                            size_t result;

                            pFile = fopen(fn, "rb");
                            if (pFile==NULL) {fputs ("File error!!! ",stderr); break;}

                            // obtain file size:
                            fseek (pFile , 0 , SEEK_END);
                            lSize = ftell (pFile);
                            rewind (pFile);
                            long filesize = lSize;
                            cout << "GetFileSize returned: " << filesize <<  "szof:" << sizeof(&filesize)<<"\n";
                            sendRes = send(sock, &filesize, sizeof(&filesize), 0);


                            // allocate memory to contain the whole file:
                            buffer = (char*) malloc (sizeof(char)*lSize);
                            if (buffer == NULL) {fputs ("Memory error",stderr);}

                            // copy the file into the buffer:
                            result = fread (buffer,1,lSize,pFile);
                            if (result != lSize) {fputs ("Reading error",stderr);}

                            //send(sock, buffer, lSize, 0);
                            char buf[1]={' '};
                            int from;
                            from=open(fn,O_RDONLY);
                            if(from<0){
	                           cout<<"Error opening file\n";
                               return 0;
                            }
                            int n=1;
                            int s;

                            while((n=read(from,buf,sizeof(buf)))!=0){
                              s=send(sock,buf,sizeof(buf),0);
                              //s=write(clientsocket,buf,n);
                              if(s<0){cout<<"error sending\n";return 0;}
                             }

                            /* the whole file is now loaded in the memory buffer. */

                            // terminate
                            fclose (pFile);
                            free (buffer);

                      }
                  }
                  else if(userInput.substr(0,userInput.find(' '))=="quit"){
                    break; // exit while
                  }




// DS - don't know about the rest of this

        else{
        //	Send to server
        int sendRes = send(sock, userInput.c_str(), userInput.size() + 1, 0);
        if (sendRes == -1)
        {
           cout << "Could not send to server! Whoops!\r\n";
           continue;
        }

        		//Wait for response
        memset(buf, 0, 4096);
        int bytesReceived = recv(sock, buf, 4096, 0);
        if (bytesReceived == -1)
        {
            cout << "There was an error getting response from server\r\n";
        }
        else
        {
            //		Display response
            string serverResponse=string(buf, bytesReceived);
            if(strcmp(serverResponse.c_str(),"Welcome Guest User")==0){                                          cout << "SERVERR> " << serverResponse << "\r\n";

              }
            else{
               cout << "SERVER> " << serverResponse << "\r\n";
            }
        }
      }

     }else{
          cout << "Client> Too long argument...Please try again\r\n";
        }
    } while (true);

    //	Close the socket
    close(sock);

    return 0;
}
