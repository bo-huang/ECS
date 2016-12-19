#include "coder.h"
#include<cstring>
#include<fstream>

Coder::Coder(int partition,int redundancy)
{
    this->partition=partition;
    this->redundancy=redundancy;
}
Coder::Coder()
{

}
void Coder::setup_tables()
{
    int b = 1;//32 bits
    for (int log = 0; log < NW - 1; log++)
    {
        gflog[b] = log;       //构造逆对数表，对数表第一位0也没有给出
        gfilog[log] = b;      //构造对数表,对数表最后一位是0，没有给出

        b = (b << 1);        //不断地乘以α，使多项式左移。
        if ((b & (0x0100)) != 0) b = (b ^ (0x011D)); //本原多项式
    }
}

void Coder::Encode(char* buffer,int fLength, string output)
{
    string filenameData[partition];
    string filenameCode[redundancy];
    length = fLength /partition;//每个原文件记录数据长度
    remainder = fLength - length * partition;//剩余的字符
    byte data[partition][length+remainder];//数据块
    byte code[partition][length+remainder];//冗余块
    //文件名
    for(int i=0;i<redundancy;i++)
        filenameCode[i]=output+"_"+(char)('0'+i);
    for(int i=0;i<partition;i++)
        filenameData[i]=output+"_"+(char)('0'+redundancy+i);
    //将原数据分块，存到data数组中
    for(int i=0;i<partition-1;i++)
    {
        for(int j=0;j<length;j++)
            data[i][j]=(byte)buffer[i*length+j];
        MakeHeaderForFile(data[i], i);
        ProduceFile(data[i],filenameData[i]);

    }
    for(int i=length*(partition-1),j=0;i<fLength;i++,j++)
        data[partition-1][j]=buffer[i];
    MakeHeaderForFile(data[partition-1], partition-1);
    ProduceFile(data[partition-1],filenameData[partition-1]);

    //生成冗余块
    InitialCauchyMatrix();
    long len=length+remainder;
    for (int k = 0; k < redundancy; k++)
    {
        for (long i = 0; i < len; i++)
        {
            code[k][i] = 0;
            for (int j = 0; j < partition; j++)
            {
                code[k][i] ^= (byte)(mult((int)maxtrix[k*partition+j], (int)data[j][i]));//计算过程中都是以字节为单位的，有限域上面的加法运算：求异或
            }
        }
        MakeHeaderForFile(code[k], k+partition);
        ProduceFile(code[k],filenameCode[k]);
    }
    //释放空间
    delete []E;
    delete []maxtrix;
}
vector<char *> Coder::Encode(char *buffer,int fLength)
{
    length = fLength /partition;//每个原文件记录数据长度
    remainder = fLength - length * partition;//剩余的字符
    byte data[partition][length+remainder];//数据块
    byte code[partition][length+remainder];//冗余块
    vector<char *>block_datas;
    char *block_data;
    //将原数据分块，存到data数组中
    for(int i=0;i<partition;i++)
    {
        block_data = new char[length+remainder+4];
        block_data[0]= i;
        block_data[1]=partition;
        block_data[2]=redundancy;
        block_data[3]=remainder;
        if(i<partition-1)
        {
            for(int j=0;j<length;j++)
            {
                data[i][j]=(byte)buffer[i*length+j];
                block_data[j+4]=data[i][j];
            }
        }
        else
        {
            for(int k=length*i,j=0;k<fLength;k++,j++)
            {
                data[i][j]=buffer[k];
                block_data[j+4]=data[i][j];
            }
        }
        block_datas.push_back(block_data);
    }


    //生成冗余块
    InitialCauchyMatrix();
    long len=length+remainder;
    for (int k = 0; k < redundancy; k++)
    {
        block_data = new char[length+remainder+4];
        block_data[0]= k+partition;
        block_data[1]=partition;
        block_data[2]=redundancy;
        block_data[3]=remainder;
        for (long i = 0; i < len; i++)
        {
            code[k][i] = 0;
            for (int j = 0; j < partition; j++)
            {
                code[k][i] ^= (byte)(mult((int)maxtrix[k*partition+j], (int)data[j][i]));//计算过程中都是以字节为单位的，有限域上面的加法运算：求异或
            }
            block_data[i+4] = code[k][i];
        }
        block_datas.push_back(block_data);
    }
    //释放空间
    delete []E;
    delete []maxtrix;
    return block_datas;
}
vector<QByteArray> Coder::Encode(QByteArray buffer)
{
    int fLength = buffer.count();
    length = fLength /partition;//每个原文件记录数据长度
    remainder = fLength - length * partition;//剩余的字符
    //这样写当length很大时，栈空间溢出
    /*
    byte data[partition][length+remainder];//数据块
    byte code[partition][length+remainder];//冗余块
    */
    byte *data[partition];
    byte *code[partition];
    for(int i=0;i<partition;i++)
    {
        data[i]=new byte[length+remainder];
        code[i]=new byte[length+remainder];
    }
    vector<QByteArray>block_datas;
    char *block_data = new char[length+remainder+4];
    //将原数据分块，存到data数组中
    for(int i=0;i<partition;i++)
    {
        block_data[0]= i;
        block_data[1]=partition;
        block_data[2]=redundancy;
        block_data[3]=remainder;
        if(i<partition-1)
        {
            for(int j=0;j<length;j++)
            {
                data[i][j]=(byte)buffer[i*length+j];
                block_data[j+4]=data[i][j];
            }
        }
        else
        {
            for(int k=length*i,j=0;k<fLength;k++,j++)
            {
                data[i][j]=buffer[k];
                block_data[j+4]=data[i][j];
            }
        }
        block_datas.push_back(QByteArray(block_data,length+remainder+4));
    }


    //生成冗余块
    InitialCauchyMatrix();
    int len=length+remainder;
    for (int k = 0; k < redundancy; k++)
    {
        block_data[0]= k+partition;
        block_data[1]=partition;
        block_data[2]=redundancy;
        block_data[3]=remainder;
        for (long i = 0; i < len; i++)
        {
            code[k][i] = 0;
            for (int j = 0; j < partition; j++)
            {
                code[k][i] ^= (byte)(mult((int)maxtrix[k*partition+j], (int)data[j][i]));//计算过程中都是以字节为单位的，有限域上面的加法运算：求异或
            }
            block_data[i+4] = code[k][i];
        }
        block_datas.push_back(QByteArray(block_data,length+remainder+4));
    }
    //释放空间
    delete []E;
    delete []maxtrix;
    delete []block_data;
    for(int i=0;i<partition;i++)
    {
        delete []data[i];
        delete []code[i];
    }
    return block_datas;
}

char* Coder::Decode(const string inputs[],int cnt)
{
    if(cnt<partition)
        return NULL;
    InitialCauchyMatrix();
    ifstream is;
    bool used[partition+redundancy];
    bool evaluated[partition];
    byte temp[partition][length+remainder];//得到的数据文件
    byte A[partition][length+remainder];//得到的文件
    inverted=new byte[partition*partition];
    memset(used,0,sizeof(used));
    memset(evaluated,0,sizeof(evaluated));
    int n=0;
    for(int i=0;i<cnt;i++)
    {
        is.open(inputs[i],ios::binary);
        char tmp[4];
        is.read(tmp,4);
        Header.ID =tmp[0];
        if(i==0)
        {
            partition = tmp[1];
            redundancy = tmp[2];
        }
        if(!used[Header.ID])
        {
            used[Header.ID]=true;
            is.read((char *)A[n],length+remainder);
            if(Header.ID<partition)
            {
                memcpy(temp[Header.ID],A[n],length+remainder);
                memcpy(inverted+n*partition,E+Header.ID*partition,partition);              
                evaluated[Header.ID]=true;
            }
            else
            {
                memcpy(inverted+n*partition,maxtrix+(Header.ID-partition)*partition,partition);
            }
            n++;
        }
        is.close();
        if(n==partition)
            break;
    }
    if(n<partition)
        return NULL;

    //矩阵求逆
    for (int i = 0; i < partition; i++)
    {
        swap(i);
        int k = inverted[i*partition+i];//k此时应该是最大的值
        if (k > 1)
        {
            for (int j = 0; j < partition; j++)
            {
                inverted[i*partition+j] = (byte)(div((int)inverted[i*partition+j], k));
                E[i*partition+j] = (byte)(div((int)E[i*partition+j], k));
            }
        }
        for (int j = 0; j < partition; j++)
        {
            if ((j == i) || (inverted[j*partition+i] == 0)) continue;
            k = inverted[j*partition+i];
            for (int t = 0; t < partition; t++)
            {
                inverted[j*partition+t] = (byte)(div((int)inverted[j*partition+t], k));
                inverted[j*partition+t] ^= inverted[i*partition+t];
                E[j*partition+t] = (byte)(div((int)E[j*partition+t], k));
                E[j*partition+t] ^= E[i*partition+t];
            }
        }
    }
    for (int i = 0; i < partition; i++)
    {
        if ((inverted[i*partition+i] != 1))
            for (int j = 0; j < partition; j++)
                E[i*partition+j] = (byte)(div((int)E[i*partition+j], inverted[i*partition+i]));
    }


    //解码
    long w;
    long len=length+remainder;
    for(int i=0;i<partition;i++)
        if(!evaluated[i])
            for(int j=0;j<len;j++)
                temp[i][j]=0;
    for (int i = 0; i < partition; i++)
    {
        if (!evaluated[i])
            for (int j = 0; j < partition; j++)
            {
                for (w = 0; w < len; w++)
                {
                    temp[i][w] ^= (byte)mult(E[i*partition+j], A[j][w]);
                }
            }
    }
    byte data[length*partition+remainder];
    for (int t = 0; t < partition; t++)
    {
        if(t==partition-1)
        {
            memcpy(data+length*t,temp[t],length+remainder);
        }
        else
        {
            memcpy(data+length*t,temp[t],length);
        }
    }
    //释放空间
    delete []inverted;  
    delete []E;
    delete []maxtrix;
    return (char *)data;
}

char* Coder::Decode(vector<char *>blocks,long blockLength)
{
    int cnt = blocks.size();
    if(cnt==0)
        return NULL;
    partition = blocks[0][1];
    redundancy = blocks[0][2];
    remainder = blocks[0][3];
    length = blockLength - remainder - 4;//实际数据的长度（不包含最后一个block）

    if(cnt<partition)
        return NULL;
    InitialCauchyMatrix();
    bool used[partition+redundancy];
    bool evaluated[partition];
    byte temp[partition][length+remainder];//得到的数据文件
    byte A[partition][length+remainder];//得到的文件
    inverted=new byte[partition*partition];
    memset(used,0,sizeof(used));
    memset(evaluated,0,sizeof(evaluated));
    int n=0;
    for(int i=0;i<cnt;i++)
    {
        Header.ID =blocks[i][0];
        if(!used[Header.ID])
        {
            used[Header.ID]=true;
            memcpy((char *)A[n],blocks[i]+4,length+remainder);
            if(Header.ID<partition)
            {
                memcpy(temp[Header.ID],A[n],length+remainder);
                memcpy(inverted+n*partition,E+Header.ID*partition,partition);
                evaluated[Header.ID]=true;
            }
            else
            {
                memcpy(inverted+n*partition,maxtrix+(Header.ID-partition)*partition,partition);
            }
            n++;
        }
        if(n==partition)
            break;
    }
    if(n<partition)
        return NULL;

    //矩阵求逆
    for (int i = 0; i < partition; i++)
    {
        swap(i);
        int k = inverted[i*partition+i];//k此时应该是最大的值
        if (k > 1)
        {
            for (int j = 0; j < partition; j++)
            {
                inverted[i*partition+j] = (byte)(div((int)inverted[i*partition+j], k));
                E[i*partition+j] = (byte)(div((int)E[i*partition+j], k));
            }
        }
        for (int j = 0; j < partition; j++)
        {
            if ((j == i) || (inverted[j*partition+i] == 0)) continue;
            k = inverted[j*partition+i];
            for (int t = 0; t < partition; t++)
            {
                inverted[j*partition+t] = (byte)(div((int)inverted[j*partition+t], k));
                inverted[j*partition+t] ^= inverted[i*partition+t];
                E[j*partition+t] = (byte)(div((int)E[j*partition+t], k));
                E[j*partition+t] ^= E[i*partition+t];
            }
        }
    }
    for (int i = 0; i < partition; i++)
    {
        if ((inverted[i*partition+i] != 1))
            for (int j = 0; j < partition; j++)
                E[i*partition+j] = (byte)(div((int)E[i*partition+j], inverted[i*partition+i]));
    }


    //解码
    long w;
    long len=length+remainder;
    for(int i=0;i<partition;i++)
        if(!evaluated[i])
            for(int j=0;j<len;j++)
                temp[i][j]=0;
    for (int i = 0; i < partition; i++)
    {
        if (!evaluated[i])
            for (int j = 0; j < partition; j++)
            {
                for (w = 0; w < len; w++)
                {
                    temp[i][w] ^= (byte)mult(E[i*partition+j], A[j][w]);
                }
            }
    }
    byte data[length*partition+remainder];
    for (int t = 0; t < partition; t++)
    {
        if(t==partition-1)
        {
            memcpy(data+length*t,temp[t],length+remainder);
        }
        else
        {
            memcpy(data+length*t,temp[t],length);
        }
    }
    //释放空间
    delete []inverted;
    delete []E;
    delete []maxtrix;
    return (char *)data;
}
QByteArray Coder::Decode(vector<QByteArray> blocks)
{
    int cnt = blocks.size();
    if(cnt==0)
        return NULL;
    partition = blocks[0][1];
    redundancy = blocks[0][2];
    remainder = blocks[0][3];
    length = blocks[0].count() - remainder - 4;//实际数据的长度（最后一个block的长度还要加reminder）
    if(cnt<partition)
        return NULL;
    InitialCauchyMatrix();
    bool used[partition+redundancy];
    bool evaluated[partition];
    //同样的问题，会栈溢出
    /*
    byte temp[partition][length+remainder];//得到的数据文件
    byte A[partition][length+remainder];//得到的文件
    */
    byte *temp[partition];
    byte *A[partition];
    for(int i=0;i<partition;i++)
    {
        temp[i]= new byte[length+remainder];
        A[i]=new byte[length+remainder];
    }

    inverted=new byte[partition*partition];
    memset(used,0,sizeof(used));
    memset(evaluated,0,sizeof(evaluated));
    int n=0;
    for(int i=0;i<cnt;i++)
    {
        Header.ID = blocks[i][0];
        if(!used[Header.ID])
        {
            used[Header.ID]=true;
            mycopy(A[n],blocks[i],4,length+remainder);
            if(Header.ID<partition)
            {
                memcpy(temp[Header.ID],A[n],length+remainder);
                memcpy(inverted+n*partition,E+Header.ID*partition,partition);
                evaluated[Header.ID]=true;
            }
            else
            {
                memcpy(inverted+n*partition,maxtrix+(Header.ID-partition)*partition,partition);
            }
            n++;
        }
        if(n==partition)
            break;
    }
    if(n<partition)
        return NULL;

    //矩阵求逆
    for (int i = 0; i < partition; i++)
    {
        swap(i);
        int k = inverted[i*partition+i];//k此时应该是最大的值
        if (k > 1)
        {
            for (int j = 0; j < partition; j++)
            {
                inverted[i*partition+j] = (byte)(div((int)inverted[i*partition+j], k));
                E[i*partition+j] = (byte)(div((int)E[i*partition+j], k));
            }
        }
        for (int j = 0; j < partition; j++)
        {
            if ((j == i) || (inverted[j*partition+i] == 0)) continue;
            k = inverted[j*partition+i];
            for (int t = 0; t < partition; t++)
            {
                inverted[j*partition+t] = (byte)(div((int)inverted[j*partition+t], k));
                inverted[j*partition+t] ^= inverted[i*partition+t];
                E[j*partition+t] = (byte)(div((int)E[j*partition+t], k));
                E[j*partition+t] ^= E[i*partition+t];
            }
        }
    }
    for (int i = 0; i < partition; i++)
    {
        if ((inverted[i*partition+i] != 1))
            for (int j = 0; j < partition; j++)
                E[i*partition+j] = (byte)(div((int)E[i*partition+j], inverted[i*partition+i]));
    }


    //解码
    int w;
    int len=length+remainder;
    for(int i=0;i<partition;i++)
        if(!evaluated[i])
            for(int j=0;j<len;j++)
                temp[i][j]=0;
    for (int i = 0; i < partition; i++)
    {
        if (!evaluated[i])
            for (int j = 0; j < partition; j++)
            {
                for (w = 0; w < len; w++)
                {
                    temp[i][w] ^= (byte)mult(E[i*partition+j], A[j][w]);
                }
            }
    }
    //byte data[length*partition+remainder];
    byte *data = new byte[length*partition+remainder];
    for (int t = 0; t < partition; t++)
    {
        if(t==partition-1)
        {
            memcpy(data+length*t,temp[t],length+remainder);
        }
        else
        {
            memcpy(data+length*t,temp[t],length);
        }
    }
    QByteArray returnData((char *)data,length*partition+remainder);
    //释放空间
    delete []inverted;
    delete []E;
    delete []maxtrix;
    delete []data;
    for(int i=0;i<partition;i++)
    {
        delete []temp[i];
        delete []A[i];
    }
    return returnData;
}

void Coder::InitialCauchyMatrix()//柯西矩阵
{
    setup_tables();
    E=new byte[partition*partition];
    maxtrix = new byte[partition*redundancy];
    int n=partition*partition;
    for (int i = 0; i < n; i++)
    {
        if(i/partition==i%partition)
            E[i]=1;
        else
            E[i]=0;
    }
    n=partition*redundancy;
    for(int i=0;i<n;i++)
        maxtrix[i] = div(1, (i/partition) ^ (i%partition + redundancy));
}

int Coder::mult(int a, int b)
{
    int sum_log;
    if (a == 0 || b == 0) return 0;
    sum_log = gflog[a] + gflog[b];
    if (sum_log >= (NW - 1)) sum_log -= (NW - 1);//相减
    return gfilog[sum_log];
}

int Coder::div(int a, int b)
{
    int diff_log;
    if (a == 0) return 0;
    if (b == 0) return 0;
    diff_log = gflog[a] - gflog[b];
    if (diff_log < 0) diff_log += NW - 1;
    return gfilog[diff_log];
}

void Coder::ProduceFile(byte* bytes,string path)
{
    ofstream ofile;
    ofile.open(path,ios::binary);
    //写入文件头
    ofile.put(Header.ID);
    ofile.put(Header.FileNum);
    ofile.put(Header.CodeNum);
    ofile.put(Header.remainder);
    ofile.write((char *)bytes,length+remainder);
    ofile.close();
}

void Coder::MakeHeaderForFile(byte* bytes, int i)
{
    Header.FileNum = (byte)partition;
    Header.CodeNum = (byte)redundancy;
    Header.ID = (byte)i;
    Header.remainder = (byte)remainder;
}

void Coder::swap(int j) //j是 cut 文件序号，貌似是在求逆矩阵
{
    byte max = inverted[j*partition+j];
    int i = -1;
    // 找到每一列的最大值，有什么用呢
    for (int k = j + 1; k < partition; k++)
    {
        if (inverted[k*partition+j] > max)
        {
            i = k;//i记录下最大值所在的列
            max = inverted[k*partition+j];
        }
    }//逆矩阵是3*3 in this instance,then this for loop is to seek the maximum number.
    if (i != -1)//
    {
        char tmp[partition];
        memcpy(tmp,E+j*partition,partition);
        memcpy(E+j*partition,E+i*partition,partition);
        memcpy(E+i*partition,tmp,partition);

        memcpy(tmp,inverted+j*partition,partition);
        memcpy(inverted+j*partition,inverted+i*partition,partition);
        memcpy(inverted+i*partition,tmp,partition);
    }//把最大的值所在的列换到最上面去，为什么
}
int Coder::GetLength()
{
    return length;
}
int Coder::GetRemainder()
{
    return remainder;
}
int Coder::GetBlockLength()
{
    //4字节是写入的头部信息
    return length + remainder + 4;
}
int Coder::GetPartition()
{
    return partition;
}

void Coder::mycopy(byte *d, QByteArray s, int offset, int count)
{
    for(int i=0,j=offset;i<count;i++,j++)
        d[i]=s[j];
}
