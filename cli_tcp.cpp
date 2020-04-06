


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctime>
#include "main.h"


void  writeToServer  (int ID);
void  readFromServer (int fd);
void HELP();


bool LOGGING=false;
ofstream logfile;

class Inter {
public:
    static int K;

    static int Lets_Go(int ID) {
        struct sockaddr_in server_addr;
        struct hostent    *hostinfo;

        // Получаем информацию о сервере по его DNS имени
        // или точечной нотации IP адреса.
        hostinfo = gethostbyname(SERVER_NAME);
        if ( hostinfo==NULL ) {
            fprintf (stderr, "Unknown host %s.\n",SERVER_NAME);
            log(logfile)<<string("Unknown host ") + SERVER_NAME + ".\n";
            exit (EXIT_FAILURE);
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
        server_addr.sin_addr = *(struct in_addr*) hostinfo->h_addr;
        int nbytes = 0;
        int err;
        int sock;
        cin.ignore();

        while(1){
            sock = socket(PF_INET, SOCK_STREAM, 0);
            if ( sock<0 ) {
                perror ("Client: socket was not created");

                exit (EXIT_FAILURE);
            }

            // Устанавливаем соединение с сервером
            err = connect (sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
            if ( err<0 ) {
                perror ("Client:  connect failure");
                exit (EXIT_FAILURE);
            }
            fprintf (stdout,"Connection is ready\n");
            if(LOGGING){
            log(logfile)<<"Connection is ready"<<endl;
            }


        if (K == 2) {
            char filename[128];
            cout << " file name : \n";
            cin >> filename;
            ifstream file;
            file.open(filename, ios_base::in);
            if (!file) {
                cout << " net faila";
                return 0;
            }
            string command;
            while (getline(file, command)) {

                if(LOGGING){
                    log(logfile) << "command: " << command << endl;
                }
                if(command == "exit"){
                    cout<<"GOOD LUCK"<<"\n";
                    if(LOGGING){
                        log(logfile)<<"GOOD LUCK"<<endl;
                    }
                    logfile.close();
                }
                if(command.substr(0, 4) == "STOP"){
                    logfile.close();
                }
                if(command.substr(0, 5) == "LogOn"){
                    cout<<"LOG is On"<<"\n";
                    LOGGING=true;

                    log(logfile)<<"LOG is on"<<endl;

                }
                if(command.substr(0, 4) == "HELP"){
                    HELP();
                    log(logfile)<<"HELP"<<endl;

                }
                if(command.substr(0, 6) == "LogOff"){
                    cout<<"LOG is Off"<<"\n";
                    LOGGING=false;

                    log(logfile)<<"LOG is off"<<endl;

                }
                nbytes = write (sock, (to_string(ID) + command).data(), (to_string(ID) + command).size() + 1);
                readFromServer(sock);
                close(sock);
                sock = socket(PF_INET, SOCK_STREAM, 0);
                if ( sock<0 ) {
                    perror ("Client: socket was not created");
                    exit (EXIT_FAILURE);
                }

                // Устанавливаем соединение с сервером
                err = connect (sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
                if ( err<0 ) {
                    perror ("Client:  connect failure");
                    exit (EXIT_FAILURE);
                }
                //fprintf (stdout,"Connection is ready\n");
                cout << endl;
            }

            K = 1;
        }
        if (K == 1) {
            cout << "Comanda: \n";
            char buffer[128]; //хранит команду
            //cin.ignore();
            cin.getline(buffer, 128, '\n'); //считывает 128 символов  пока не встретит enter
             if(LOGGING){
            log(logfile) << "command: " << buffer << endl;
             }
            if(strcmp(buffer,"exit")==0){
                cout<<"GOOD LUCK"<<"\n";
                if(LOGGING){
                log(logfile)<<"GOOD LUCK"<<endl;
                }
             logfile.close();
             return 0;
            }
            if(strcmp(buffer,"STOP")==0){
             logfile.close();
            }
            if(strcmp(buffer,"HELP")==0){
                HELP();
                log(logfile)<<"HELP"<<endl;
            }
            if(string(buffer).substr(0, 5) == "LogOn" || strcmp(buffer,"LogOn")==0){
                cout<<"LOG is On"<<"\n";
                LOGGING=true;

                log(logfile)<<"LOG is on"<<endl;

            }
            if(strcmp(buffer,"LogOff")==0){
                cout<<"LOG is Off"<<"\n";
                LOGGING=false;

                log(logfile)<<"LOG is off"<<endl;

            }
            nbytes = write (sock, (to_string(ID) + buffer).data(), (to_string(ID) + buffer).size() + 1);
            cout << endl;
        }
        else {
            cout << "\neto ne 1 ili 2\n\n";
        }
         readFromServer(sock);
         close (sock);
        }
         return nbytes;
    }
};

int Inter::K = 0;






int  main (void)
{
    int id=time(0);
    id %= 1000;
    if (id < 10) {
        id *= 10;
    }
    if (id < 100) {
        id *= 10;
    }
    cout << "Id: " << id << endl;
    logfile.open(string("client_logs/client") + to_string(id) + "_log.txt", ios::app);

    cout << "Konsole ili Fail: 1 ill 2?: ";
    cin >> Inter::K;

    writeToServer(id);

    exit (EXIT_SUCCESS);
}



void  writeToServer (int ID)
{
    int   nbytes;

    nbytes = Inter::Lets_Go(ID);
    if ( nbytes<0 ) { perror("write"); exit (EXIT_FAILURE); }
}


void  readFromServer (int fd)
{
    int   nbytes;
    char  buf[BUFLEN];

    nbytes = read(fd,buf,BUFLEN);
    if ( nbytes<0 ) {
        // ошибка чтения
        perror ("read"); exit (EXIT_FAILURE);
    } else if ( nbytes==0 ) {
        // нет данных для чтения
        fprintf (stderr,"Client: no message\n");
        if(LOGGING){
        log(logfile)<<"Client: no message"<<endl;
        }

    } else {
        // ответ успешно прочитан

        fprintf (stdout,"Server's replay: %s\n",buf);
        if(LOGGING){
        log(logfile)<<string("Server's replay: ") + buf + ".\n";
        }
        if(strcmp(buf,"STOP IS OK")==0){
            logfile.close();
        }
    }
}

void HELP(){
          ifstream file;
          file.open("HELP.txt", ios_base::in);
          if (!file) {
              cout << " help will not come";
              return;
          }
          string command;
          while (getline(file, command)) {
              cout << command << endl;
          }
          file.close();


    }


