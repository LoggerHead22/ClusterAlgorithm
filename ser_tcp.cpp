

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <fstream>
#include "main.h"
using namespace std;


// Определимся с номером порта и другими константами.
#define PORT    5555
#define BUFLEN  512

// Две вспомогательные функции для чтения/записи (см. ниже)
int  readFromClient (int fd, char *buf,Contoller &c);
void  writeToClient (int fd, char *buf);
void MemberFunc(int x,int y,Contoller &c, char* buf);
string Com(char* buf);

ofstream logfile("server_log.txt");
int adminId = -1;
bool LOGGING = false;


int  main (void)
{
    int     i, err, opt=1;
    int     sock, new_sock;
    fd_set  active_set, read_set;
    struct  sockaddr_in  addr;
    struct  sockaddr_in  client;
    char    buf[BUFLEN];
    socklen_t  size;
    Field ff;
    Contoller cc(&ff);
    // Создаем TCP сокет для приема запросов на соединение
    sock = socket (PF_INET, SOCK_STREAM, 0);
    if ( sock<0 ) {
        perror ("Server: cannot create socket");
        log(logfile)<<"Server: cannot create socket"<<"\n";
        exit (EXIT_FAILURE);
    }
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));

    // Заполняем адресную структуру и
    // связываем сокет с любым адресом
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    err = bind(sock,(struct sockaddr*)&addr,sizeof(addr));
    if ( err<0 ) {
        perror ("Server: cannot bind socket");
         if(LOGGING){
        log(logfile)<<"Server: cannot bind socket"<<"\n";
         }
        exit (EXIT_FAILURE);
    }

    // Создаем очередь на 3 входящих запроса соединения
    err = listen(sock,3);
    if ( err<0 ) {
        perror ("Server: listen queue failure");
         if(LOGGING){
        log(logfile)<<"Server: listen queue failure"<<"\n";
         }
        exit(EXIT_FAILURE);
    }

    // Подготавливаем множества дескрипторов каналов ввода-вывода.
    // Для простоты не вычисляем максимальное значение дескриптора,
    // а далее будем проверять все дескрипторы вплоть до максимально
    // возможного значения FD_SETSIZE.
    FD_ZERO(&active_set);
    FD_SET(sock, &active_set);

    // Основной бесконечный цикл проверки состояния сокетов
    while (1) {
        // Проверим, не появились ли данные в каком-либо сокете.
        // В нашем варианте ждем до фактического появления данных.
        read_set = active_set;
        if ( select(FD_SETSIZE,&read_set,NULL,NULL,NULL)<0 ) {
            perror("Server: select  failure");
             if(LOGGING){
            log(logfile)<<"Server: select  failure"<<"\n";
             }
            exit (EXIT_FAILURE);
        }

        // Данные появились. Проверим в каком сокете.
        for (i=0; i<FD_SETSIZE; i++) {
            if ( FD_ISSET(i,&read_set) ) {
                if ( i==sock ) {
                    // пришел запрос на новое соединение
                    size = sizeof(client);
                    new_sock = accept(sock,(struct sockaddr*)&client,&size);
                    if ( new_sock<0 ) {
                        perror("accept");
                        exit (EXIT_FAILURE);
                    }
                    fprintf (stdout, "\nServer: connect from host %s, port %hu.\n",
                            inet_ntoa(client.sin_addr),
                            ntohs(client.sin_port));
                     if(LOGGING){
                    log(logfile)<<"Server: connect from host"<< inet_ntoa(client.sin_addr)<<"port"<<ntohs(client.sin_port)<<"\n";
                     }
                    FD_SET(new_sock, &active_set);
                } else {
                    // пришли данные в уже существующем соединени
                    err = readFromClient(i,buf,  cc);
                    if ( err<0 ) {
                        // ошибка или конец данных
                        close (i);
                        FD_CLR(i,&active_set);
                    } else {
                        // данные прочитаны нормально
                        writeToClient(i,buf);
                        close (i);
                        FD_CLR (i,&active_set);
                        // а если это команда закончить работу?
                        if ( !strcmp(buf,"STOP IS OK") ) {
                            close(sock);
                            return 0;
                        }
                    }
                }
            }
        }
    }
}



int  readFromClient (int fd, char *buf,Contoller &c)
{
    int  nbytes;
    nbytes = read(fd,buf,BUFLEN);

    string s = buf;
    int ID = stoi(s.substr(0, 3));
    s = s.substr(3);
    strcpy(buf, s.data());

    if ( nbytes<0 ) {
        // ошибка чтения
        perror ("Server: read failure");
        return -1;
    } else if ( nbytes==0 ) {
        // больше нет данных
        return -1;
    } else {
        // есть данные
        fprintf(stdout,"Server got message: %s\n",buf);
         if(LOGGING){
        log(logfile)<<"Server got message: "<<buf<<"\n";
         }
         string com=string(buf);
        if((adminId == ID) && strcmp(buf, "admin off") == 0){
            adminId = -1;
            strcpy(buf,"bye admin");
            return 0;
        }
        if(strcmp(buf, "admin on") == 0){
            if (adminId==-1) {
                adminId = ID;
                cout << "client " << ID << " is new admin\n";
                 if(LOGGING){
                log(logfile) << "client " << ID << " is new admin\n";
                 }
                strcpy(buf,"You are new admin");
            } else {
                strcpy(buf, (string("Current admin is ") +to_string(ID)).c_str());
            }
            return 0;
        }

        if(strcmp(com.substr(0,8).c_str(), "SerLogOn") == 0){
             LOGGING=true;
             strcpy(buf,"Server Log On");
             return 0;
        }
        if(strcmp(buf, "SerLogOFF") == 0){
            strcpy(buf,"Server Log OFF");
             LOGGING=false;
             return 0;
        }
        if(strcmp(buf, "HELP") == 0){
            strcpy(buf,"OK");
             return 0;
        }
        if(strcmp(buf, "exit") == 0){
            strcpy(buf,"OK");
             return 0;
        }
        if(string(buf).substr(0, 5) == "LogOn" || string(buf) == "LogOn" || strcmp(buf, "LogOff") == 0 || strcmp(buf, "exit") == 0){
            strcpy(buf,"Ok");
            return 0;
        }
        float x,y;
   //     LOG(__LINE__);
        if(parse(buf).str_("MemberFunc").float_(x).float_(y).success()){
            MemberFunc(x,y,c,buf);
            return 0;
        }
        if(parse(buf).str_("ForelFromFile").float_(x).success()){
            ClientID=ID;
//            return 0;
        }

   //     LOG(__LINE__);
        bool usedAdminCmd = false;     // команда предназначена для админа
        bool isAdmin = (ID == adminId) || (adminId == -1);  //является ои клиент админом
        bool flag=c.handleCommand(string(buf), isAdmin, usedAdminCmd);
        if ((!isAdmin && usedAdminCmd)) { // если не админ и использовал админскую команду
            strcpy(buf,"You are not admin");
             if(LOGGING){
            log(logfile)<<"client " << ID << " is not admin";
             }
            return 0;
        }
        if(strcmp(buf,"STOP")==0){
            PostLog::save_all();
              strcpy(buf,"STOP IS OK");
        }
        else {
            if(flag==true){
                strcpy(buf,"OK");
            }else{
                strcpy(buf,"FALL");
            }
        }
        return 0;
    }
}


void MemberFunc(int x,int y,Contoller &c, char* buf){
    Point P(x,y);
    float min;
    int NumberOfClaster = -1;
    if(!c.clasterizing){
        strcpy(buf,"Ne clasterizovano");
        return;
    }
    if(!c.ForelFlag){
        strcpy(buf,"Ne forelizovano");
        return;
    }
    c.NET();
    bool b=false;
    FormalPoint MyTur;
    c.handleCommand("MakeDataFile",true,b);
    min=sqrt((P.x-c.ff->Cover[0].FormalElem.x)*(P.x-c.ff->Cover[0].FormalElem.x)+(P.y-c.ff->Cover[0].FormalElem.y)*(P.y-c.ff->Cover[0].FormalElem.y));
    MyTur=c.ff->Cover[0];
    for(FormalPoint Turtel:c.ff->Cover){

        if(sqrt((P.x-Turtel.FormalElem.x)*(P.x-Turtel.FormalElem.x)+(P.y-Turtel.FormalElem.y)*(P.y-Turtel.FormalElem.y)) < min){
            LOG(Turtel.FormalElem);
            LOG(min);
            min=sqrt((P.x-Turtel.FormalElem.x)*(P.x-Turtel.FormalElem.x)+(P.y-Turtel.FormalElem.y)*(P.y-Turtel.FormalElem.y));
            MyTur=Turtel;
        }
    }
    if(min > (2*c.ff->Cover[0].Rad) ){
      strcpy(buf,"Tochka ne prinadlezhit");
      return;
    }
    c.ff->FindSector(P);
    LOG((c.data_file.lineCount()));
    for (int i: MyTur.Slaves) {
        LOG(i);
        Point p = c.data_file.readLine(i);
        if (sqrt((P.x-p.x)*(P.x-p.x)+(P.y-p.y)*(P.y-p.y))<min) {
            min=sqrt((P.x-p.x)*(P.x-p.x)+(P.y-p.y)*(P.y-p.y));
            LOG(p);
            LOG(p.CloudNumber);
            NumberOfClaster=p.CloudNumber;
        }
    }
    strcpy(buf,to_string(NumberOfClaster).c_str());
}

void  writeToClient (int fd, char *buf)
{
    int  nbytes;
    nbytes = write(fd,buf,strlen(buf)+1);
    fprintf(stdout,"Write back: %s\nnbytes=%d\n",buf,nbytes);
    if(LOGGING){
    log(logfile)<<"Write back: "<<buf<<"nbytes="<<nbytes<<"\n";
    }
    if ( nbytes<0 ) {
        perror ("Server: write failure");
    }
}

