#include "databasechar.h"
/*We should know that the max size of type(Char) is 255, if you set the size larger than 255
 * the size will force to be 255, whatever your input is.
 * */
DatabaseChar::DatabaseChar(int size)
{
    NullCondition();
    if (size > MAX_CHAR_SIZE)
        size = MAX_CHAR_SIZE;
    this->size = size;
    data = (char*)malloc(size + 1);
    memset(data, 0, size + 1);
    this->Nullable = true;
    this->isNull = false;
}
DatabaseChar::DatabaseChar(int size, bool cannull)
{
    NullCondition();
    if (size > MAX_CHAR_SIZE)
        size = MAX_CHAR_SIZE;
    this->size = size;
    data = (char*)malloc(size + 1);
    memset(data, 0, size + 1);
    this->Nullable = cannull;
    this->isNull = false;
}

//Return length of type(Char)
int DatabaseChar::getSize()
{
    return this->size + 1;
}
int DatabaseChar::getRealSize()
{
    return this->size;
}
int DatabaseChar::getMaxSize()
{
    return this->size + 1;
}
/* Don't try to use them in a public method. Try to use checkRight or
 * checkRightAndChange instead.
 * */
void DatabaseChar::change(string input)
{
    isNull = false;
    int size = input.length();
    memset(data, 0, this->size);
    memcpy(data, input.data(), size);
    data[this->size] = IS_NOT_NULL;
}
void DatabaseChar::change(char* input, int size)
{
    isNull = false;
    memset(data, 0, this->size);
    memcpy(data, input, size);
    data[this->size] = IS_NOT_NULL;
}
void DatabaseChar::changetoNull()
{
    isNull = true;
    data[this->size] = IS_NULL;
}

/* Really commend to use these two methods
 * */
bool DatabaseChar::checkRight(string input)
{
    int lengthofinput = input.length();
    if (lengthofinput > this->size)
        return false;
    return analysis(input);
}
bool DatabaseChar::checkRight(string input, bool inputisnull)
{
    if (inputisnull && !Nullable)
        return false;
    if (inputisnull)
        return true;
    int lengthofinput = input.length();
    if (lengthofinput > this->size)
        return false;
    return analysis(input);
}
bool DatabaseChar::checkRightAndChange(string input)
{
    bool can = checkRight(input);
    if (can) {
        change(input);
        return true;
    }
    return false;
}
bool DatabaseChar::checkRightAndChange(string input, bool inputisnull)
{
    if (inputisnull && !Nullable)
        return false;
    if (inputisnull) {
        changetoNull();
        return true;
    }
    bool can = checkRight(input);
    if (can) {
        change(input);
        return true;
    }
    return false;
}
/* Don't Be careful to use these two methods. If you want to use that,
 * you can delete char*input,
 * because databasechar will copy the content.
 * */
bool DatabaseChar::checkRightc(char* input, int size)
{
    if (size > this->size)
        return false;
    return analysisc(input, size);
}
bool DatabaseChar::checkRightc(char* input, int size, bool inputisnull)
{
    if (inputisnull && !Nullable)
        return false;
    if (inputisnull)
        return true;
    if (size > this->size)
        return false;
    return analysisc(input, size);
}
bool DatabaseChar::checkRightAndChangec(char* input, int size)
{
    if (checkRightc(input, size)) {
        change(input, size);
        return true;
    }
    return false;
}
bool DatabaseChar::checkRightAndChangec(char* input, int size, bool inputisnull)
{
    if (checkRightc(input, size, inputisnull)) {
        change(input, size);
        return true;
    }
    return false;
}
/* Return the type and the maxlength of Type(char)
 * */
string DatabaseChar::getType()
{
    return "Type: CHAR";
}
string DatabaseChar::output()
{
    if (isNull)
        return "NULL__DATA";
    string s(data);
    return s;
}

/* Don't delete that
 * */
DatabaseChar::~DatabaseChar()
{
}
bool DatabaseChar::read(char* input, int inputlen, int& position)
{
    memcpy(data, input, this->size + 1);
    position += this->size + 1;
    if (data[this->size] == IS_NULL)
        this->isNull = true;
    else
        this->isNull = false;
    return true;
}
bool DatabaseChar::write(char* input, int& position)
{
    memcpy(input, data, this->size + 1);
    position += this->size + 1;
    return true;
}
bool DatabaseChar::analysis(string input)
{
    string analysistype(condition, 4);
    if (analysistype == "NULL")
        return true;
    if (analysistype == "FRTO") {
        char* temp = (char*)malloc(size);
        memset(temp, 0, size);
        memcpy(temp, input.data(), input.length());
        int size1 = chartoint(condition + 4);
        char* par1 = (char*)malloc(size);
        memset(par1, 0, size);
        memcpy(par1, condition + 8, size1);
        int size2 = chartoint(condition + 8 + size1);
        char* par2 = (char*)malloc(size);
        memset(par2, 0, size);
        memcpy(par2, condition + 12 + size1, size2);
        int res1 = compare(temp, size, par1, size);
        int res2 = compare(temp, size, par2, size);
        free(temp);
        free(par1);
        free(par2);
        if (res1 >= 0 && res2 <= 0)
            return true;
        else
            return false;
    }
    if (analysistype == "frTO") {
        char* temp = (char*)malloc(size);
        memset(temp, 0, size);
        memcpy(temp, input.data(), input.length());
        int size1 = chartoint(condition + 4);
        char* par1 = (char*)malloc(size);
        memset(par1, 0, size);
        memcpy(par1, condition + 8, size1);
        int size2 = chartoint(condition + 8 + size1);
        char* par2 = (char*)malloc(size);
        memset(par2, 0, size);
        memcpy(par2, condition + 12 + size1, size2);
        int res1 = compare(temp, size, par1, size);
        int res2 = compare(temp, size, par2, size);
        free(temp);
        free(par1);
        free(par2);
        if (res1 > 0 && res2 <= 0)
            return true;
        else
            return false;
    }
    if (analysistype == "FRto") {
        char* temp = (char*)malloc(size);
        memset(temp, 0, size);
        memcpy(temp, input.data(), input.length());
        int size1 = chartoint(condition + 4);
        char* par1 = (char*)malloc(size);
        memset(par1, 0, size);
        memcpy(par1, condition + 8, size1);
        int size2 = chartoint(condition + 8 + size1);
        char* par2 = (char*)malloc(size);
        memset(par2, 0, size);
        memcpy(par2, condition + 12 + size1, size2);
        int res1 = compare(temp, size, par1, size);
        int res2 = compare(temp, size, par2, size);
        free(temp);
        free(par1);
        free(par2);
        if (res1 >= 0 && res2 < 0)
            return true;
        else
            return false;
    }
    if (analysistype == "frto") {
        char* temp = (char*)malloc(size);
        memset(temp, 0, size);
        memcpy(temp, input.data(), input.length());
        int size1 = chartoint(condition + 4);
        char* par1 = (char*)malloc(size);
        memset(par1, 0, size);
        memcpy(par1, condition + 8, size1);
        int size2 = chartoint(condition + 8 + size1);
        char* par2 = (char*)malloc(size);
        memset(par2, 0, size);
        memcpy(par2, condition + 12 + size1, size2);
        int res1 = compare(temp, size, par1, size);
        int res2 = compare(temp, size, par2, size);
        free(temp);
        free(par1);
        free(par2);
        if (res1 > 0 && res2 < 0)
            return true;
        else
            return false;
    }
    if (analysistype == "NTEQ") {
        char* temp = (char*)malloc(size);
        memset(temp, 0, size);
        memcpy(temp, input.data(), input.length());
        int size1 = chartoint(condition + 4);
        char* par1 = (char*)malloc(size);
        memset(par1, 0, size);
        memcpy(par1, condition + 8, size1);
        int res1 = compare(temp, size, par1, size);
        free(temp);
        free(par1);
        if (res1 != 0)
            return true;
        else
            return false;
    }
    if (analysistype == "CHOI") {
        char* temp = (char*)malloc(size);
        memset(temp, 0, size);
        memcpy(temp, input.data(), input.length());
        int paranum = chartoint(condition + 4);
        char* par = (char*)malloc(size);
        int index = 8;
        for (int i = 0; i < paranum; i++) {
            memset(par, 0, size);
            int sizei = chartoint(condition + index);
            index += 4;
            memcpy(par, condition + index, sizei);
            index += sizei;
            int res = compare(temp, size, par, size);
            if (res == 0) {
                free(par);
                free(temp);
                return true;
            }
        }
        free(par);
        free(temp);
        return false;
    }
    cout << "ERROR: No condition will be NUll" << endl;
    NullCondition();
    return true;
}
bool DatabaseChar::analysisc(char* input, int inputsize)
{
    string analysistype(condition, 4);
    if (analysistype == "NULL")
        return true;
    if (analysistype == "FRTO") {
        char* temp = (char*)malloc(size);
        memset(temp, 0, size);
        memcpy(temp, input, inputsize);
        int size1 = chartoint(condition + 4);
        char* par1 = (char*)malloc(size);
        memset(par1, 0, size);
        memcpy(par1, condition + 8, size1);
        int size2 = chartoint(condition + 8 + size1);
        char* par2 = (char*)malloc(size);
        memset(par2, 0, size);
        memcpy(par2, condition + 12 + size1, size2);
        int res1 = compare(temp, size, par1, size);
        int res2 = compare(temp, size, par2, size);
        free(temp);
        free(par1);
        free(par2);
        if (res1 >= 0 && res2 <= 0)
            return true;
        else
            return false;
    }
    if (analysistype == "frTO") {
        char* temp = (char*)malloc(size);
        memset(temp, 0, size);
        memcpy(temp, input, inputsize);
        int size1 = chartoint(condition + 4);
        char* par1 = (char*)malloc(size);
        memset(par1, 0, size);
        memcpy(par1, condition + 8, size1);
        int size2 = chartoint(condition + 8 + size1);
        char* par2 = (char*)malloc(size);
        memset(par2, 0, size);
        memcpy(par2, condition + 12 + size1, size2);
        int res1 = compare(temp, size, par1, size);
        int res2 = compare(temp, size, par2, size);
        free(temp);
        free(par1);
        free(par2);
        if (res1 > 0 && res2 <= 0)
            return true;
        else
            return false;
    }
    if (analysistype == "FRto") {
        char* temp = (char*)malloc(size);
        memset(temp, 0, size);
        memcpy(temp, input, inputsize);
        int size1 = chartoint(condition + 4);
        char* par1 = (char*)malloc(size);
        memset(par1, 0, size);
        memcpy(par1, condition + 8, size1);
        int size2 = chartoint(condition + 8 + size1);
        char* par2 = (char*)malloc(size);
        memset(par2, 0, size);
        memcpy(par2, condition + 12 + size1, size2);
        int res1 = compare(temp, size, par1, size);
        int res2 = compare(temp, size, par2, size);
        free(temp);
        free(par1);
        free(par2);
        if (res1 >= 0 && res2 < 0)
            return true;
        else
            return false;
    }
    if (analysistype == "frto") {
        char* temp = (char*)malloc(size);
        memset(temp, 0, size);
        memcpy(temp, input, inputsize);
        int size1 = chartoint(condition + 4);
        char* par1 = (char*)malloc(size);
        memset(par1, 0, size);
        memcpy(par1, condition + 8, size1);
        int size2 = chartoint(condition + 8 + size1);
        char* par2 = (char*)malloc(size);
        memset(par2, 0, size);
        memcpy(par2, condition + 12 + size1, size2);
        int res1 = compare(temp, size, par1, size);
        int res2 = compare(temp, size, par2, size);
        free(temp);
        free(par1);
        free(par2);
        if (res1 > 0 && res2 < 0)
            return true;
        else
            return false;
    }
    if (analysistype == "NTEQ") {
        char* temp = (char*)malloc(size);
        memset(temp, 0, size);
        memcpy(temp, input, inputsize);
        int size1 = chartoint(condition + 4);
        char* par1 = (char*)malloc(size);
        memset(par1, 0, size);
        memcpy(par1, condition + 8, size1);
        int res1 = compare(temp, size, par1, size);
        free(temp);
        free(par1);
        if (res1 != 0)
            return true;
        else
            return false;
    }
    if (analysistype == "CHOI") {
        char* temp = (char*)malloc(size);
        memset(temp, 0, size);
        memcpy(temp, input, inputsize);
        int paranum = chartoint(condition + 4);
        char* par = (char*)malloc(size);
        int index = 8;
        for (int i = 0; i < paranum; i++) {
            int sizei = chartoint(condition + index);
            index += 4;
            memcpy(par, condition + index, sizei);
            index += sizei;
            int res = compare(temp, size, par, size);
            if (res == 0) {
                free(par);
                free(temp);
                return true;
            }
        }
        free(par);
        free(temp);
        return false;
    }
    cout << "ERROR: No condition will be NUll" << endl;
    NullCondition();
    return true;
}
