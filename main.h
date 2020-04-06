#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <ctime>
#include <cassert>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <chrono>
#include <map>
#include <sstream>
#include <algorithm>
using namespace std;

#define LOG(x) cout << (#x) << " = "<< x << endl;
#define  BUFLEN          512
#define  SERVER_PORT     5555
#define  SERVER_NAME    "127.0.0.1"
#define UNUSED(x) (void)x

const float eps = 0.0001;
const float PI = 3.14159265;
const float DEG_TO_RAD = 1 / (180 / PI);

int ClientID = -1;

const string currentDateTime() {
    auto ms = chrono::high_resolution_clock::now().time_since_epoch().count() % 1000000000;
    string ms_as_str = (string(":") + to_string(ms));
    while (ms_as_str.size() < 10) ms_as_str.insert(ms_as_str.begin() + 1, '0');

    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    strcat(buf, ms_as_str.c_str());
    return buf;
}

std::ofstream& log(ofstream& logfile) {
    logfile << currentDateTime() << ": ";
    return logfile;
}

class InitLogFileCloser {
public:
    ofstream& file;
    
    InitLogFileCloser(ofstream& file_):
        file(file_) {}
    
    ~InitLogFileCloser() {
        file.close();
    }
};

vector<string> readfile(string path) {
    ifstream file(path);
    vector<string> ret;
    string line;
    while (getline(file, line)) {
        ret.push_back(line);
    }
    file.close();
    return ret;
}

class PostLog {
private:
    static map<string, stringstream> data;

public:
    ~PostLog() {
        save_all();
    }

    static stringstream& write(string filename) {
        return data[filename];
    }

    static void show() {
        for (auto& it : data) {
            cout << it.first << ": " << it.second.str() << endl;
        }
    }

    static bool cmp(string a, string b) {
        return a.substr(0, a.find(": ")) < b.substr(0, b.find(": "));
    }

    static void save(string filename) {
        ofstream file(filename, ios::app);
        if (file) {
            file << data[filename].str();
            file.close();

            vector<string> lines = readfile(filename);
            sort(lines.begin(), lines.end(), cmp);
            ofstream file(filename);
            for (auto it : lines) {
                file << it << endl;
            }
            file.close();

        } else {
            cout << "Can not open file " << filename << endl;
            exit(-1);
        }
    }

    static void save_all() {
        for (auto& it : data) {
            save(it.first);
        }
    }
};

map<string, stringstream> PostLog::data;

template <typename T>
class FileVector {//Delaet fail vectorom dannih
private:
    std::fstream m_handle;
    int m_lineCount{0};

public:
    FileVector() {}

    FileVector(const std::string& filename)
    {
        open(filename);
    }

    void open(std::string filename) {
        ofstream file(filename);
        file.close();
        m_handle.open(filename, std::ios::in | std::ios::out | std::ios::binary/* | std::ios::app*/);
        if (!m_handle.is_open()) {
            assert(false && "can\'t open file");
        }
        m_lineCount = 0;
        // TODO: set line count on reopening
    }

    ~FileVector() {
        close();
    }

    int lineCount() {
        return m_lineCount;
    }

    void pushLine(const T& data) {
        m_handle.write(const_cast<char*>(reinterpret_cast<const char*>(&data)) , sizeof(T));
        m_lineCount++;
    }

    void writeLine(const T& data, int line) {
        if (!(0 <= line && line < m_lineCount)) {
            std::cout << "writeLine: 0 <= " << line << " < m_lineCount\n - NEPRAVILNO";
            assert(false);
        }

        m_handle.seekp(line * sizeof(T));
        m_handle.write(const_cast<char*>(reinterpret_cast<const char*>(&data)) , sizeof(T));
    }

    T readLine(int line) {
        if (!(0 <= line && line < m_lineCount)) {
            std::cout << "readLine: 0 <= " << line << " < m_lineCount(" << m_lineCount << ") - NEPRAVILNO\n";
            assert(false);
        }

        T ret;
        m_handle.seekg(line * sizeof(T));
        m_handle.read((char*) &ret, sizeof(T));
        return ret;
    }

    void close() {
       m_handle.close();
       m_lineCount = 0;
    }
};

class Point {
public:
    float x{0}, y{0}, z{0};//x,y,z - koordinaty tochek
    int col{-1},column{-1},string{-1};//col =cvet column , string - nomer stroki i stolbca v setki
    int TotalNumber{-1};//obshchij nomer v massive tochek
    int CloudNumber{-1};

    Point() {}
    Point(float x_, float y_) {
        x = x_;
        y = y_;
    }

    Point operator +(const Point& other) const  {//peregruzka operatora + dlya klassa tochka
        return Point(x + other.x, y + other.y);
    }

    bool operator ==(const Point  &other) const {//peregruzka operatora == dlya klassa tochka
        return fabs(x - other.x)<eps && fabs(y - other.y)<eps;
    }

    Point& operator +=(const Point& other) {////peregruzka operatora += dlya klassa tochka
        return *this = *this + other;
    }

    void rotatePoint(float ang) {// povorot na ugl ang
        ang *= DEG_TO_RAD;
        Point ret = Point(0, 0);
        ret.x = x * cos(ang) - y * sin(ang);
        ret.y = x * sin(ang) + y * cos(ang);
        *this = ret;
    }
};


class FormalPoint{
public:
    Point FormalElem;
    vector<int> Slaves;
    float Rad;
    FormalPoint(){}
    FormalPoint(const Point& FormalElem_,const vector<int>& Slaves_,float Rad_):
        FormalElem(FormalElem_),
        Slaves(Slaves_),
        Rad(Rad_){}

};

ostream& operator<<(ostream& stream, Point p) {
    stream << "{" << p.x << ", " << p.y << "}";
    return stream;
}

class Plane {//klass dlya proektirovaniya tochek na ploskost' v R3
public:
    float A, B, C, D;//koehfficienty v uravnenie Ax+By+Cz+D=0
    Plane(float A_, float B_, float C_, float D_) {
        A = A_;
        B = B_;
        C = C_;
        D = D_;
    }

    float ZValue(const Point& P) const  {//vychislenie z koordinaty dlya dannoj ploskosti
        //	cout <<"A" << A << endl;
        return (-D - A * P.x - B * P.y) / C;
    }

};

class Cloud {
public:
    static int last_cloud_number;

    Point Centr;//tochka cenra
    float d1{0}, d2{0};//koehfficienty rastyazhenie po osyam
    vector<Point>* field_allpoint{nullptr};
    vector<int> point_index;
    unsigned this_cloud_number=0;


    Cloud(vector<Point>* field_allpoint_):
        field_allpoint(field_allpoint_) ,
        this_cloud_number(last_cloud_number++){}

    Cloud(Point Centr_, float d1_, float d2_, vector<Point>* field_allpoint_):
        Centr(Centr_),
        d1(d1_),
        d2(d2_),
        field_allpoint(field_allpoint_),
        this_cloud_number(last_cloud_number++)
    {}

    void Create(int N) {//funkciya dlya sozdaniya oblaka iz N tochek
        unsigned allpoint_size = field_allpoint->size();

        for (int i = 0; i < N; i++) {
            Point p = genPoint();
            p.TotalNumber = allpoint_size + i;
            p.CloudNumber = this_cloud_number;
            field_allpoint->push_back(p);
            point_index.push_back(allpoint_size + i);
        }
    }

    void Add(int id){
        point_index.push_back(id);
        field_allpoint->at(id).CloudNumber=this_cloud_number;
    }
    Point genPoint() {
        float x = 0, y = 0;
        for (int i = 0; i<1000; i++) {
            x += Rand(-10, 10);
            y += Rand(-10, 10);
        }
        x /= 1000;
        y /= 1000;
        x = x * d1 + Centr.x;
        y = y * d2 + Centr.y;
        return Point(x, y);
    }

    float Rand(float min, float max) {
        return min + (max - min) * (float(rand()) / RAND_MAX);
    }

    vector<Point*> fpoints() {
        vector<Point*> ret;
        for (int i : point_index) {
            ret.push_back(&((*field_allpoint)[i]));
        }
        return ret;
    }

    void Noize() {//dobavlenie shuma po z koordinate
        for (Point* p : fpoints()) {
            p->z += Rand(-0.1, 0.1);
        }
    }

    void Projection(Plane Pl) {//proekci tochek na ploskost' Pl
        //	cout << "in rojection" << endl;
        for (Point* p : fpoints()) {
            //	cout << i << endl;
            p->z = Pl.ZValue(*p);
            //	LOG(PC[i].z);
        }
    }

    void sdvig(float x, float y) {//sdvig na vektor x,y
        Centr += Point(x, y);
        for (Point* p : fpoints()) {
            *p += Point(x, y);
        }
        cout << " sdvig is OK \n";
    }

    void deform(float k1, float k2) {
        for (Point* p : fpoints()) {
            p->x = Centr.x + (p->x - Centr.x)*k1;
            p->y = Centr.y + (p->y - Centr.y)*k2;
        }
        cout << " deform is OK \n";
    }

    void rotateCloud( float ang) {//rastyazhenie na koehfficienty d1,d2
        for (Point* p : fpoints()) {
            p->rotatePoint(ang);
        }
        Centr.rotatePoint(ang);
        cout << " rotate is OK \n";
    }

    void rotate_ct(float ang) {//povernut' olbako na ang
        for (Point* p : fpoints()) {
            p->x = p->x - Centr.x;
            p->y = p->y - Centr.y;
            p->rotatePoint(ang);
            p->x = p->x + Centr.x;
            p->y = p->y + Centr.y;
        }
        cout << "rotate_ct is  OK \n";
    }

    Point CT() {
        float x=0, y=0;
        for (Point* p : fpoints()) {
            x += p->x;
            y += p->y;
        }
        x /= point_index.size();
        y /= point_index.size();
        return Point(x, y);
    }


};

int Cloud::last_cloud_number = 0;

class Field {
public:
    vector<Point> allpoint;
    vector<Cloud> CS;
    vector<Plane> Planes;
    vector<FormalPoint> Cover;
    Point Net;//tochka hranyashchaya kak x,y dlinu i shirinu kletki setki. I column string - chislo stolbcov i strok v setk
    float left=0,right=0,up=0,down=0;

    Field() {
        //      Cloud a(Point(0, 0), 1, 1, &allpoint);
        //	CS.push_back(a);
        Net=Point(1,1);
    }

    void FieldFromCover() {
        Field ret;
        for (auto it : Cover) {
            ret.allpoint.push_back(it.FormalElem);
        }
        ret.Volnovoi(0.85);
    }
    void UpDateNumber(){
        for(Cloud C:CS){
            for(int i:C.point_index){
                allpoint[i].CloudNumber=C.this_cloud_number;
            }
        }
    }
    Point Field_CT() {  //centr tiazhesti polia
        Point p(0,0);
        for (unsigned int i = 0; i < allpoint.size(); i++) {
            p.x += allpoint[i].x;
            p.y += allpoint[i].y;
        }
        p.x /= allpoint.size();
        p.y /= allpoint.size();
        return p;
    }

    void CreateCloud(int N, float d1, float d2, float x, float y) {
        Cloud c(Point(x, y), d1, d2, &allpoint);
        c.Create(N);
        CS.push_back(c);
        cout << "Cloud Number: " << CS.size() - 1 << "\n";
    }

    void save(string path) {//save v path id cloud
        ofstream file(path);
        for (unsigned i = 0; i < CS.size(); i++) {
            for (Point* p : CS[i].fpoints()) {
                file << p->x << " " << p->y << " " << p->CloudNumber << endl;
            }
            file << CS[i].Centr.x << " " << CS[i].Centr.y << " " << Cloud::last_cloud_number << endl;
        }
        file.close();
        cout << " save is OK \n";

    }

    void saveCol(string path) {//save v path id cloud
        ofstream file(path);
        for (unsigned i = 0; i < CS.size(); i++) {
            for (Point* p : CS[i].fpoints()) {
                file << p->x << " " << p->y << " " << p->col << endl;
            }
        }
        file.close();
        cout << " saveCol is OK \n";

    }

    void save3Pl(int id,const Cloud& C) {//save id cloud C after progection on some plane
        ofstream  file((string("cloud_") + to_string(id) + string("on3d.txt")));
        for (Point* p:C.fpoints()) {
            file<<p->x<<" "<<p->y<<" "<<p->z<<endl;
        }
        cout << " OK \n";
        file.close();
    }

    void loadFromFile(string path) {
        ifstream file(path);

        float x, y;
        int id;

        unsigned allpoint_size = allpoint.size();
        unsigned this_cloud_number = Cloud::last_cloud_number++;

        int i = 0;
        while (file >> x >> y >> id) {
            Point p = {x, y};
            p.TotalNumber = allpoint_size + i++;
            p.CloudNumber = this_cloud_number;
            allpoint.push_back(p);
        }

        cout << "Load is  OK \n";
    }

    float Lenght(int i, int k) {  //dlina ot i do k tochi massiva vseh toshek (massiv,nomer i,nomer k)
        float d;

        float xx = allpoint[i].x - allpoint[k].x;
        float yy =  allpoint[i].y - allpoint[k].y;
        d = sqrt(xx * xx + yy * yy);
        return d;
    }
    //######################################################################################################################################################
    //NET ON FIELD
    //######################################################################################################################################################
    void NetOnField(){//sozdanie setki na pole
        left=allpoint[0].x,right=allpoint[0].x,up=allpoint[0].y,down=allpoint[0].y;
        float width,height;
        for (unsigned int i = 0; i < allpoint.size(); i++) {
            if(allpoint[i].x < left)  {left =allpoint[i].x;};
            if(allpoint[i].x > right) {right=allpoint[i].x;};
            if(allpoint[i].y < down)  {down =allpoint[i].y;};
            if(allpoint[i].y > up)   { up=allpoint[i].y;};
        }
        width=fabs(left-right);
        height=fabs(up-down);
        Net.y=height/20;
        Net.x=width/20;
        Net.column=20;
        Net.string=20;


        for (unsigned int i = 0; i < allpoint.size(); i++) {
            allpoint[i].column=	(int)((allpoint[i].x-left)/Net.x);
            allpoint[i].string=	(int)((up -allpoint[i].y)/Net.y);
        }


        FILE* file = fopen("NetOnField.txt", "w");

        for(int i = 0;i<Net.column+1;i++){
            for(int l=0;l<Net.string+1;l++){
                fprintf(file, "%f %f \n", left + i*Net.x, up - l*Net.y);

            }
            fprintf(file, "\n");
        }
        fprintf(file, "\n");
        for(int i = 0;i<Net.column+1;i++){
            for(int l=0;l<Net.string+1;l++){
                fprintf(file, "%f %f \n", left + l*Net.x, up - i*Net.y);

            }
            fprintf(file, "\n");
        }

        cout << " OK \n";
        fclose(file);


    }

    float MinInNet(){
        float min = Net.x<Net.y ? Net.x:Net.y;
        return min;
    }

    void FindSector(Point &P){
        int kx,ky;
        if(P.x<left || P.y>up || P.x>right || P.y< down){
            cout<<"Point outside the net"<<endl;
        }
        else{
            kx=	(int)((P.x -left)/Net.x);
            ky=	(int)((up  -P.y) /Net.y);
        }
        P.column=kx;
        P.string=ky;
    }
    //######################################################################################################################################################
    //KNEARESTNEIGHBOOR
    //######################################################################################################################################################
    bool KNN(int x,int y,int k){
        FileVector<Point> file("DataFile.txt");
        NetOnField();
        MakeDataFile(file);
        Point P(x,y);
        FindSector(P);
        if(P.column==-1 || P.string==-1){
            return false;
        }
        vector<int> Neigh;
        int border=0;
        while(Neigh.size() < unsigned (k) ){
            if(border == 10){
                return false;
            }
            for (int i = 0; i < file.lineCount(); i++) {
               Point  AP = file.readLine(i);
               if(AP.col == -1){
                    for(int l=P.column-border;l<P.column+border+1;l++){
                        if(AP.column==l){
                            for(int k=P.string-border;k<P.string+1+border;k++){
                                if(AP.string==k){
                                        Neigh.push_back(AP.TotalNumber);
                                        AP.col=-2;
                                        file.writeLine(AP, i);
                                }
                            }
                        }
                }
            }

        }
        border++;
        }
      SortVec(Neigh,P);
      Neigh.resize(k);

      for( int i = 0;i< k;i++){
          LOG(allpoint[Neigh[i]]);

      }
      return true;
    }
    void  SortVec(vector<int> &Neigh,Point P){
        int swap;
        float len1,len2;
        for (unsigned int i = 0;i< Neigh.size() - 1;i++){
            len1=sqrt((allpoint[Neigh[i]].x- P.x)*(allpoint[Neigh[i]].x- P.x) + (allpoint[Neigh[i]].y -  P.y)*(allpoint[Neigh[i]].y- P.y));
            for(unsigned int k = i + 1;k<Neigh.size();k++){
                len2=sqrt((allpoint[Neigh[k]].x- P.x)*(allpoint[Neigh[k]].x- P.x) + (allpoint[Neigh[k]].y -  P.y)*(allpoint[Neigh[k]].y- P.y));
                if( len1 > len2 ){
                swap = Neigh[i];
                Neigh[i] = Neigh[k];
                Neigh[k] = swap;
                }
            }
        }
    }
    //######################################################################################################################################################
    //FOREL FROM FILE
    //######################################################################################################################################################

    void ForelFromFile(float Rad){//sozdaet massiv formal'nyh tochek s radiusom Rad
        FileVector<Point> file("ForelDataFile.txt");
        NetOnField();
        MakeDataFile(file);
        int free=file.lineCount();
        float min=MinInNet();
        Point P,NewP;
        if(Rad>min) {
            Rad = min;
        }
        while(free > 0){
            P=FindFreePointInFile(file);
            NewP=FindCTFormalPointFromFile(file,FindInnerFormalPointFromFile(file,Rad,P));
            while(fabs(sqrt((P.x - NewP.x)*(P.x - NewP.x) + (P.y - NewP.y)*(P.y - NewP.y)))>100*eps){
                P=NewP;
                NewP=FindCTFormalPointFromFile(file,FindInnerFormalPointFromFile(file,Rad,P));
            }
            free-=FindFormalCircleFromFile(file,Rad,NewP);
        }
        ofstream  outfile("ForelFromFile.txt");
        for(FormalPoint Turtel:Cover){
            outfile<<Turtel.FormalElem.x<<" "<<Turtel.FormalElem.y<<" "<<Rad<<endl;
        }
        outfile.close();
        saveFFF(file);
        cout<<" Fish is ready:  "<<Cover.size()<<endl;
        cout<<" Data Ok";
    }

    vector<int> FindInnerFormalPointFromFile(FileVector<Point>& file,float Rad,Point P){//vozvrashchaet centr tyazhesti cherepashki
        //(radius cherepashki,v ehtoj tochki, vektor vsekh tochek polya)
        vector<int> inner;
        Point AP;
        FindSector(P);
        for (int i = 0; i < file.lineCount(); i++) {
            AP = file.readLine(i);
            if(AP.col==-1){
                for(int l=P.column-1;l<P.column+2;l++){
                    if(AP.column==l){
                        for(int k=P.string-1;k<P.string+2;k++){
                            if(AP.string==k){
                                if(sqrt((AP.x - P.x)*(AP.x - P.x) + (AP.y - P.y)*(AP.y - P.y)) < Rad ){
                                    inner.push_back(AP.TotalNumber);
                                }
                            }
                        }
                    }
                }
            }
        }
        return inner;
    }

    Point FindCTFormalPointFromFile(FileVector<Point>& file,vector<int> inner){
        Point NewP(0,0);
        Point P;
        for(int i : inner){
            P = file.readLine(i);
            NewP+=P;
        }
        NewP.x/=inner.size();
        NewP.y/=inner.size();
        return NewP;
    }


    int FindFormalCircleFromFile(FileVector<Point>& file,float Rad,Point P ){//pomechaet vse tochki konechnogo formal'nogo ehlementa
        Point TempP;
        vector<int> inner=FindInnerFormalPointFromFile(file,Rad,P);
        FormalPoint Turtel(P,inner,Rad);
        Cover.push_back(Turtel);
        for(int i:inner){
            TempP = file.readLine(i);
            TempP.col=Cover.size();
            file.writeLine(TempP, i);
        }
        PostLog::write(string("client_logs/client") + to_string(ClientID) + "_log.txt") << currentDateTime() << ": inner.size() = " << inner.size() << endl;

        return inner.size();
    }

    void MakeDataFile(FileVector<Point> &file){
        for(Point p : allpoint){
            file.pushLine(p);
        }
    }

    Point FindFreePointInFile(FileVector<Point>& file){//vozvrashchaet nomer ne pomechennoj tochki
        Point p(0,0);
        for (int i = 0; i < file.lineCount(); i++) {
            Point p = file.readLine(i);
            if (p.col == -1) {
                PostLog::write(string("client_logs/client") + to_string(ClientID) + "_log.txt") << currentDateTime() << ": New Point: "<<p<< endl;
                return p;
            }
        }
        return Point(0, 0);
    }

    void saveFFF(FileVector<Point>& file){//vozvrashchaet nomer ne pomechennoj tochki
        ofstream ofile("ALLPoint.txt");
        Point P;
        for (int i = 0; i < file.lineCount(); i++) {
            P=file.readLine(i);
           ofile<<P.x<<" "<<P.y<<" "<<P.col<<endl;
        }
        ofile.close();
    }


    //######################################################################################################################################################
    //FOREL FROM FIELD
    //######################################################################################################################################################



    void Forel(float Rad){//sozdaet massiv formal'nyh tochek s radiusom Rad
        NetOnField();
        int free=allpoint.size();
        float min=MinInNet();
        Point P,NewP;


        if(Rad>min) {
            Rad = min;
        }
        while(free > 0){
            P=allpoint[FindFreePoint()];
            NewP=FindCTFormalPoint(FindInnerFormalPoint(Rad,P));
            while(fabs(sqrt((P.x - NewP.x)*(P.x - NewP.x) + (P.y - NewP.y)*(P.y - NewP.y)))>100*eps){
                P=NewP;
                NewP=FindCTFormalPoint(FindInnerFormalPoint(Rad,P));
            }
            free-=FindFormalCircle(Rad,NewP);
        }

        ofstream  file("Forel.txt");
        for(FormalPoint Turtel:Cover){
            file<<Turtel.FormalElem.x<<" "<<Turtel.FormalElem.y<<" "<<Rad<<endl;
        }

        file.close();
        saveCol("ALLPoint.txt");
         FieldFromCover();
        cout<<" Fish is ready:  "<<Cover.size()<<endl;
    }


    vector<int> FindInnerFormalPoint(float Rad,Point P){//vozvrashchaet centr tyazhesti cherepashki
        //(radius cherepashki,v ehtoj tochki, vektor vsekh tochek polya)
        vector<int> inner;

        FindSector(P);
        for(Point AP:allpoint){
            if(AP.col==-1){
                for(int l=P.column-1;l<P.column+2;l++){
                    if(AP.column==l){
                        for(int k=P.string-1;k<P.string+2;k++){
                            if(AP.string==k){
                                if(sqrt((AP.x - P.x)*(AP.x - P.x) + (AP.y - P.y)*(AP.y - P.y)) < Rad ){
                                    inner.push_back(AP.TotalNumber);
                                }
                            }
                        }
                    }
                }
            }
        }


        return inner;
    }

    Point FindCTFormalPoint(vector<int> inner){
        Point NewP(0,0);
        for(int i : inner){
            NewP+=allpoint[i];
        }
        NewP.x/=inner.size();
        NewP.y/=inner.size();
        return NewP;
    }


    int FindFormalCircle(float Rad,Point P ){//pomechaet vse tochki konechnogo formal'nogo ehlementa
        vector<int> inner=FindInnerFormalPoint(Rad,P);
        FormalPoint Turtel(P,inner,Rad);
        Cover.push_back(Turtel);
        for(int i:inner){
            allpoint[i].col=Cover.size();
        }
        return inner.size();
    }

    int FindFreePoint(){//vozvrashchaet nomer ne pomechennoj tochki
        for(Point p : allpoint ){
            if(p.col==-1){
                return p.TotalNumber;
            }
        }
        return 0;
    }
    //##########################################################################################################################################################################
    //VOLNOVOI WITH NET!!!
    //##########################################################################################################################################################################

    void VolnovoiWithNet(float Por) {//algoritm sviaznih component (porog)
        NetOnField();
        float min=MinInNet();
        if(Por>min){
            cout<<"ERROR: Porog bol'shoi "<<endl;
        }else{
            vector<vector<int> > NewCloud = splitfieldWithNet(Por);

            CSResize(NewCloud.size());
            for (unsigned int i = 0; i < NewCloud.size(); i++) {
                for (unsigned int k = 0; k < NewCloud[i].size(); k++) {
                    CS[i].Add(NewCloud[i][k]);
                }
            }
            UpDateNumber();
            save("field.txt");
        }
    }

    vector<vector<int> > splitfieldWithNet(float Por) {//razdeliaem nomera tochek na sviaznie componenti (matrica dlin)
        vector<vector<int> > NewCloud;
        vector<int> Visited(allpoint.size(), 0);

        for (unsigned int i = 0; i < allpoint.size(); i++) {
            if (Visited[i] == 0) {
                waveWithNet(i,Por,Visited);
                NewCloud.push_back(vector<int>());
                for (unsigned int k = 0; k < allpoint.size(); k++) {
                    if (Visited[k] == 1) {
                        NewCloud[NewCloud.size() - 1].push_back(k);
                        Visited[k] = -1;
                    }
                }
            }
        }

        cout << "New kol-vo oblakov : " << NewCloud.size() << endl;
        return NewCloud;
    }

    void waveWithNet(int i,float Por,vector<int> &Visited) {//puskaem volnu iz tochki pod nomero i
        if (Visited[i] == 0) {
            Visited[i]++;
            vector<int> inner=FindInnerFormalPoint(Por,allpoint[i]);
            for (int k:inner) {
                waveWithNet(k,Por,Visited);
            }
        }
    }

    //##########################################################################################################################################################################
    //VOLNOVOI ALGHORITM!!!
    //##########################################################################################################################################################################

    vector<vector<float> > Matr() { //sozdaem matricu dlin(massive tochek)

        vector<vector<float> > Matr;

        Matr.resize(allpoint.size());
        for (unsigned int i = 0; i < allpoint.size(); i++) {
            for (unsigned int k = 0; k < allpoint.size(); k++) {
                Matr[i].push_back(Lenght(i, k));
            }
        }
        //   Vivod(Matr);
        return Matr;
    }

    vector<vector<float> >  EMatr(vector<vector<float> > Matr, float Por) {//binarnaya matrica dlya matricy dlin s usloviem dliny men'shej Por
        vector<vector<float> > E;
        E.resize(Matr.size());
        for (unsigned int i = 0; i <Matr.size(); i++) {
            E[i].resize(Matr.size());
            for (unsigned int k = 0; k < Matr.size(); k++) {
                if (k == i) {
                    E[i][k] = 0;
                }
                else if (Matr[i][k] < Por) {
                    E[i][k] = 1;
                }
                else if (Matr[i][k] > Por) {
                    E[i][k] = 0;
                }
            }
        }
        //    Vivod(E);
        return E;
    }

    void wave(int i, vector<vector<float> > &map, vector<int> &Visited) {//puskaem volnu iz tochki pod nomero i
        if (Visited[i] == 0) {
            Visited[i]++;
            for (unsigned int k = 0; k < map.size(); k++) {
                if ((fabs(map[i][k] - 1)) < eps) {
                    wave(k, map, Visited);
                }
            }
        }
    }

    vector<vector<int> > splitfield(vector<vector<float> > map) {//razdeliaem nomera tochek na sviaznie componenti (matrica dlin)
        vector<vector<int> > NewCloud;
        vector<int> Visited(map.size(), 0);

        for (unsigned int i = 0; i < map.size(); i++) {
            if (Visited[i] == 0) {
                wave(i, map, Visited);
                NewCloud.push_back(vector<int>());
                for (unsigned int k = 0; k < map.size(); k++) {
                    if (Visited[k] == 1) {
                        NewCloud[NewCloud.size() - 1].push_back(k);
                        Visited[k] = -1;
                    }
                }
            }
        }

        cout << "New kol-vo oblakov : " << NewCloud.size() << endl;
        return NewCloud;
    }

    void Volnovoi(float Por) {//algoritm sviaznih component (porog)
        vector<vector<float> > MatrLen = Matr();
        //Vivod(MatrLen);
        MatrLen = EMatr(MatrLen, Por);
        vector<vector<int> > NewCloud = splitfield(MatrLen);
        CSResize(NewCloud.size());
        for (unsigned int i = 0; i < NewCloud.size(); i++) {
            for (unsigned int k = 0; k < NewCloud[i].size(); k++) {
                CS[i].Add(NewCloud[i][k]);
            }
        }
        UpDateNumber();
        save("field.txt");
    }

    void CSResize(int NewSize){
        CS.clear();
        Cloud::last_cloud_number=0;
        for (int i = 0; i < NewSize; i++) {
            CS.push_back(Cloud(&allpoint));
        }
    }
    //##########################################################################################################################################################################
    //KMEANS!!!
    //##########################################################################################################################################################################


    void NewFieldKMeans(int k) { //peresborka polia ispolzuia k-means(4islo klasterov)
        vector<Point> Centr;
        vector<vector<int> > NCS = KMeans(k, &Centr,allpoint);
        //	LOG(Centr);
        CSResize(NCS.size());
        for (unsigned int i = 0; i <NCS.size(); i++) {
            CS[i].Centr.x = Centr[i].x;
            CS[i].Centr.y = Centr[i].y;
            for (unsigned int k = 0; k < NCS[i].size(); k++) {
                CS[i].Add(NCS[i][k]);
            }
        }
        UpDateNumber();
        PoiskNY(Centr);
        save("field.txt");
        cout << " New Field k-means is Ok" << endl;
    }

    vector<vector<int> > KMeans(int k, vector<Point>* Centr, vector<Point> &all) {//K-Means (4islo clusterov, massiv centrov clusterov,massiv vseh tochek polia)
                    vector<Point> NewCentres(k);
                    vector<Point> OldCentres(k);
                    vector<vector<int> > NewCloud;
                    NewCloud.resize(k);
                    BegCentr(NewCentres, all, k);
                    for (;;) {//Запускаем основной цикл
                            NewCloud = Bind(k, NewCentres, all, NewCloud);//Связываем точки с кластерами
                            OldCentres = NewCentres;//Высчитываем новые координаты центроидов
                            NewCentres = GetNewCentres(k, NewCloud, all);
                            if (OldCentres == NewCentres) {
                                    *Centr = NewCentres;
                                    return NewCloud;
                                    break;
                            }
                            NewCloud.clear();
                            NewCloud.resize(k);
                    }
                    return NewCloud;
            }

            void BegCentr(vector<Point> &NewCentres, vector<Point> &all, int k) {
                    int l = all.size() / k;
                    for (int i = 0; i < k; i++) {
                            NewCentres[i] = all[i*l];
                    }
            }

            vector<vector<int> > Bind(int k, vector<Point> &Centres, vector<Point> &all, vector<vector<int> >&NewCloud) {
                    float min;
                    float tmp;
                    Point* all_data = all.data(); // возвращает указтаелельль на первый элемент
                    for (unsigned int i = 0; i < all.size(); i++) {
                            Point* Centers_data = Centres.data();
                            min = sqrt(pow(((all_data + i)->x - (Centers_data + 0)->x), 2) + pow(((all_data + i)->y - (Centers_data + 0)->y), 2));
                            (all_data + i)->col = 0;
                            for (int j = 0; j < k; j++) {
                                    float xx = (all_data + i)->x - (Centers_data + j)->x;
                                    float yy = (all_data + i)->y - (Centers_data + j)->y;
                                    tmp = sqrt(xx * xx + yy * yy); // было all[i].x, стало (all_data + i)->x
                                    if (tmp < min) {
                                            (all_data + i)->col = j;
                                            min = tmp;
                                    }
                            }
                            NewCloud[(all_data + i)->col].push_back(i);
                    }

                    return NewCloud;
            }

            vector<Point> GetNewCentres(int k, vector<vector<int> > &NewCloud, vector<Point> &all) {
                    vector<Point> Centres(k);
                    Point* Centers_data = Centres.data();
                    float sumX = 0, sumY = 0;
                    for ( int i = 0; i < k; i++) {
                    //	int* NewCloud_i_data = NewCloud[i].data();
                            for (unsigned int j = 0; j < NewCloud[i].size(); j++) {
                                    sumX += all[NewCloud[i][j]].x;
                                    sumY += all[NewCloud[i][j]].y;
                            }
                            (Centers_data + i)->x = sumX / NewCloud[i].size();
                            (Centers_data + i)->y = sumY / NewCloud[i].size();
                            //cout << sumX << " " << sumY << endl;
                            //cout << Field[i].PC.size() <<endl;
                            sumX = 0;
                            sumY = 0;
                            //	cout << Centres[i].x << " " << Centres[i].y << endl;
                    }

                    //	cout << "New Centre OK" << endl;
                    return Centres;
            }


    //##########################################################################################################################################################################
    //KMEANS WITH KERNELS
    //##########################################################################################################################################################################

            void NewFieldKPMeans(int k,int p) { //peresborka polia ispolzuia k-means(4islo klasterov)

                vector<vector<int> > NCS = KPMeans(k,p);
                //	LOG(Centr);
                CSResize(NCS.size());
                for (unsigned int i = 0; i <NCS.size(); i++) {
                    for (unsigned int k = 0; k < NCS[i].size(); k++) {
                        CS[i].Add(NCS[i][k]);
                    }
                }
                UpDateNumber();

                save("field.txt");
                cout << " New Field kp-means is Ok" << endl;
            }


            vector<vector<int>> KPMeans(int k,int p) {//K-Means c iadrami (4islo clusterov, 4islo iader,massiv vseh tochek polia)
                    vector<vector<Point> > NewCentres(k);
                    vector<vector<Point> > OldCentres(k);
                    vector<vector<int> > NewAll(k);
                    vector<Point> Centr;


                    NewAll = KMeans(k, &Centr,allpoint);
                    NewCentres = GetNewPCentres(p, k, NewAll);




            //        for (int i = 0;i<20;i++) {//Запускаем основной цикл
                    for(;;){
                    LOG("!!!!!!!!!");
                            NewAll.clear();
                            NewAll.resize(k);
                            NewAll = PBind(p, k, NewCentres);

                            OldCentres = NewCentres;
                            NewCentres = GetNewPCentres(p, k, NewAll);

                            if (OldCentres == NewCentres) {
                                    break;
                            }

                    }

                    ofstream file("CentreP.txt");
                    for (unsigned int i = 0; i < NewCentres.size(); i++) {
                            for (unsigned int l = 0; l < NewCentres[i].size(); l++) {
                                 file << NewCentres[i][l].x<<" "<<NewCentres[i][l].y<<endl;
                            }
                    }
                    file.close();
                    cout << " OK \n";

                    return NewAll;
            }



            vector<vector<Point> > GetNewPCentres(int p, int k, vector<vector<int>> &NewAll) {
                    vector<vector<Point> > Centres(k);
                    vector<vector<int> > NewField(p);
                    for (int i = 0; i < k; i++){
                        vector<Point> InKMeans=NumbToPoint(NewAll[i]);
                            NewField = KMeans(p, &Centres[i],InKMeans);
                    }

                    return Centres;
            }
            vector<Point> NumbToPoint(vector<int> &all){
                vector<Point> PC;
                for(int i:all){
                    PC.push_back(allpoint[i]);
                }
                return PC;
            }

            vector<vector<int> > PBind(int p, int k, vector<vector<Point> > &NewCentres) {
                    float min=0,tmp=0;
                    vector<vector<int> > NewAll(k);
                    for (unsigned int i = 0; i < allpoint.size(); i++) {
                            min = PLenght(p, NewCentres[0], allpoint[i]);
                            allpoint[i].col = 0;
                            for (int j = 1; j < k; j++) {
                            tmp=PLenght(p, NewCentres[j], allpoint[i]);
                            if (tmp < min) {
                                    min = tmp;
                                    allpoint[i].col = j;
                            }
                            }
                            NewAll[allpoint[i].col].push_back(allpoint[i].TotalNumber);
                    }

                    return NewAll;
            }

            float PLenght(int p, vector<Point> Cen, Point P){
                    float sum = 0;
                    for (int i = 0; i < p; i++) {
                            sum += sqrt((Cen[i].x - P.x)*(Cen[i].x - P.x) + (Cen[i].y - P.y)*(Cen[i].y - P.y));
                    }
                    return sum;
            }
    //##########################################################################################################################################################################
    //POISK NY
    //##########################################################################################################################################################################


    float  PoiskNY(vector<Point> Centr) {//Poisk 4isla NY dlia massiva tochek all
        float SS = 0;

        for (unsigned int i = 0; i < CS.size(); i++) {
            SS += SumRastObl(CS[i].point_index);
        }
        SS += SumRastCentr(Centr);
        return SS;
    }

    float SumRastObl( vector<int> index) {
        float sum = 0;
        for (unsigned int i = 0; i < index.size(); i++) {
            for (unsigned int l = i; l < index.size(); l++) {
                sum +=Lenght(index[i],index[l]);
            }
        }

        return sum;
    }

    float SumRastCentr(vector<Point> &Centr) {
        float sum = 0;

        for (unsigned int i = 0; i <Centr.size(); i++) {
            for (unsigned int j = i; j <Centr.size(); j++) {
                sum += sqrt((Centr[i].x-Centr[j].x)*(Centr[i].x-Centr[j].x)+(Centr[i].y-Centr[j].y)*(Centr[i].y-Centr[j].y));
            }
        }
        return sum;
    }
    //##########################################################################################################################################################################
    //METHOD GLAVNIH COMPONENT
    //##########################################################################################################################################################################

    void MethodMC(int idCl, int idPl) {//Proektirovanie na idPl idCl oblaka + vivod sobstvennih vectorov (nomer oblaka,nomer ploskosti)
        double XMatr[3][3] = { { 0,0,0 },{ 0,0,0 },{ 0,0,0 }};
        float Xct = 0, Yct = 0, Zct = 0;
        // double     E[3][3] = { { 1,0,0 },{ 0,1,0 },{ 0,0,1 } };
        //double XMatr[3][3] = { { 5,-4,-1 },{ -4,5,-4 },{ -1,-4,5 } };
        CS[idCl].Projection(Planes[idPl]);
        CS[idCl].Noize();
        for (Point * p:CS[idCl].fpoints()) {
            Xct +=p->x;
            Yct += p->y;
            Zct += p->z;
        }
        Xct /= CS[idCl].point_index.size();
        Yct /= CS[idCl].point_index.size();
        Zct /= CS[idCl].point_index.size();

        for (Point * p:CS[idCl].fpoints()) {
            //LOG(p->x);
            //	LOG(XMatr[0][0]);
            XMatr[0][0] += (p->x - Xct)*(p->x - Xct);
            XMatr[1][1] += (p->y - Yct)*(p->y - Yct);
            XMatr[2][2] += (p->z - Zct)*(p->z - Zct);
            XMatr[0][1] += (p->x - Xct)*(p->y - Yct);
            XMatr[0][2] += (p->x - Xct)*(p->z - Zct);
            XMatr[1][2] += (p->y - Yct)*(p->z - Zct);
            XMatr[1][0] = XMatr[0][1]; XMatr[2][1] = XMatr[1][2]; XMatr[2][0] = XMatr[0][2];
        }
     /*   for (int i = 0; i < 3; i++) {
            for (int l = 0; l < 3; l++) {
                cout << XMatr[i][l] << "\t";
            }
            cout << endl;
        }*/



        double Trace = XMatr[0][0] + XMatr[1][1] + XMatr[2][2];
        double q = Trace / 3;
        double p1 = XMatr[0][1] * XMatr[0][1] + XMatr[0][2] * XMatr[0][2] + XMatr[1][2] * XMatr[1][2];
        double p2 = (XMatr[1][1] - q) *(XMatr[1][1] - q) + (XMatr[2][2] - q) *(XMatr[2][2] - q) + (XMatr[0][0] - q) *(XMatr[0][0] - q) + 2 * p1;
        double p = sqrt(p2 / 6);
        double     B[3][3] = { { 1/ p * (XMatr[0][0] - q),1 / p * XMatr[0][1],     1 / p * XMatr[0][2] },
                               { 1 / p * XMatr[1][0],     1 / p *(XMatr[1][1] - q),1 / p * XMatr[1][2] },
                               { 1 / p * XMatr[2][0],     1 / p * XMatr[2][1],     1 / p * (XMatr[2][2] - q) } };
        double DetB = B[0][0] * B[1][1] * B[2][2] + B[0][1] * B[1][2] * B[2][0] + B[0][2] * B[1][0] * B[2][1] - B[0][2] * B[1][1] * B[2][0] - B[0][0] * B[1][2] * B[2][1] - B[0][1] * B[1][0] * B[2][2];
        double r = DetB / 2;

        double phi;
        if (r <= -1) {
            phi = PI / 3;
        }
        else if (r >= 1) {
            phi = 0;
        }
        else {
            phi = acos(r) / 3;
        }


        double eig1 = q + 2 * p * cos(phi);
        double eig3 = q + 2 * p * cos(phi + (2 * PI / 3));
        double eig2 = 3 * q - eig1 - eig3;

        save3Pl(idCl, CS[idCl]);
        Point EigVec1, EigVec2, EigVec3;

        EigVec1 = EigVec(XMatr, eig1);
        EigVec2 = EigVec(XMatr, eig2);
        EigVec3 = EigVec(XMatr, eig3);


//        float Val1 = EigVec1.x * EigVec2.x + EigVec1.y * EigVec2.y + EigVec1.z * EigVec2.z;
//        float Val2 = EigVec2.x * EigVec3.x + EigVec2.y * EigVec3.y + EigVec2.z * EigVec3.z;
//        float Val3 = EigVec2.x * EigVec3.x + EigVec2.y * EigVec3.y + EigVec2.z * EigVec3.z;
        //float Sum = eig1 + eig2 + eig3;

        FILE* file = fopen("Eigenvec.txt", "w");
        fprintf(file, "%f %f %f %f %f %f\n", Xct, Yct, Zct, EigVec1.x, EigVec1.y , EigVec1.z);
        fprintf(file, "%f %f %f %f %f %f\n", Xct, Yct, Zct, EigVec2.x , EigVec2.y , EigVec2.z);
        fprintf(file, "%f %f %f %f %f %f\n", Xct, Yct, Zct, EigVec3.x * 0.1, EigVec3.y *0.1 , EigVec3.z * 0.1);
        cout << " OK \n";
        fclose(file);
        float dv1=sqrt(EigVec1.x*EigVec1.x +  EigVec1.y*EigVec1.y + EigVec1.z*EigVec1.z);
        float dv2=sqrt(EigVec2.x*EigVec2.x +  EigVec2.y*EigVec2.y + EigVec2.z*EigVec2.z);

        FILE* cfile = fopen("EigenvecOnPlane.txt", "w");
        fprintf(cfile, "%f %f %f %f \n", Xct, Yct, EigVec1.x/dv1, EigVec1.y/dv1);
        fprintf(cfile, "%f %f %f %f \n", Xct, Yct, EigVec2.x/dv2, EigVec2.y/dv2);
        cout << " OK \n";
        fclose(cfile);


        cout << "Good Projection" << endl;
    }


    Point EigVec(double XMatr[3][3], double eig)//nahodit sobstvenni vector matr XMatr c eig. value eig (matrice 3*3,eig.val)
    {
        //	double temp1;
        Point eigvec;
        eigvec.z = 1;
        double C[3][3] = { { (XMatr[0][0] - eig),  XMatr[0][1],        XMatr[0][2] },
                           {  XMatr[1][0],        (XMatr[1][1] - eig), XMatr[1][2] },
                           {  XMatr[2][0],         XMatr[2][1],       (XMatr[2][2] - eig) } };

        eigvec.y = (-C[1][2] * C[0][0] + C[0][2] * C[0][1]) / (C[1][1] * C[0][0] - C[0][1] * C[0][1]);
        eigvec.x = (-C[0][2] - C[0][1] * eigvec.y) / C[0][0];

        return eigvec;
    }

    void CreatePL(float A, float B, float C, float D) {
        Planes.push_back(Plane(A, B, C, D));
        cout << "Number Plane: " << Planes.size() - 1 << endl;
    }

    double Det(double B[3][3]) {
        return B[0][0] * B[1][1] * B[2][2] - B[0][0] * B[1][2] * B[2][1] - B[0][1] * B[1][0] * B[2][2] + B[0][1] * B[1][2] * B[2][0] + B[0][2] * B[1][0] * B[2][1] - B[0][2] * B[1][1] * B[2][1];
    }

    double Trace(double A[3][3]) {
        return A[0][0] + A[1][1] + A[2][2];
    }

    //##########################################################################################################################################################################
    //SPANNING TREE
    //##########################################################################################################################################################################

    void SpanningTree() {//nahodit porog dlia algoriyma sviazhih component
        vector<vector<float> > MatrLen = Matr();
        int N = allpoint.size();
        vector <float> ribs;
        vector <int> visited(2);
        vector <int> notvisited(N);
        float min = MatrLen[1][0];
        visited[0] = 0;
        visited[1] = 1;
        for (int i = 0; i < N; i++) {
            notvisited[i] = i;
            for (int l = i + 1; l < N; l++) {
                if (MatrLen[i][l] < min) {
                    visited[0] = i;
                    visited[1] = l;
                    min = MatrLen[i][l];
                }
            }
        }

        if (visited[0] > visited[1]) {
            float tmp = visited[0];
            visited[0] = visited[1];
            visited[1] = tmp;
        }
        notvisited.erase(notvisited.begin() + visited[0]);
        notvisited.erase(notvisited.begin() + visited[1] - 1);
        ribs.push_back(min);
        //LOG(MatrLen);
        AddTop(MatrLen, ribs, visited, notvisited,  N);
        float por = Gisto(ribs);
        Volnovoi(por);
    }

    float Gisto(vector <float> &ribs) {
        FILE* file = fopen("Gisto.txt", "w");

        float por = 0;
        vector<float> Gisto(31, 0);
        float min = ribs[0], max = ribs[0];
        for (unsigned int i = 0; i < ribs.size(); i++) {
            if (ribs[i] > max) {
                max = ribs[i];
            }
        }
        float d = (max - min) / (30);
        int ind;
        for (unsigned int i = 0; i < ribs.size(); i++) {
            ind = floor((-min + ribs[i]) / d);
            //	cout<<ind<<endl;
            Gisto[ind]++;
        }
        //cout<<"33333333"<<endl;
        for (int i = 0; i < 31; i++) {
            cout << Gisto[i] << " ";
            fprintf(file, "%f %f\n",i*d, Gisto[i]);

        }
        fclose(file);
        cout << endl;
        for (int i = 0; i < 30; i++) {
            if (fabs(Gisto[i])<eps) {
                por = min + (i + 1.0 / 2.0)*d;
                break;
            }
        }

        return por;
    }
    void AddTop(vector<vector<float> > &MatrLen, vector <float> &ribs, vector <int> &visited, vector <int> &notvisited,int N) {
        int K;

        //LOG(ribs.size());
        float Min = 1000000;

        visited.push_back(0);
        for (unsigned int i = 0; i < notvisited.size(); i++) {
            for (unsigned int l = 0; l < visited.size() - 1; l++) {
                if (MatrLen[notvisited[i]][visited[l]] < Min) {
                    Min = MatrLen[notvisited[i]][visited[l]];
                    visited[visited.size() - 1] = notvisited[i];
                    K = i;
                }
            }
        }
        /*notvisited[K] = notvisited.back();
                notvisited.pop_back();*/
        notvisited.erase(notvisited.begin() + K);
        ribs.push_back(Min);
        //LOG(ribs);
        int L=visited.size();
        if (L!= N) {
            AddTop(MatrLen, ribs, visited, notvisited, N);
        }
    }
    //##########################################################################################################################################################################
    //##########################################################################################################################################################################


    /*void MemberCHPor(float por) {//function of member po porogu (porog)
                Point p = Field_CT();
                Cloud C(p,10, 10,&Point);
                C.Create(1000);

        FILE* file = fopen("Big.txt", "w");




                cout << "1" << endl;
                MemberFuncChPor(por, C.PC);
                cout << "2" << endl;
        for (unsigned int i = 0; i < C.PC.size(); i++) {
                        if (C.PC[i].col > 0) {
                                cout << C.PC[i].col << endl;
                                CS[C.PC[i].col].PC.push_back(C.PC[i]);
                        }
            if(C.PC[i].col==0 ){
                fprintf(file, "%f %f\n",C.PC[i].x, C.PC[i].y);
            }
                }

        fclose(file);
                cout << "3" << endl;
        for (unsigned int i = 1; i < CS.size(); i++) {
                        save(string("cloud_") + to_string(i) + string(".txt"), i);
                }
                cout << "4" << endl;
        }

        void MemberFuncChPor(float Por, vector<Point> &P) {
        float X;
        for (unsigned int i = 0; i < P.size(); i++) {
                        P[i].col = 0;
            for (unsigned int l = 1; l< CS.size(); l++) {
                                X = MinLenToCLoud(P[i], l);
                                if (X < Por) {
                                        P[i].col = l;
                                }
                                if (X >= Por) {
                                }
                        }
                }
        }

        float MinLenToCLoud(Point p, int id) {
                float Len, min;
                min = sqrt((p.x - CS[id].PC[0].x)*(p.x - CS[id].PC[0].x) + (p.y - CS[id].PC[0].y)*(p.y - CS[id].PC[0].y));
        for (unsigned int i = 1; i < CS[id].PC.size(); i++) {
                        Len = sqrt((p.x - CS[id].PC[i].x)*(p.x - CS[id].PC[i].x) + (p.y - CS[id].PC[i].y)*(p.y - CS[id].PC[i].y));
                        if (Len < min) {
                                min = Len;
                        }
                }
                return min ;
        }
*/
    //##########################################################################################################################################################################
    //##########################################################################################################################################################################


    void MemberCHRad() {//function of mamber po radiusu
        Point p = Field_CT();
        vector<Point> Points;
        Cloud C(p, 10, 10,&Points);
        C.Create(1000);
        float X;
        save("field.txt");
        ofstream file("field.txt",ofstream::app);
        for (Point* p:C.fpoints()) {
            p->col = Cloud::last_cloud_number;
            for (Cloud &Cl:CS) {
                X = LenToCLoudCT(*p, Cl.this_cloud_number);
                if (X < MaxLenToCT(Cl.this_cloud_number)) {
                    p->col = Cl.this_cloud_number;
                }
            }
        }
        for (Point* p:C.fpoints()) {
            file<<p->x<<" "<<p->y<<" "<<p->col<<endl;
        }
        file.close();
        cout << "Ok Rad member" << endl;
        Cloud::last_cloud_number--;
    }

    float LenToCLoudCT(Point p, int id) {
        float  min;
        Point CT = CS[id].CT();
        min = sqrt((p.x - CT.x)*(p.x - CT.x) + (p.y - CT.y)*(p.y - CT.y));
        return min;
    }

    float MaxLenToCT(int id) {
        float Len, max;
        Point p;
        p = CS[id].CT();
        max = sqrt((p.x - allpoint[CS[id].point_index[0]].x)*(p.x - allpoint[CS[id].point_index[0]].x) + (p.y - allpoint[CS[id].point_index[0]].y)*(p.y - allpoint[CS[id].point_index[0]].y));
        for (Point* PP:CS[id].fpoints()) {
            Len = sqrt((p.x - PP->x)*(p.x - PP->x) + (p.y - PP->y)*(p.y - PP->y));
            if (Len > max) {
                max = Len;
            }
        }
        return max;
    }

    //##########################################################################################################################################################################
    //##########################################################################################################################################################################


    void Vivod(vector<vector<float> > &D) {//vivod matrici
        for (unsigned int i = 0; i < D.size(); i++) {
            for (unsigned int k = 0; k < D[i].size(); k++) {
                cout << D[i][k] << "\t";
            }
            cout << endl << endl;
        }
    }
};

bool cmp(float left, float right) {
    return abs(left - right) < 0.000001;
}

template <typename T>
bool eps_equal(T a, T b) {
    return std::abs(a - b) <= std::numeric_limits<T>::epsilon();
}


bool move_ptr(int& ptr, const std::string& str) {
    if (ptr < int(str.size()) - 1) {
        ptr++;
        return true;
    }
    ptr = str.size();
    return false;
}

bool test_ptr(int& ptr, const std::string& str, char c) {
    if (ptr < int(str.size())) {
        return str[ptr] == c;
    }
    return false;
}

bool match_int(const std::string& str, int& val, std::string& tail) {
    int ptr = 0;
    if (test_ptr(ptr, str, '+') || test_ptr(ptr, str, '-')) {
        move_ptr(ptr, str);
    }
    if (!isdigit(str[ptr])) {
        return false;
    }
    while (isdigit(str[ptr]) && move_ptr(ptr, str));

    std::string slice = str.substr(0, ptr);
    val = std::stoi(slice);
    tail = str.substr(ptr);
    return true;
}

bool match_float(const std::string& str, float& val, std::string& tail) {
    int ptr = 0;
    if (test_ptr(ptr, str, '+') || test_ptr(ptr, str, '-')) {
        move_ptr(ptr, str);
    }
    if (!isdigit(str[ptr])) {
        return false;
    }
    while (isdigit(str[ptr]) && move_ptr(ptr, str));
    if (str[ptr] == '.') {
        move_ptr(ptr, str);
    }
    while (isdigit(str[ptr]) && move_ptr(ptr, str));

    std::string slice = str.substr(0, ptr);
    val = std::stof(slice);
    tail = str.substr(ptr);
    return true;
}

bool skip_str(const std::string& str, const std::string& val, std::string& tail) {
    if (str.find(val) == 0) {
        std::string slice = str.substr(val.size());
        tail = slice;
        return true;
    }
    return false;
}

bool match_str(const std::string& str, std::string& val, std::string& tail) {
    int ptr = 0;
    while (move_ptr(ptr, str));

    std::string slice = str.substr(0, ptr);
    val = slice;
    tail = str.substr(ptr);
    return true;
}

std::string skip_spaces(const std::string& str) {
    unsigned ptr = 0;
    while (ptr < str.size() && (str[ptr] == ' ' || str[ptr] == '\n' || str[ptr] == '\t')) {
        ptr++;
    }
    return str.substr(ptr);
}

class parse {
private:
    std::string tail;
    bool err{false};
    std::string err_info;

public:
    parse(const std::string& tail_):
        tail(tail_) {}

    parse& float_(float& val) {
        if (err) return *this;
        handle_err(match_float(tail, val, tail), "float");
        tail = skip_spaces(tail);
        return *this;
    }

    parse& float_(float& val, float def_val) {
        if (err) return *this;
        if (tail == "") val = def_val;
        else handle_err(match_float(tail, val, tail), "float");
        tail = skip_spaces(tail);
        return *this;
    }

    parse& int_(int& val) {
        if (err) return *this;
        handle_err(match_int(tail, val, tail), "int");
        tail = skip_spaces(tail);
        return *this;
    }

    parse& int_(int& val, int def_val) {
        if (err) return *this;
        if (tail == "") val = def_val;
        else handle_err(match_int(tail, val, tail), "int");
        tail = skip_spaces(tail);
        return *this;
    }

    parse& str_(const std::string& str) {
        if (err) return *this;
        handle_err(skip_str(tail, str, tail), "string");
        tail = skip_spaces(tail);
        return *this;
    }

    parse& qrstr_(std::string& val) {
        if (err) return *this;
        handle_err(match_str(tail, val, tail), "string");
        tail = skip_spaces(tail);
        return *this;
    }

    parse& qrstr_(std::string& val, const std::string& def_val) {
        if (err) return *this;
        if (tail == "") val = def_val;
        else handle_err(match_str(tail, val, tail), "string");
        tail = skip_spaces(tail);
        return *this;
    }

    bool success() {
        return !err;
    }

    std::string get_err_info() {
        return err_info;
    }

private:
    void handle_err(bool cnd, const std::string& parser_type) {
        if (!cnd) {
            err = true;
            std::string s = (tail.size() < 7) ? tail : tail.substr(0, 7) + "...";
            err_info = std::string("can\'t parse \"") + s + "\" as " + parser_type;
        }
    }
};

class Contoller {
public:
    Field* ff{nullptr};
    bool clasterizing = false;
    bool ForelFlag = false;
    FileVector<Point> data_file;

    Contoller(Field* ff_) {
        ff = ff_;
        data_file.open("DataFile.txt");
    }

    bool handleCommand(string cmd, bool isAdmin, bool& usedAdminCmd) {
        if (cmd == "") {
            cout << "Pustoi vvod " << endl;
            return false;
        }

        bool first_block_passed = false;
        float x, y, d1, d2,por ,A ,B ,C ,D,Rad;
        int id, ang, N,k,idCl,idPl,p;
        std::string file;
        if (parse(cmd).str_("help").success()) {
            usedAdminCmd = false; if (usedAdminCmd && !isAdmin) return false;
            ifstream file;
            file.open("HELP.txt", ios_base::in);
            if (!file) {
                cout << " help will not come";
                return false;
            }
            string command;
            while (getline(file, command)) {
                cout << command << endl;
            }
            file.close();
            first_block_passed = true;
            return true;
        }

        else if (parse(cmd).str_("exit").success()) {
            usedAdminCmd = true; if (usedAdminCmd && !isAdmin) return false;
            cout << "Vihod\n";
            exit(0);//выходим и возвращаем 0
        }

        // BLOK COMMAND, DLY KOTORIH OBLAKA NE NUCZNI

        else if (parse(cmd).str_("create").int_(N).float_(d1).float_(d2).float_(x).float_(y).success()) {
            usedAdminCmd = true; if (usedAdminCmd && !isAdmin) return false;
            this->create(N, d1, d2, x, y);
            first_block_passed = true;
        }

        else if (parse(cmd).str_("load").qrstr_(file).success()) {
            usedAdminCmd = true; if (usedAdminCmd && !isAdmin) return false;
            ifstream f(file);
            if (!f) {
                cout << "No file\n";
                return false;
            }
            this->load(file);
            first_block_passed = true;
        }
        else if(parse(cmd).str_("CreatePl").float_(A).float_(B).float_(C).float_(D).success()){
            usedAdminCmd = true; if (usedAdminCmd && !isAdmin) return false;
            if (fabs(C)<eps) {
                cout << "4isla ne pravilnii\n";
                return false;
            }
            this->CreatePlane(A,B,C,D);
            first_block_passed = true;
        }
        else {
            if(first_block_passed){
                cout << "Nevernaya comanda\n";
                return false;
            }
        }

        // --------------------

        if (Cloud::last_cloud_number == 0) { // esli oblakov net
            cout << "Sozdaite oblaka\n";
            return false;
        }

        // BLOK COMMAND, DLY KOTORIH OBLAK
        if(parse(cmd).str_("sdvig").float_(x).float_(y).int_(id, Cloud::last_cloud_number -1 ).success()) {
            usedAdminCmd = true; if (usedAdminCmd && !isAdmin) return false;
            this->sdvig(x, y, id);
        }

        else if(parse(cmd).str_("deform").float_(x).float_(y).int_(id, Cloud::last_cloud_number -1 ).success()) {
            usedAdminCmd = true; if (usedAdminCmd && !isAdmin) return false;
            this->deform(x, y, id);
        }

        else if(parse(cmd).str_("rotate").int_(ang).int_(id, Cloud::last_cloud_number - 1 ).success()) {
            usedAdminCmd = true; if (usedAdminCmd && !isAdmin) return false;
            this->rotate(ang, id);
        }

        else if(parse(cmd).str_("rotate_ct").int_(ang).int_(id, Cloud::last_cloud_number - 1 ).success()) {
            usedAdminCmd = true; if (usedAdminCmd && !isAdmin) return false;
            this->rotate_ct(ang, id);
        }
        else if(parse(cmd).str_("Volnovoi").float_(por).success()){
            clasterizing = true;
            usedAdminCmd = true; if (usedAdminCmd && !isAdmin) return false;
            this->Volnovoi(por);
        }
        else if(parse(cmd).str_("VolnovoiWithNet").float_(por).success()){
            usedAdminCmd = true ; if (usedAdminCmd && !isAdmin) return false;
            this->VolnovoiWithNet(por);
        }
        else if(parse(cmd).str_("KMeans").int_(k).success()){
            clasterizing = true;
            usedAdminCmd = true ; if (usedAdminCmd && !isAdmin) return false;
            this->KMeans(k);
        }
        else if(parse(cmd).str_("KPMeans").int_(k).int_(p).success()){
            clasterizing = true;
            usedAdminCmd = true ; if (usedAdminCmd && !isAdmin) return false;
            ff->NewFieldKPMeans(k,p);
        }
        else if(parse(cmd).str_("Forel").float_(Rad).success()){
            ForelFlag = true;
            usedAdminCmd = false ; if (usedAdminCmd && !isAdmin) return false;
            this->Forel(Rad);
        }
        else if(parse(cmd).str_("ForelFromFile").float_(Rad).success()){
            ForelFlag = true;
            usedAdminCmd = false ; if (usedAdminCmd && !isAdmin) return false;
            this->ForelFromFile(Rad);
        }
        else if(parse(cmd).str_("KNN").float_(x).float_(y).int_(k).success()) {
            usedAdminCmd = true; if (usedAdminCmd && !isAdmin) return false;
            ff->KNN(x, y, k);
        }
        else if(parse(cmd).str_("MethodMC").int_(idCl).int_(idPl).success()){
            usedAdminCmd = false ; if (usedAdminCmd && !isAdmin) return false;
            if (idCl <= 0 || idCl >= Cloud::last_cloud_number || idPl < 0 || idPl >= (int) this->ff->Planes.size()) {
                cout << "Neprav. id or Pl\n";
                return false;
            }
            this->MethodMC(idCl,idPl);
        }
        else if(parse(cmd).str_("SpanningTree").success()){
            clasterizing = true;
            usedAdminCmd = true ; if (usedAdminCmd && !isAdmin) return false;
            this->STree();
        }
        else if(parse(cmd).str_("MemberCHRad").success()){
            usedAdminCmd = false ; if (usedAdminCmd && !isAdmin) return false;
            this->MemberCHRad();
        }
        else if (parse(cmd).str_("save").success()) {
            usedAdminCmd = false ; if (usedAdminCmd && !isAdmin) return false;
            this->ff->save("field.txt");
        }
        else if (parse(cmd).str_("MakeDataFile").success()) {
            usedAdminCmd = false ; if (usedAdminCmd && !isAdmin) return false;
            data_file.close();
            data_file.open("DataFile.txt");
            this->ff->MakeDataFile(data_file);
        }
        else if (parse(cmd).str_("NET").success()) {
            usedAdminCmd = false ; if (usedAdminCmd && !isAdmin) return false;
            this->NET();
        }

        else {
            if (!first_block_passed) {
                cout << "Nevernaya comanda\n";
                return false;
            }
        }

        // --------------------

        return true;
    }

    void sdvig(float x, float y, int id) {
        ff->CS[id].sdvig(x,y);
    }

    void deform(float k1, float k2, int id) {
        ff->CS[id].deform(k1,k2);
    }

    void rotate(float ang, int id) {
        ff->CS[id].rotateCloud(ang);
    }

    void rotate_ct(float ang, int id) {
        ff->CS[id].rotate_ct(ang);
    }

    void create(int N, float d1, float d2, float x, float y) {
        ff->CreateCloud(N, d1, d2, x, y);
    }

    bool load(string path) {
        ff->loadFromFile(path);
        return true;
    }
    void NET(){
        ff->NetOnField();
    }

    void Volnovoi(float por) {
        ff->Volnovoi(por);
    }

    void VolnovoiWithNet(float por) {
        ff->VolnovoiWithNet(por);
    }

    void KMeans(int k) {
        ff->NewFieldKMeans(k);
    }

    void MethodMC(int idCl,int idPl) {
        ff->MethodMC(idCl,idPl);
    }

    void CreatePlane(float A, float B, float C, float D) {
        ff->CreatePL(A, B, C, D);
    }
    void STree() {
        ff->SpanningTree();
    }
    void MemberCHRad() {
        ff->MemberCHRad();
    }

    void Forel(float Rad) {
        ff->Forel(Rad);
    }
    void ForelFromFile(float Rad) {
        ff->ForelFromFile(Rad);
    }

    // void MemberCHPor(float por) {
    // ff->MemberCHPor(por);
    // }

    // void KPMeans(int k,int p) {
    // ff->NewFieldKPMeans(k,p);
    // }
};

template <typename T>
ostream& operator<<(ostream& stream, vector<T> vec) {
    stream<< "[";
    for (unsigned i = 0; i < vec.size(); i++) {
        stream<< vec[i];
        if (i != vec.size() - 1) {
            stream<< ", ";
        }
    }
    stream<< "]";
    return stream;
}


