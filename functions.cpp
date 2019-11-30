#include "functions.h"

void relation::print()
{
    for(int i=0;i<this->num_tuples;i++)
    {
        std::cout<<this->tuples[i].key<<". "<<this->tuples[i].payload<<std::endl;
    }
}

relation::relation()
{
    num_tuples = 0;
    tuples = NULL;
}

relation::~relation()
{
    if (tuples != NULL)
        delete [] tuples;
}

InputArray::InputArray(uint64_t rowsNum, uint64_t columnsNum) {
    this->rowsNum = rowsNum;
    this->columnsNum = columnsNum;
    this->columns = new uint64_t*[columnsNum];
    for (int i = 0; i < columnsNum; i++) {
        this->columns[i] = new uint64_t[rowsNum];
    }
}

InputArray::~InputArray() {
    for (int i = 0; i < columnsNum; i++) {
        delete[] this->columns[i];
    }
    delete[] this->columns;
}

IntermediateArray::IntermediateArray(uint64_t columnsNum, uint64_t sortedByInputArrayId, uint64_t sortedByFieldId) {
    this->rowsNum = 0;
    this->sortedByFieldId = sortedByFieldId;
    this->sortedByInputArrayId = sortedByInputArrayId;
    this->columnsNum = columnsNum;
    this->results = new uint64_t*[columnsNum];
    for (int j = 0; j < columnsNum; j++) {
        this->results[j] = NULL;
    }
    this->inputArrayIds = new int[columnsNum];
}

IntermediateArray::~IntermediateArray() {
    for (int i = 0; i < columnsNum; i++) {
        if (this->results[i] != NULL) {
            delete[] this->results[i];
        }
    }
    delete[] this->results;
    delete[] this->inputArrayIds;
}

void IntermediateArray::extractFieldToRelation(relation* resultRelation, InputArray* inputArray, int inputArrayId, uint64_t fieldId) {
    resultRelation->tuples = new tuple[rowsNum];

    uint64_t columnIndex = 0;
    for (uint64_t j = 0; j < columnsNum; j++) {
        if (inputArrayIds[j] == inputArrayId) {
            columnIndex = j;
            break;
        }
    }

    for (uint64_t i = 0; i < rowsNum; i++) {
        uint64_t inputArrayRowId = this->results[columnIndex][i];
        
        resultRelation->tuples[i].key = i; // row id of this intermediate array
        resultRelation->tuples[i].payload = inputArray->columns[fieldId][inputArrayRowId];
    }
}

// by convention the 1st column of intermediateResult contains row ids of this IntermediateArray and the 2nd column contains row ids of the first-time-used input array
void IntermediateArray::populate(uint64_t (*intermediateResult)[2], uint64_t resultRowsNum, IntermediateArray* prevIntermediateArray, int inputArrayId) {
    this->rowsNum = resultRowsNum;

    for (uint64_t j = 0; j < columnsNum; j++) {
        results[j] = new uint64_t[resultRowsNum];
    }

    if (prevIntermediateArray == NULL) {
        // first time creating an IntermediateArray
        for (uint64_t i = 0; i < resultRowsNum; i++) {
            results[0][i] = intermediateResult[0][i];
            results[1][i] = intermediateResult[1][i];
        }
        return;
    }

    for (uint64_t i = 0; i < resultRowsNum; i++) {
        uint64_t prevIntermediateArrayRowId = intermediateResult[0][i];
        uint64_t inputArrayRowId = intermediateResult[1][i];
        for (uint64_t j = 0; j < prevIntermediateArray->columnsNum; j++) {
            inputArrayIds[j] = prevIntermediateArray->inputArrayIds[j];
            results[j][i] = prevIntermediateArray->results[j][prevIntermediateArrayRowId];
        }
        inputArrayIds[columnsNum - 1] = inputArrayId;
        results[columnsNum - 1][i] = inputArrayRowId;
    }
}

bool IntermediateArray::hasInputArrayId(int inputArrayId) {
    for (uint64_t j = 0; j < columnsNum; j++) {
        if (inputArrayIds[j] == inputArrayId) {
            return true;
        }
    }
    return false;
}

bool IntermediateArray::shouldSort(int nextQueryInputArrayId, uint64_t nextQueryFieldId) {
    return ! (this->sortedByInputArrayId == nextQueryInputArrayId && this->sortedByFieldId == nextQueryFieldId);
}

unsigned char hashFunction(uint64_t payload, int shift) {
    return (payload >> (8 * shift)) & 0xFF;
}

result* join(relation* R, relation* S,uint64_t**rr,uint64_t**ss,int rsz,int ssz,int joincol)
{
    int samestart=-1;
    int lstsize=1024*1024;
    //list* lst=new list(lstsize,rsz+ssz-1);
    list*lst=new list(lstsize,2);
    for(int r=0,s=0,i=0;r<R->num_tuples&&s<S->num_tuples;)
    {
        //std::cout<<"checking: R:"<<R->tuples[r].payload<<" S:"<<S->tuples[s].payload<<std::endl;
        int dec=R->tuples[r].payload-S->tuples[s].payload;
        if(dec==0)
        {
            //std::cout<<R->tuples[r].payload<<" same"<<std::endl; 
            //char x[lstsize+2];
            //x[0]='\0';
            lst->insert(R->tuples[r].key);
            lst->insert(S->tuples[s].key);
            /*for(int i=0;i<rsz;i++)
            {
                //sprintf(x,"%s%ld ",x,rr[i][R->tuples[r].key]);
                lst->insert(rr[i][R->tuples[r].key]);
            }
            for(int i=0;i<ssz;i++)
            {
                if(joincol==i)
                    continue;
                //sprintf(x,"%s%ld ",x,ss[i][S->tuples[s].key]);
                lst->insert(ss[i][S->tuples[s].key]);
            }*/
            //sprintf(x,"%s\n",x);
            //printf("%s",x);
            //sprintf(x,"%ld %ld\n",R->tuples[r].key,S->tuples[s].key);
        //lst->insert(x);
            //std::cout<<i<<". "<<R->tuples[r].key<<" "<<S->tuples[s].key<<std::endl;
            //std::cout<<x;
            i++;
            //std::cout<<"Matching: R:"<<R->tuples[r].key<<" S:"<<S->tuples[s].key<<std::endl;
            /*array[i][0]=R->tuples[r].key;
            array[i][1]=R->tuples[s].key;
            i++;*/
            switch(samestart)
            {
                case -1:
                    if(s+1<S->num_tuples&&S->tuples[s].payload==S->tuples[s+1].payload)
                        samestart=s;
                default:
                    if(S->tuples[samestart].payload!=S->tuples[s].payload)
                        samestart=-1;
            }
            if(s+1<S->num_tuples&&S->tuples[s].payload==S->tuples[s+1].payload)
            {
                s++;
            }
            else
            {
                r++;
                if(samestart>-1)
                {
                    s=samestart;
                    samestart=-1;
                }
            }
            continue;
        }
        else if(dec<0)
        {
            //std::cout<<R->tuples[r].payload<<" <(r)"<<std::endl;
            r++;
            /*if(samestart!=-1)
            {
                s=samestart;
                samestart=-1;
            }*/
            continue;
        }
        else
        {
            //std::cout<<R->tuples[r].payload<<" >(s)"<<std::endl;
            s++;
            continue;
        }
    }
    /*std::cout<<"got out"<<std::endl;
    for(int i=0;i<50000;i++)
    {
        if(array[i][0]==-1&&array[i][1]==-1)
            break;
        std::cout<<i<<". "<<array[i][0]<<" "<<array[i][1]<<std::endl;
    }*/
    std::cout<<std::endl;
    result* rslt=new result;
    rslt->lst=lst;
    return rslt;
    //lst->print();
    //lst->print();
    //evala Return gia na mhn vgazei Warning
    return NULL;
}

uint64_t** create_hist(relation *rel, int shift)
{
    int x = pow(2,8);
    uint64_t **hist = new uint64_t*[3];
    for(int i = 0; i < 3; i++)
        hist[i] = new uint64_t[x];
    uint64_t payload, mid;
    for(int i = 0; i < x; i++)
    {
        hist[0][i]= i;
        hist[1][i]= 0;
        hist[2][i]= shift;
    }

    for (int i = 0; i < rel->num_tuples; i++)
    {
        //mid = (0xFFFFFFFF & rel->tuples[i].payload) >> (8*shift);
        payload = hashFunction(rel->tuples[i].payload, 7-shift);

        if (payload > 0xFF)
        {
            std::cout << "ERROR " << payload << std::endl;
            return NULL;
        }
        hist[1][payload]++;
    }
    return hist;
}

uint64_t** create_psum(uint64_t** hist, int size)
{
    int count = 0;
    int x = size;
    uint64_t **psum = new uint64_t*[3];
    for(int i = 0; i < 3; i++)
        psum[i] = new uint64_t[x];

    for (int i = 0; i < x; i++)
    {
        psum[0][i] = hist[0][i];
        psum[1][i] = (uint64_t)count;
        psum[2][i] = hist[2][i];
        count+=hist[1][i];
    }
    return psum;
}

void pr(uint64_t** a, int array_size)
{
    int i;
    for (i = 0; i < array_size; i++)
    {
        if (a[1][i] != 0)
            std::cout << a[0][i] << ". " << a[1][i] << " - " << a[2][i] << std::endl;
    }
}

uint64_t** combine_hist(uint64_t** big, uint64_t** small, int position, int big_size)   //big_size == size of row in big
{
    int x = pow(2,8), i, j;    //size of small == pow(2,8)
    /*
    std::cout << "Small: " << std::endl;
    pr(small, x);
    std::cout << "----------" << std::endl;

    std::cout << "Big: " << std::endl;
    pr(big, big_size);
    std::cout << "----------" << std::endl;
    */

    uint64_t **hist = new uint64_t*[3];
    for(i = 0; i < 3; i++)
        hist[i] = new uint64_t[x + big_size];
    
    //std::memcpy(hist, big, sizeof(*big) * position);
    //std::memcpy(&hist[position], small, sizeof(*small)*x);
    //std::memcpy(&hist[position+x], &big[position], sizeof(*big)*(big_size-position-1));

    for (i = 0; i < position; i++)
    {
        hist[0][i] = big[0][i];
        hist[1][i] = big[1][i];
        hist[2][i] = big[2][i];
    }
    for (j = 0; j < x; j++)
    {
        hist[0][i] = small[0][j];
        hist[1][i] = small[1][j];
        hist[2][i] = small[2][j];
        i++;
    }
    /*std::cout << "Mid: " << std::endl;
    pr(hist, x);
    std::cout << "----------" << std::endl;*/
    for (i = position + 1; i < big_size; i++)
    {
        hist[0][i + x - 1] = big[0][i];
        hist[1][i + x - 1] = big[1][i];
        hist[2][i + x - 1] = big[2][i];
    }

    delete [] big[0];
    delete [] big[1];
    delete [] big[2];
    delete [] big;
    delete [] small[0];
    delete [] small[1];
    delete [] small[2];
    delete [] small;
    /*std::cout << "Hist: " << std::endl;
    pr(hist, big_size + x - 1);
    std::cout << "----------" << std::endl;*/
    return hist;
}

int find_shift(uint64_t **hist, int hist_size, uint64_t payload)
{
    int i, shift = -1;
    uint64_t hash;
    for (i = 0; i < hist_size; i++)
    {
        if (hashFunction(payload, 7 - hist[2][i]) == hist[0][i] && hist[1][i] != 0)
        {
            //std::cout << hist[0][i] << " " << hist[1][i] << " " << hist[2][i] << " " << std::endl;
            return hist[2][i];
        }
    }
}

relation* re_ordered_2(relation *rel, relation* new_rel)
{
    int shift = 0;
    int x = pow(2, 8), array_size = x;
    // relation *new_rel = new relation();
    relation *temp = NULL;
    relation *rtn  = NULL;
    //create histogram
    uint64_t** hist = create_hist(rel, shift), **temp_hist = NULL;
    //create psum
    uint64_t** psum = create_psum(hist, x), **temp_psum = NULL;
    uint64_t payload;
    int i, j, y;
    bool clear;

    bool *flag = new bool[rel->num_tuples];
    for (i = 0; i < rel->num_tuples; i++)
        flag[i] = false;

    i = 0;
    while(i < rel->num_tuples)
    {
        //hash
        //payload = (0xFFFFFFFF & rel->tuples[i].payload) >> (8*shift) & 0xFF;
        payload = hashFunction(rel->tuples[i].payload, 7 - shift);
        //find hash in psum = pos in new relation
        
        int next_i = psum[1][payload];

        //key++ until their is an empty place
        while ((next_i < rel->num_tuples) && flag[next_i])
            next_i++;

        if (next_i < rel->num_tuples)
        {
            new_rel->tuples[next_i].payload = rel->tuples[i].payload;
            new_rel->tuples[next_i].key = rel->tuples[i].key;
            flag[next_i] = true;
        }
        i++;
    }

    //testing
    /*for (int i = 0; i < x; i++)
        if (i == x-1 || psum[1][i] != psum[1][i+1])
            std::cout << psum[0][i] << " " << psum[1][i] << std::endl;
    std::cout << "<<<<<<<" << std::endl;*/

    clear = false; //make a full loop with clear == false to end
    i = 0;
    while (i < array_size)
    {
        if ((hist[1][i] > TUPLES_PER_BUCKET) && (hist[2][i]  < 7))
        {
            clear = true;
            //new relation from psum[1][i] to psum[1][i+1]
            if (rel == NULL)
                rel = new relation();
            int first = psum[1][i];
            int last = new_rel->num_tuples;
            if (i != array_size - 1)
                last = psum[1][i+1];
            rel->num_tuples = last - first;
            if(rel->tuples == NULL)
                rel->tuples = new tuple[new_rel->num_tuples];
            y = 0;
            for (j = first; j < last; j++)
            {
                rel->tuples[y] = new_rel->tuples[j];
                y++;
            }
            temp_hist = create_hist(rel, hist[2][i] + 1);
            temp_psum = create_psum(temp_hist, x);

            for (j = 0; j < rel->num_tuples; j++)
                flag[j] = false;
            //testing
            /*
            std::cout << "------" << std::endl;
            for (int k = 0; k < x; k++)
            if (k == x-1 || temp_psum[1][k] != temp_psum[1][k+1])
                std::cout << temp_hist[0][k] << " " << temp_hist[1][k] << " " << temp_hist[2][k] << std::endl;
            std::cout << "------" << std::endl;
            */
            hist = combine_hist(hist, temp_hist, i, array_size);
            array_size+=x;
            array_size-=1; //??????????????????????
            delete [] psum[0];
            delete [] psum[1];
            delete [] psum[2];
            delete [] psum;
            psum = create_psum(hist, array_size);
            delete [] temp_psum[0];
            delete [] temp_psum[1];
            delete [] temp_psum[2];
            delete [] temp_psum;

            for (j = 0; j < new_rel->num_tuples; j++)
                flag[j] = false;

            j = 0;
            if (rel == NULL)
                rel = new relation();
            if (sizeof(*rel->tuples) != sizeof(*new_rel->tuples))
            {
                delete [] rel->tuples;
                rel->tuples = new tuple[new_rel->num_tuples];
            }
            rel->num_tuples = new_rel->num_tuples;
            while(j < new_rel->num_tuples)
            {
                //hash
                //payload = (0xFFFFFFFF & rel->tuples[j].payload) >> (8*shift) & 0xFF;
                payload = hashFunction(new_rel->tuples[j].payload, 7 - find_shift(hist, array_size, new_rel->tuples[i].payload));
                //find hash in psum = pos in new relation
                
                int next_i = psum[1][payload];

                //key++ until their is an empty place
                while ((next_i < new_rel->num_tuples) && flag[next_i])
                    next_i++;

                if (next_i < new_rel->num_tuples)
                {
                    rel->tuples[next_i].payload = new_rel->tuples[j].payload;
                    rel->tuples[next_i].key = new_rel->tuples[j].key;
                    flag[next_i] = true;
                }
                j++;
            }
            tuple *temp_tuple = rel->tuples;
            rel->tuples = new_rel->tuples;
            new_rel->tuples = temp_tuple;
            j = rel->num_tuples;
            rel->num_tuples = new_rel->num_tuples;
            new_rel->num_tuples = j;
        }
        
        if (hist[1][i] < TUPLES_PER_BUCKET || hist[2][i] >= 7)
        {
            if (hist[1][i] > 0)
            {
                if (i + 1 < x)
                    sortBucket(new_rel, psum[1][i], psum[1][i+1] - 1);
                else
                    sortBucket(new_rel, psum[1][i], rel->num_tuples - 1);
            }
        }
        i++;
        if (i == array_size && clear)
        {
            i = 0;
            clear = false;
        }
    }

    delete [] hist[0];
    delete [] hist[1];
    delete [] hist[2];

    delete [] hist;
    
    delete [] psum[0];
    delete [] psum[1];
    delete [] psum[2];

    delete [] psum;
    delete [] flag;

    return new_rel;
}

relation* re_ordered(relation *rel, relation* new_rel, int shift)
{
    int x = pow(2, 8);
    // relation *new_rel = new relation();
    relation *temp;
    relation *rtn;
    //create histogram
    uint64_t** hist = create_hist(rel, shift);
    //create psum
    uint64_t** psum = create_psum(hist, x);
    uint64_t payload;
    int i, j, y;

    bool *flag = new bool[rel->num_tuples];
    for (i = 0; i < rel->num_tuples; i++)
        flag[i] = false;

    i = 0;
    while(i < rel->num_tuples)
    {
        //hash
        //payload = (0xFFFFFFFF & rel->tuples[i].payload) >> (8*shift) & 0xFF;
        payload = hashFunction(rel->tuples[i].payload, 7 - shift);
        //find hash in psum = pos in new relation
        
        int next_i = psum[1][payload];

        //key++ until their is an empty place
        while ((next_i < rel->num_tuples) && flag[next_i])
            next_i++;

        if (next_i < rel->num_tuples)
        {
            new_rel->tuples[next_i].payload = rel->tuples[i].payload;
            new_rel->tuples[next_i].key = rel->tuples[i].key;
            flag[next_i] = true;
        }
        i++;
    }

    //testing
    /*for (int i = 0; i < x; i++)
        if (i == x-1 || psum[1][i] != psum[1][i+1])
            std::cout << psum[0][i] << " " << psum[1][i] << std::endl;
    std::cout << "<<<<<<<" << std::endl;*/

    i = 0;
    while (i < x)
    {
        if ((hist[1][i] > TUPLES_PER_BUCKET) && (shift  < 7))
        {
            // new rel to re_order
            temp = new relation();
            temp->num_tuples = hist[1][i];
            temp->tuples = (new_rel->tuples + psum[1][i]);
            rtn = re_ordered(temp, rel, shift + 1);
            j = psum[1][i];
            y = 0;
            while (j < x)
            {
                if (j >= psum[1][i+1])
                    break;
                new_rel->tuples[j].payload = rtn->tuples[y].payload;
                new_rel->tuples[j].key = rtn->tuples[y].key;
                j++;
                y++;
            }
            std::free(temp); // free only relation's pointer because the tuples are not taking additional space
        }
        else if (hist[1][i] > 0)
        {
                //print bucket before sort
                /*std::cout << "{" << std::endl;
                if (i + 1 < x)
                {
                    for (int l = psum[1][i]; l < psum[1][i+1]; l++)
                        std::cout << new_rel->tuples[l].payload << ". " << new_rel->tuples[l].key << std::endl;
                }
                else
                {
                    for (int l = psum[1][i]; l < new_rel->num_tuples; l++)
                        std::cout << new_rel->tuples[l].payload << ". " << new_rel->tuples[l].key << std::endl;
                }
                std::cout << "}" << std::endl;
            std::cout << std::endl;*/
            if (i + 1 < x)
            {
                //std::cout << "-sort- " << psum[1][i] << " " << psum[1][i+1] << std::endl;
                sortBucket(new_rel, psum[1][i], psum[1][i+1] - 1);
            }
            else
            {
                //std::cout << "-sort- " << psum[1][i] << " " << new_rel->num_tuples << std::endl;
                sortBucket(new_rel, psum[1][i], rel->num_tuples - 1);
            }
        }
        
        //print buckets
        /*if (hist[1][i] > 0)
        {
            if (i + 1 < x)
            {
                for (int l = psum[1][i]; l < psum[1][i+1]; l++)
                    std::cout << new_rel->tuples[l].payload << ". " << new_rel->tuples[l].key << std::endl;
            }
            else
            {
                for (int l = psum[1][i]; l < new_rel->num_tuples; l++)
                    std::cout << new_rel->tuples[l].payload << ". " << new_rel->tuples[l].key << std::endl;
            }
            std::cout << std::endl;
        }*/
        
        i++;
    }

    // std::cout << "before sort" << std::endl;
    // new_rel->print();

    // sortBucket(new_rel, 0, 4);

    // std::cout << "after sort" << std::endl;
    // new_rel->print();

    delete [] hist[0];
    delete [] hist[1];

    delete [] hist;
    
    delete [] psum[0];
    delete [] psum[1];

    delete [] psum;
    delete [] flag;

    return new_rel;
}

void swap(tuple* tuple1, tuple* tuple2)
{
    uint64_t tempKey = tuple1->key;
    uint64_t tempPayload = tuple1->payload;

    tuple1->key = tuple2->key;
    tuple1->payload = tuple2->payload;

    tuple2->key = tempKey;
    tuple2->payload = tempPayload;
}

int randomIndex(int startIndex, int stopIndex) {
    srand(time(NULL));

    return rand()%(stopIndex - startIndex + 1) + startIndex;
}

int partition(tuple* tuples, int startIndex, int stopIndex)
{ 
    int pivotIndex = randomIndex(startIndex, stopIndex);

    uint64_t pivot = tuples[pivotIndex].payload;

    swap(&tuples[pivotIndex], &tuples[stopIndex]);

    int i = startIndex - 1;  // index of smaller element 
  
    for (int j = startIndex; j < stopIndex; j++) 
    {
        if (tuples[j].payload < pivot) 
        { 
            // if current element is smaller than the pivot 
            i++;    // increment index of smaller element 
            
            swap(&tuples[i], &tuples[j]);
        }
    }
    swap(&tuples[i + 1], &tuples[stopIndex]);
    return (i + 1);
}

// (startIndex, stopIndex) -> inclusive
void quickSort(tuple* tuples, int startIndex, int stopIndex)
{
    if (startIndex < stopIndex) 
    { 
        int partitionIndex = partition(tuples, startIndex, stopIndex); 
  
        quickSort(tuples, startIndex, partitionIndex - 1); 
        quickSort(tuples, partitionIndex + 1, stopIndex); 
    } 
}

// (startIndex, stopIndex) -> inclusive
void sortBucket(relation* rel, int startIndex, int stopIndex) {
    // stopIndex--;
    quickSort(rel->tuples, startIndex, stopIndex);
}

void extractcolumn(relation& rel,uint64_t **array, int column)
{
    rel.tuples=new tuple[rel.num_tuples];
    for(int i=0;i<rel.num_tuples;i++)
    {
        rel.tuples[i].key=i;
        rel.tuples[i].payload=array[column][i];
    }
}

InputArray** readArrays() {
    InputArray** inputArrays = new InputArray*[MAX_INPUT_ARRAYS_NUM]; // size is fixed

    size_t fileNameSize = MAX_INPUT_FILE_NAME_SIZE;
    char fileName[fileNameSize];

    unsigned int inputArraysIndex = 0;
    while (fgets(fileName, fileNameSize, stdin) != NULL) {
        fileName[strlen(fileName) - 1] = '\0'; // remove newline character

        uint64_t rowsNum, columnsNum, cellValue;
        FILE *fileP;

        fileP = fopen(fileName, "rb");

        fread(&rowsNum, sizeof(uint64_t), 1, fileP);
        fread(&columnsNum, sizeof(uint64_t), 1, fileP);

        inputArrays[inputArraysIndex] = new InputArray(rowsNum, columnsNum);

        for (uint64_t i = 0; i < columnsNum; i++) {
            for (uint16_t j = 0; j < rowsNum; j++) {
                fread(&inputArrays[inputArraysIndex]->columns[i][j], sizeof(uint64_t), 1, fileP);
            }
        }

        fclose(fileP);

        inputArraysIndex++;
    }

    return inputArrays;
}
char** readbatch(int& lns)
{
    char ch;
    list* l=new list(1024,0);
    int flag=0;
    int lines=0;
    while(1)
    {
        ch=getchar();
        if(ch=='\n'&&flag)
            continue;
        l->insert(ch);
        if(ch=='F'&&flag)
            break;
        if(ch=='\n')
        {
            flag=1;
            lines++;
        }
        else flag=0;
    }
    char* arr=l->lsttocharr();
    char** fnl=new char*[lines];
    int start=0;
    int ln=0;
    for(int i=0;arr[i]!='\0';i++)
    {
        if(arr[i]=='\n')
        {
            fnl[ln]=new char[i-start+1];
            memcpy(fnl[ln],arr+start,i-start);
            fnl[ln][i-start]='\0';
            ln++;
            start=i+1;
        }
    }
    /*std::cout<<"\n";
    for(int i=0;i<ln;i++)
    {
        std::cout<<fnl[i]<<std::endl;
    }*/
    delete[] arr;
    delete l;
    lns=ln;
    return fnl;
}
char** makeparts(char* query)
{
    //std::cout<<query<<std::endl;
    int start=0;
    char** parts;
    parts=new char*[3];
    for(int part=0,i=0;query[i]!='\0';i++)
    {
        if(query[i]=='|')
        {
            query[i]='\0';
            parts[part]=query+start;
            start=i+1;
            part++;
        }
    }
    parts[2]=query+start;
    return parts;
}
void handlequery(char** parts,InputArray** allrelations)
{
    /*for(int i=0;i<3;i++)
    {
        std::cout<<parts[i]<<std::endl;
    }*/
    std::cout<<std::endl;
    int relationsnum;
    InputArray** relations=loadrelations(parts[0],allrelations,relationsnum);
    InputArray* result=handlepredicates(relations,parts[1],relationsnum);
    handleprojection(result,parts[2]);
    std::cout<<std::endl;
    

}
InputArray** loadrelations(char* part,InputArray** allrelations,int& relationsnum)
{
    std::cout<<"LOADRELATIONS: "<<part<<std::endl;
    int cntr=1;
    uint64_t*** relations;
    for(int i=0;part[i]!='\0';i++)
    {
        if(part[i]==' ')
            cntr++;
    }
    relations=new uint64_t**[cntr];
    relationsnum=cntr;
    //std::cout<<cntr<<" relations"<<std::endl;
}
InputArray* handlepredicates(InputArray** relations,char* part,int relationsnum)
{
    std::cout<<"HANDLEPREDICATES: "<<part<<std::endl;
    int cntr;
    uint64_t** preds=splitpreds(part,cntr);
    for(int i=0;i<cntr;i++)
    {
        for(int j=0;j<5;j++)
        {
            std::cout<<"  "<<preds[i][j];
        }
        std::cout<<std::endl;
    }
    preds=optimizepredicates(preds,cntr,relationsnum);
    std::cout<<std::endl;
    for(int i=0;i<cntr;i++)
    {
        for(int j=0;j<5;j++)
        {
            std::cout<<"  "<<preds[i][j];
        }
        std::cout<<std::endl;
    }
    for(int i=0;i<cntr;i++)
    {
        /*******TO ANTONIS******************/
        //kathe grammi edo einai ena predicate olokliro
        //preds[i][0]=sxesi1
        //preds[i][1]=stili1
        //preds[i][2]=praxi opou
            //0  einai to >
            //1  einai to <
            //2  einai to =
        //preds[i][3]=sxesi2
            //sigrine me (uint64_t)-1 to opoio bgazei 18446744073709551615 kai theoroume an einai isa tote exoume filtro
        //preds[i][4]=stili2 
            //opou an h sxesi 2 einai isi me -1 opos eipa apo pano tote to stili 2 periexei to filtro
        /***********END***************************/

    }

    


}
void handleprojection(InputArray* array,char* part)
{
    std::cout<<"HANDLEPROJECTION: "<<part<<std::endl;
    int keep=1;
    for(int i=0;part[i]!='\0';i++)
    {
        if(part[i]==' ')
            keep++;
    }
    //InputArray* fnl=new InputArray(keep,array->columnsNum);
    /*for(int i=0,newcol=0;i<array->columnsNum;i++)
    {
        for(int j=0;j<array->rowsNum;j++)
        {
            //dostuff
        }
    }*/

}
uint64_t** splitpreds(char* ch,int& cn)
{
    int cntr=1;
    for(int i=0;ch[i]!='\0';i++)
    {
        if(ch[i]=='&')
            cntr++;
    }
    uint64_t** preds;
    preds=new uint64_t*[cntr];
    for(int i=0;i<cntr;i++)
    {
        preds[i]=new uint64_t[5];
    }
    cntr=0;
    int start=0;
    for(int i=0;ch[i]!='\0';i++)
    {
        if(ch[i]=='&')
        {
            //preds[cntr]=ch+start;
            //start=i+1;
            ch[i]='\0';
            predsplittoterms(ch+start,preds[cntr][0],preds[cntr][1],preds[cntr][3],preds[cntr][4],preds[cntr][2]);
            cntr++;
            start=i+1;
        }
    }
    predsplittoterms(ch+start,preds[cntr][0],preds[cntr][1],preds[cntr][3],preds[cntr][4],preds[cntr][2]);
    cntr++;
    cn=cntr;
    return preds;
}
bool notin(uint64_t** check, uint64_t* in, int cntr)
{
    
    for(int j=0;j<cntr;j++)
    {
        if(check[j]==in)
            return false;
    }
    
    return true;
}
uint64_t** optimizepredicates(uint64_t** preds,int cntr,int relationsnum)
{
    //filters first
    uint64_t** result=new uint64_t*[cntr];
    int place=0;
    for(int i=0;i<relationsnum;i++)
    {
        for(int j=0;j<cntr;j++)
        {
            if(preds[j][0]==i&&preds[j][3]==(uint64_t)-1)
            {
                if(notin(result,preds[j],cntr))
                {
                    result[place]=preds[j];
                    place++;
                }
            }
        }
    }
    for(int i=0;i<relationsnum;i++)
    {
        for(int j=0;j<cntr;j++)
        {
            if(preds[j][0]==i)
            {
                if(notin(result,preds[j],cntr))
                {
                    result[place]=preds[j];
                    place++;
                }
            }
        }
    }
    delete[] preds;
    return result;
    


}

void predsplittoterms(char* pred,uint64_t& rel1,uint64_t& col1,uint64_t& rel2,uint64_t& col2,uint64_t& flag)
{
    char buffer[1024];
    rel1=col1=rel2=col2=flag=-1;
    for(int start=0,end=0,i=0,j=0,term=0;(i==0)||(i>0&&pred[i-1])!='\0';i++,j++)
    {
        if(pred[i]=='.')
        {
            buffer[j]='\0';
            if(term==0)
                rel1=atoll(buffer);
            else
                rel2=atoll(buffer);
            j=-1;
            term++;
        }
        else if(pred[i]=='>'||pred[i]=='<'||pred[i]=='=')
        {
            if(pred[i]=='>')
                flag=0;
            else if(pred[i]=='<')
                flag=1;
            else if(pred[i]=='=')
                flag=2;
            buffer[j]='\0';
            col1=atoll(buffer);
            term++;
            j=-1;
        }
        else if(pred[i]=='\0')
        {
            buffer[j]='\0';
            col2=atoll(buffer);
        }
        else
            buffer[j]=pred[i];

    }
}

