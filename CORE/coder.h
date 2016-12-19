#ifndef CODER_H
#define CODER_H
#include<string>
#include<vector>
#include<QByteArray>
using namespace std;
typedef unsigned char byte;

struct FileHeader //文件头
{
    byte ID;//编号
    byte FileNum;//数据块数量
    byte CodeNum;//冗余块数量
    byte remainder;//剩余的字节数（fileLength%partition）
};

class Coder
{  
public:
    Coder(int partition,int redundancy);
    Coder();
    void Encode(char* buffer,int fLength, string output);
    char* Decode(const string inputs[],int cnt);
    vector<char *> Encode(char *buffer,int fLength);
    char * Decode(vector<char *>,long blockLength);
    vector<QByteArray> Encode(QByteArray buffer);
    QByteArray Decode(vector<QByteArray>);
    int GetLength();//block的实际大小
    int GetRemainder();
    int GetBlockLength();//block的大小（包含头）
    int GetPartition();
private:
    void InitialCauchyMatrix();//柯西矩阵
    void setup_tables();
    int mult(int a, int b);
    int div(int a, int b);
    void ProduceFile(byte* bytes, string path);
    void MakeHeaderForFile(byte* bytes,int i);//bytes用于求MD5值，暂时不用
    void swap(int j);
    void mycopy(byte *d,QByteArray s,int offset,int count);
private:
    int partition;//数据块
    int redundancy;//冗余块
    const static int NW = (1 << 8);
    int gflog[NW];
    int gfilog[NW];
    byte* E;//单位矩阵(partition*partition)
    byte* maxtrix;//编码矩阵(redundancy*partition)
    byte* inverted;//解码矩阵 (partition*partition)
    FileHeader Header;
    int remainder;
    int length;//block实际大小

};
#endif // CODER_H
