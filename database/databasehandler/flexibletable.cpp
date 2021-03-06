#include "flexibletable.h"

FlexibleTable::FlexibleTable()
{
    this->PageNum = 0;
    this->MaxRecordSize = 0;
    this->reservedSizeInPage = NULL;
}
int FlexibleTable::getPageNum() { return PageNum; }
int FlexibleTable::getMaxRowNum() { return 0; }
int FlexibleTable::getRowSize(int rownum) { return 0; }

bool FlexibleTable::Initialize()
{
    FM = new FileManager;
    bool canopen = FM->openFile(filename.data(), fileid);
    if (!canopen) {
        bool cancreate = FM->createFile(filename.data());
        if (cancreate) {
            canopen = FM->openFile(filename.data(), fileid);
            if (!canopen)
                return false;
        } else {
            return false;
        }
    }
    BPM = new BufPageManager(FM);
    int index;
    BufType b = BPM->allocPage(fileid, 0, index, true);
    /*
cout<<"Now begin "<<"testment"<<endl;
for (int i=0;i<PAGE_INT_NUM;i++)
{
    int k=b[i];
    cout<<"No."<<i<<" is: "<<k<<endl;
}
cout<<"Now end "<<"testment"<<endl;
*/
    char* temp = (char*)malloc(4);
    memcpy(temp, b, 4);
    string ob1 = "HEAD";
    if (!UIC::equal(temp, ob1.data(), 4) || havecreatetable) {
        PackageHeadFile(b);
        BPM->markDirty(index);
        Nullindex();

    } else {
        PackageFromHeadFile(b);
        BPM->access(index);
        readindex();
    }
    free(temp);
    haveinitialize = true;
    return true;
}

void FlexibleTable::PackageFromHeadFile(BufType b)
{
    string filepath = filename + ".tableinfo";
    int f = open(filepath.c_str(), PARA);

    int namelen;
    read(f,(void*)&namelen,4);
    char* namechar = (char*)malloc(namelen);
    read(f,(void*)namechar,namelen);
    string namestr(namechar, namelen);
    free(namechar);
    name = namestr;

    read(f,(void*)&PageNum,4);
    read(f,(void*)&majornum,4);

    clearcolumn();
    read(f,(void*)&columncount,4);

    columnname = new string[this->columncount];
    column = new DataBaseType*[this->columncount];
    multivalue = new bool[this->columncount];
    for (int i = 0; i < this->columncount; i++) {

        int namelen;
        read(f,(void*)&namelen,4);
        char* namechar = (char*)malloc(namelen);
        read(f,(void*)namechar,namelen);
        string namestr(namechar, namelen);
        free(namechar);
        columnname[i] = namestr;

        char* temptype = (char*)malloc(4);
        read(f,(void*)temptype,4);

        int tempsize;
        read(f,(void*)&tempsize,4);

        char* nullable = (char*)malloc(4);
        read(f,(void*)nullable,4);

        bool cannull;
        if (nullable[0] == 'A')
            cannull = true;
        else
            cannull = false;
        char* canmulti = (char*)malloc(4);
        read(f,(void*)canmulti,4);
        if (canmulti[0] == 'A')
            multivalue[i] = true;
        else
            multivalue[i] = false;
        DataBaseType* t = UIC::realreconvert(temptype, tempsize, cannull);

        int conditionsize;
        read(f,(void*)&conditionsize,4);

        char* tempcondition = (char*)malloc(conditionsize);
        read(f,(void*)tempcondition,conditionsize);

        int position = 0;
        t->readcondition(tempcondition, conditionsize, position);

        column[i] = t;
        free(temptype);
        free(nullable);
        free(canmulti);
        free(tempcondition);
    }

    read(f,(void*)&MaxRecordSize,4);
    if (reservedSizeInPage != NULL)
        delete[] reservedSizeInPage;
    reservedSizeInPage = new int[MaxRecordSize];
    for (int i = 0; i < MaxRecordSize; i++)
        read(f,(void*)&reservedSizeInPage[i],4);

    tablecondition.clear();
    linkedcolumn.clear();
    foreignkeys.clear();
    int vecsize, strsize, first, second;
    read(f,(void*)&vecsize,4);
    for (int i = 0; i < vecsize; i++) {
        read(f,(void*)&first,4);
        read(f,(void*)&second,4);
        read(f,(void*)&strsize,4);
        char* tmp = (char*)malloc(strsize);
        read(f,(void*)tmp,strsize);
        string third(tmp, strsize);
        tablecondition.push_back(make_triple(first, second, third));
        free(tmp);
    }
    read(f,(void*)&vecsize,4);
    for (int i = 0; i < vecsize; i++)
    {
        vector<pair<string,int>> temp;
        temp.clear();
        linkedcolumn.push_back(temp);
        int vecisize2;
        read(f,(void*)&vecisize2,4);
        for (int j=0;j<vecisize2;j++)
        {
            read(f,(void*)&second,4);
            read(f,(void*)&strsize,4);
            char* tmp = (char*)malloc(strsize);
            read(f,(void*)tmp,strsize);
            string third(tmp, strsize);
            linkedcolumn[i].push_back(make_pair(third, second));
            free(tmp);
        }
    }
    read(f,(void*)&vecsize,4);
    for (int i = 0; i < vecsize; i++)
    {
        read(f,(void*)&second,4);
        read(f,(void*)&strsize,4);
        char* tmp = (char*)malloc(strsize);
        read(f,(void*)tmp,strsize);
        string third(tmp, strsize);
        foreignkeys.push_back(make_pair(third, second));
        free(tmp);
    }
    close(f);
}
void FlexibleTable::PackageHeadFile(BufType b)
{
    string filepath = filename + ".tableinfo";
    int can = access(filepath.c_str(), 0);
    if (can == -1) {
        FILE* f = fopen(filepath.c_str(), "ab+");
        if (f == NULL) {
            cout << "ERROR: CAN'T CREATE FILE" << endl;
        }
        fclose(f);
    }
    int f = open(filepath.c_str(), PARA);
    string headstr = "HEAD";
    memcpy(b, headstr.data(), 4);

    int namelen = name.length();
    write(f,(void*)&namelen,4);
    write(f,(void*)name.data(),namelen);
    write(f,(void*)&PageNum,4);
    write(f,(void*)&majornum,4);
    write(f,(void*)&columncount,4);

    for (int i = 0; i < columncount; i++) {
        int namelen = columnname[i].length();

        write(f,(void*)&namelen,4);
        write(f,(void*)columnname[i].data(),namelen);

        char* temptype = (char*)malloc(4);
        char* nullable = (char*)malloc(4);
        char* canmulti = (char*)malloc(4);
        UIC::convert(column[i], temptype, nullable);
        UIC::convertmulti(multivalue[i], canmulti);

        write(f,(void*)temptype,4);
        int tempsize = column[i]->getMaxSize();
        write(f,(void*)&tempsize,4);
        write(f,(void*)nullable,4);
        write(f,(void*)canmulti,4);

        free(temptype);
        free(nullable);
        free(canmulti);

        int conditionsize = column[i]->getconditionsize();
        write(f,(void*)&conditionsize,4);

        char* tempcondition = (char*)malloc(conditionsize);
        int position = 0;
        column[i]->writecondition(tempcondition, position);
        write(f,(void*)tempcondition,conditionsize);

        free(tempcondition);
    }
    write(f,(void*)&MaxRecordSize,4);
    for (int i = 0; i < MaxRecordSize; i++)
        write(f,(void*)&reservedSizeInPage[i],4);

    int vecsize = this->tablecondition.size(), strsize;
    write(f,(void*)&vecsize,4);
    for (int i = 0; i < vecsize; i++) {
        int first = tablecondition[i].first;
        int second = tablecondition[i].second.first;
        string third = tablecondition[i].second.second;
        strsize = third.length();
        write(f,(void*)&first,4);
        write(f,(void*)&second,4);
        write(f,(void*)&strsize,4);
        write(f,(void*)third.data(),strsize);
    }
    vecsize = this->linkedcolumn.size();
    write(f,(void*)&vecsize,4);
    for (int i = 0; i < vecsize; i++) {
        int vecisize2 = linkedcolumn[i].size();
        write(f,(void*)&vecisize2,4);
        for (int j=0;j<vecisize2;j++)
        {
            int second = linkedcolumn[i][j].second;
            string third = linkedcolumn[i][j].first;
            strsize = third.length();
            write(f,(void*)&second,4);
            write(f,(void*)&strsize,4);
            write(f,(void*)third.data(),strsize);
        }
    }
    vecsize = this->foreignkeys.size();
    write(f,(void*)&vecsize,4);
    for (int i = 0; i < vecsize; i++) {
        int second = foreignkeys[i].second;
        string third = foreignkeys[i].first;
        strsize = third.length();
        write(f,(void*)&second,4);
        write(f,(void*)&strsize,4);
        write(f,(void*)third.data(),strsize);
    }
    close(f);
}

void FlexibleTable::createTable(vector<string> clname, vector<DataBaseType*> cltype)
{
    remove(this->filename.c_str());
    string infoname = this->filename + ".tableinfo";
    remove(infoname.c_str());
    this->tablecondition.clear();
    this->clearcolumn();
    this->columncount = clname.size();
    linkedcolumn.clear();
    foreignkeys.clear();
    for (int i=0;i<columncount;i++)
    {
        vector<pair<string,int>> temp;
        temp.clear();
        linkedcolumn.push_back(temp);
        foreignkeys.push_back(make_pair("",0));
    }
    columnname = new string[this->columncount];
    column = new DataBaseType*[this->columncount];
    multivalue = new bool[this->columncount];
    for (int i = 0; i < columncount; i++) {
        columnname[i] = clname[i];
        column[i] = cltype[i];
        multivalue[i] = true;
    }
    this->PageNum = 0;
    this->MaxRecordSize = 5000;
    if (this->reservedSizeInPage != NULL)
        delete[] reservedSizeInPage;
    reservedSizeInPage = new int[MaxRecordSize];
    for (int i = 0; i < this->MaxRecordSize; i++)
        this->reservedSizeInPage[i] = PAGE_SIZE - 16;
    havecreatetable = true;
}
char* FlexibleTable::Packager(int totalsize)
{
    char* temp = (char*)malloc(totalsize);
    UIC::inttochar(totalsize, temp);
    int nowsize = 4;
    for (int i = 0; i < columncount; i++) {
        int csize = column[i]->getSize();
        UIC::inttochar(csize, temp + nowsize);
        nowsize += 4;
        memcpy(temp + nowsize, column[i]->getdata(), csize);
        nowsize += csize;
    }
    return temp;
}
void FlexibleTable::Reconstruct(int pagenum, BufType b)
{
    cout << "do a reconstruct" << endl;
    int pagerownum = 0;
    int reservedsize = 0;
    int reservedpointer = 0;
    pagerownum = UIC::chartoint(b + 4);
    reservedsize = UIC::chartoint(b + 8);
    reservedpointer = UIC::chartoint(b + 12);
    char** temprecord = new char*[pagerownum];
    int* preposition = new int[pagerownum];
    for (int i = 0; i < pagerownum; i++) {
        preposition[i] = UIC::chartoint(b + __position(i));
        temprecord[i] = getat(b, preposition[i]);
    }
    reservedpointer = 16;
    reservedsize = PAGE_SIZE - 16;
    int nowrownum = 0;
    for (int i = 0; i < pagerownum; i++)
        if (temprecord[i] != NULL) {
            int datalen = UIC::chartoint(temprecord[i]);
            modifyall(temprecord[i], datalen, pagenum, preposition[i], pagenum, reservedpointer);
            putat(b, reservedpointer, nowrownum, temprecord[i]);
            reservedpointer += datalen;
            reservedsize -= datalen + 4;
            nowrownum++;
        }
    UIC::inttochar(pagerownum, b + 4);
    UIC::inttochar(reservedsize, b + 8);
    UIC::inttochar(reservedpointer, b + 12);
    if (pagenum < this->MaxRecordSize) {
        this->reservedSizeInPage[pagenum] = reservedsize;
    }
    for (int i = 0; i < pagerownum; i++)
        if (temprecord[i] != NULL)
            free(temprecord[i]);
    delete[] temprecord;
}
char* FlexibleTable::getat(BufType b, int pageposition)
{
    if (pageposition == 0)
        return NULL;
    int recordsize = UIC::chartoint(b + pageposition);
    char* temprecord = (char*)malloc(recordsize);
    memcpy(temprecord, b + pageposition, recordsize);
    return temprecord;
}
void FlexibleTable::putat(BufType b, int pageposition, int rownum, char* data)
{
    int datalen = UIC::chartoint(data);
    memcpy(b + pageposition, data, datalen);
    UIC::inttochar(pageposition, b + __position(rownum));
}

bool FlexibleTable::InsertAt(int pagenum, char* insertdata, int& rownum)
{
    if (pagenum == 0)
        return false;
    int nowindex = 0;
    BufType b = BPM->getPage(fileid, pagenum, nowindex);
    int pagerownum = 0;
    int reservedsize = 0;
    int reservedpointer = 0;
    char* temp = (char*)malloc(4);
    string option1 = "PAGE";
    char* option2 = (char*)malloc(4);
    memset(option2, 0, 4);
    memcpy(temp, b, 4);
    if (UIC::equal(temp, option1.data(), 4)) {
        pagerownum = UIC::chartoint(b + 4);
        reservedsize = UIC::chartoint(b + 8);
        reservedpointer = UIC::chartoint(b + 12);
    } else if (UIC::equal(temp, option2, 4)) {
        memcpy(b, option1.data(), 4);
        pagerownum = 0;
        reservedsize = PAGE_SIZE - 16;
        reservedpointer = 16;
        UIC::inttochar(pagerownum, b + 4);
        UIC::inttochar(reservedsize, b + 8);
        UIC::inttochar(reservedpointer, b + 12);

    } else {
        // cout<<"ERROR:: ????"<<endl;
        memcpy(b, option1.data(), 4);
        pagerownum = 0;
        reservedsize = PAGE_SIZE - 16;
        reservedpointer = 16;
        UIC::inttochar(pagerownum, b + 4);
        UIC::inttochar(reservedsize, b + 8);
        UIC::inttochar(reservedpointer, b + 12);
    }
    free(temp);
    free(option2);
    int datalen = UIC::chartoint(insertdata);
    if (datalen + 4 > reservedsize) {
        //cout<<"error becaule datalen>reservedsize"<<' '<<datalen<<' '<<reservedsize<<endl;
        return false;
    }
    if (reservedsize > PAGE_SIZE - pagerownum * 4 - reservedpointer) {
        Reconstruct(pagenum, b);
        pagerownum = UIC::chartoint(b + 4);
        reservedsize = UIC::chartoint(b + 8);
        reservedpointer = UIC::chartoint(b + 12);
    }

    putat(b, reservedpointer, pagerownum, insertdata);
    rownum = pagerownum;

    insertall(insertdata, datalen, pagenum, rownum);

    //if (pagenum==1 && rownum<3)
    //cout<<rownum<<' '<<reservedpointer<<' '<<datalen<<endl;
    datalen = UIC::chartoint(b + reservedpointer);
    pagerownum++;
    reservedsize -= datalen + 4;
    reservedpointer += datalen;
    UIC::inttochar(pagerownum, b + 4);
    UIC::inttochar(reservedsize, b + 8);
    UIC::inttochar(reservedpointer, b + 12);
    if (pagenum < this->MaxRecordSize) {
        this->reservedSizeInPage[pagenum] = reservedsize;
    }
    BPM->markDirty(nowindex);
    return true;
}
bool FlexibleTable::modifypd(int pagenum, int rownum, BufType& ct,
    int& newindex, int& pagenewnum)
{
    if (pagenum > this->PageNum || pagenum == 0)
        return false;
    int nowindex = 0;
    BufType b = BPM->getPage(fileid, pagenum, nowindex);
    int pagerownum = 0;
    char* temp = (char*)malloc(4);
    string option1 = "PAGE";
    char* option2 = (char*)malloc(4);
    memset(option2, 0, 4);
    memcpy(temp, b, 4);
    if (UIC::equal(temp, option1.data(), 4)) {
        pagerownum = UIC::chartoint(b + 4);
    } else if (UIC::equal(temp, option2, 4)) {
        free(temp);
        free(option2);
        pagerownum = 0;
        return false;
    } else {
        cout << "ERROR:: ???" << endl;
        free(temp);
        free(option2);
        pagerownum = 0;
        return false;
    }
    free(temp);
    free(option2);
    if (rownum >= pagerownum)
        return false;
    ct = b;
    newindex = nowindex;
    pagenewnum = pagerownum;
    return true;
}

bool FlexibleTable::DeleteAt(int pagenum, int rownum)
{
    BufType b;
    int nowindex, pagerownum;
    if (!modifypd(pagenum, rownum, b, nowindex, pagerownum))
        return false;
    int position;
    position = UIC::chartoint(b + __position(rownum));
    if (position == 0)
        return false;

    int newposition = 0, currentsize;
    currentsize = UIC::chartoint(b + position);

    deleteall(b + position, currentsize, pagenum, rownum);

    UIC::inttochar(newposition, b + __position(rownum));

    int reservedsize = UIC::chartoint(b + 8);
    reservedsize -= currentsize + 4;
    UIC::inttochar(reservedsize, b + 8);
    if (pagenum < this->MaxRecordSize) {
        this->reservedSizeInPage[pagenum] = reservedsize;
    }
    BPM->markDirty(nowindex);
    return true;
}
FlexibleTable::~FlexibleTable()
{
    int index = 0;
    BufType b = BPM->getPage(fileid, 0, index);
    PackageHeadFile(b);
    BPM->markDirty(index);
    if (reservedSizeInPage != NULL)
        delete[] reservedSizeInPage;
}
int FlexibleTable::getPageRowNum(int pagenum)
{
    //cout << "maxrecordsize=" << this->MaxRecordSize << endl;
    if (pagenum > this->PageNum || pagenum < 1)
        return 0;
    int index = 0;
    BufType b = BPM->getPage(fileid, pagenum, index);
    int nowrownum = UIC::chartoint(b + 4);
    return nowrownum;
}
bool FlexibleTable::FastInsert(int& pagenum, int& rownum, Record* rec)
{
    bool can = InsertAt(pagenum, rec->getData(), rownum);
    if (!can)
        return false;
    return true;
}
bool FlexibleTable::FastAllInsert(int& pagenum, int& rownum, Record* rec)
{
    bool can = false;
    for (int i = min(this->MaxRecordSize - 1, this->PageNum); i > 0; i--)
        if (this->reservedSizeInPage[i] >= rec->getSize() + 4) {
            pagenum = i;
            can = FastInsert(pagenum, rownum, rec);
            if (!can) {
                cout << "ERROR:: ?????" << ' ' << i << ' ' << this->reservedSizeInPage[i] << ' ' << rec->getSize() << endl;
            }
            if (can) {
                if (this->PageNum < i)
                    this->PageNum = i;
                return true;
            }
        }
    for (int i = this->MaxRecordSize; i <= this->PageNum; i++) {
        pagenum = i;
        can = FastInsert(pagenum, rownum, rec);
        if (can)
            return true;
    }
    this->PageNum++;
    pagenum = this->PageNum;
    can = FastInsert(pagenum, rownum, rec);
    return can;
}

bool FlexibleTable::FastOutput(int pagenum, int rownum, Record* rec)
{
    int index;
    BufType b = BPM->getPage(fileid, pagenum, index);
    //cout<<"position at "<<__position(rownum)<<endl;
    int pageposition = UIC::chartoint(b + __position(rownum));
    if (pageposition == 0)
        return false;
    rec->Input(b + pageposition);
    return true;
}
void FlexibleTable::FastOutput(int pagenum, int rownum, char* output, int& outputsize)
{
    int index;
    BufType b = BPM->getPage(fileid, pagenum, index);
    int pageposition = UIC::chartoint(b + __position(rownum));
    if (pageposition == 0) {
        output = NULL;
        outputsize = 0;
    }
    outputsize = UIC::chartoint(b + pageposition);
    memcpy(output, b + pageposition, outputsize);
}
void FlexibleTable::modifyall(char* data, int datasize, int prepagenum, int prerownum, int newpagenum, int newrownum)
{
    int index = 4;
    for (int i = 0; i < columncount; i++)
    {
        int nowdatasize = UIC::chartoint(data + index);
        index += 4;
        if (DBindex != NULL && DBindex[i] != NULL) {

            ModifyindexAt(i, data + index, nowdatasize - 1, prepagenum, prerownum, newpagenum, newrownum);

        }
        index += nowdatasize;
    }
}
void FlexibleTable::deleteall(char* data, int datasize, int pagenum, int rownum)
{
    int index = 4;

    for (int i = 0; i < columncount; i++)
    {
        int nowdatasize = UIC::chartoint(data + index);
        index += 4;
        //string t(data + index, nowdatasize - 1);
        //cout << "delete at " << t << ' ' << pagenum << ' ' << rownum << endl;
        if (DBindex != NULL && DBindex[i] != NULL) {
            DeleteindexAt(i, data + index, nowdatasize - 1, pagenum, rownum);

        }
        index += nowdatasize;
    }
}
void FlexibleTable::insertall(char* data, int datasize, int pagenum, int rownum)
{

    int index = 4;
    for (int i = 0; i < columncount; i++)
    {
        int nowdatasize = UIC::chartoint(data + index);
        index += 4;
        if (DBindex != NULL && DBindex[i] != NULL) {

            //string t(data+index,nowdatasize);
            //cout<<"insert"<<' '<<t<<' '<<nowdatasize<<endl;
            InsertindexAt(i, data + index, nowdatasize - 1, pagenum, rownum);
        }
        index += nowdatasize;
    }
}
int FlexibleTable::getinfo(int reqhashnum, int pagenum, int rownum, vector<int>* infovec)
{
    //Don't need to write that
    return 0;
}
void FlexibleTable::createindex(vector<int> columnnum)
{
    bool* v = new bool[columncount];
    memset(v, 0, columncount);
    for (int bj : columnnum) {
        v[bj] = 1;
        createemptyindex(bj);
    }
    for (int pg = 1; pg <= PageNum; pg++) {
        int pageindex = 0;
        BufType b = BPM->getPage(fileid, pg, pageindex);
        int pagerownum = UIC::chartoint(b + 4);
        for (int i = 0; i < pagerownum; i++) {
            int pageposition = UIC::chartoint(b + __position(i));
            if (pageposition != 0) {
                int index = 4;
                for (int j = 0; j < this->columncount; j++) {
                    int nowdatasize = UIC::chartoint(b + pageposition + index);
                    index += 4;
                    //string t(data+index,nowdatasize);
                    //cout<<"insert"<<' '<<t<<' '<<nowdatasize<<endl;
                    if (v[j])
                        InsertindexAt(j, b + pageposition + index, nowdatasize - 1, pg, i);
                    index += nowdatasize;
                }
            }
        }
    }
    delete[] v;
}

string FlexibleTable::gettabletype()
{
    return "Flexible";
}

unsigned long FlexibleTable::getTraverseCost() {
    return (unsigned long) getPageNum();
}
