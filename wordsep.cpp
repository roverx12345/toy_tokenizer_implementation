#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<cstring>
#include<cmath>
#include<locale.h>
#include<iomanip>
#include<windows.h>

using namespace std;

#define OK 1
#define ERROR 0     //状态码

#define LIST_INIT_SIZE      10  //顺序表初始大小
#define LIST_INCREMENT       5  //顺序表每次增量

#define STACK_INIT_SIZE 100     //顺序栈初始大小
#define STACKINCREMENT 10       //顺序栈每次增量

#define MAX_INPUT_LEN 100   // 字串的最大字数
#define MAX_LINE_LEN 100     // 词典一行的最大长度
#define MAX_PATH_NUM 30     // 路径的最大数量
#define MAX_WORD_LENGTH 25  // 单个词语的最大字数

#define ENCODE_NUM 2    //使用的编码格式所需的字节数，如GBK为2，UTF-8为4

#define abs_equal(a,b) (fabs(a-b)<1e-5? 1:0)    // 浮点数比较函数

typedef int Status;     // 返回值，表示状态

string filename = "dict.txt";

struct character{       // 自定义数据结构，表示每个字，利用字符数组进行存储
    char ch[ENCODE_NUM+1];
    character(){
        memset(ch, 0, ENCODE_NUM+1);
    }
};

int total_word_num;     // 词表总词数
long long total_word_freq;  // 词表总词频
//---------顺序表构建部分--------
template <class T>      // 定义顺序表SqList类，包括push方法每次在表尾添加元素
class SqList{
    public:
        T* elem;
        int len;
        SqList();
        ~SqList();
        Status push(T e);
    private:
        int listsize;
        Status IncreList();
};

template <class T>
SqList<T>::SqList(){
    elem = (T*)malloc(LIST_INIT_SIZE*sizeof(T));
    if(!elem){
        printf("列表构建失败，请检查是否配置词典文件dict.txt！\n");
        exit(ERROR);
    }
    len = 0;
    listsize = LIST_INIT_SIZE;
}

template <class T>
SqList<T>::~SqList(){
    free(elem);
    elem = nullptr;
}
template <class T>
Status SqList<T>::IncreList(){
    T *olddata = this->elem;
    this->listsize = this->listsize + LIST_INCREMENT;
    this->elem = (T*)realloc(this->elem, this->listsize*sizeof(T));
    if(!this->elem){
        exit(ERROR);
    }
    return OK;
}

template <typename T>
Status SqList<T>::push(T e){
    if (this->len == this->listsize){
        if(this->IncreList() != OK){
            exit(ERROR);
        }
    }
    T* l = (T*)malloc(sizeof(T));
    *l = e;
    elem[len] = *l; 
    len++;
    free(l);
    l = nullptr;
    return OK;
}
//--------顺序表构建部分结束--------
//--------构建栈--------
template <class T>
class Stack{
    private:
        T* top;
        T* base;
        int stacksize;
    public:
        Stack();
        ~Stack();
        Status Push(T e);
        Status Pop(T &e);
};

template <class T>
Stack<T>::Stack(){
    this->base = (T*)malloc(STACK_INIT_SIZE*sizeof(T));
    if(!this->base){
		exit(ERROR);
    }
	this->top = this->base;
	this->stacksize = STACK_INIT_SIZE;
}

template <class T>
Stack<T>::~Stack(){
    free(this->base);
    this->top=nullptr;
    this->base=nullptr;
}

template <class T>
Status Stack<T>::Push(T e){
    if(this->top-this->base>=this->stacksize)
	{
		this->base=(T*)realloc(this->base,(this->stacksize+STACKINCREMENT)*sizeof(T));
		if(!this->base){
			exit(ERROR);
        }
		this->top=this->base+this->stacksize;
		this->stacksize+=STACKINCREMENT;
	}
	*(this->top) = e;
	this->top++;
	return OK;
}

template <class T>
Status Stack<T>::Pop(T &e)
{
	if(this->top==this->base){
		return ERROR;
    }
	else{	
		e=*(this->top-1);
		this->top--;
	}
	return OK;
}
//--------构建栈部分结束--------
//--------下面是Trie树的定义与相关操作--------
struct TrieNode{        // 自定义Trie树的基本单位，该Trie树采用二叉结构
    character* word;    // 每个节点所代表的字
    int freq;           // 当该节点是一个词语的尾部时，freq表示该词的词频
    TrieNode* LNode;    // 左子节点，表示下一个字
    TrieNode* RNode;    // 右子节点，表示与该词同级的兄弟节点
    TrieNode(char* wd, int len):freq(0){
        LNode = nullptr;
        RNode = nullptr;
        word = new character;
        strncpy(word->ch, wd, len);
    }
};

class Trie
{
	private:
        int cn2en_cmp(character* a, char* b, int len);  // 自定义字符串比较函数
	public:
	    TrieNode* root;
	    Trie();
	    Status Insert(char* s, int frequency);    // 向Trie树中添加词组
	    int Search(string s);       // 搜索Trie树上是否有某一个词组
};

//--------Trie树相关定义与操作结束--------
typedef struct{
    int pre;        // 指向该节点的上一个节点
    int pre_index;  // 该路径的序号
    int dis_index;  // 这条路径的总距离
    double all_dis; // 该路径在到达该点的所有路径中的排名
}PreNode;   // PreNode节点记录路径相关信息

int path_id = 0;    //路径序号
SqList<PreNode>* Nodelist[MAX_INPUT_LEN];   //每个节点路径相关信息表
int trajectory[MAX_PATH_NUM][MAX_INPUT_LEN]={0};    //最终路径

Status build_dict(Trie &tree);
Status char2character(char* s, SqList<character*> &S);
Status dag_construction(Trie &tree, SqList<character*> &words, double WordMatrix[MAX_INPUT_LEN+1][MAX_INPUT_LEN+1]);
void get_prenode(int input_len, double WordMatrix[MAX_INPUT_LEN+1][MAX_INPUT_LEN+1],int n);
void get_index(SqList<PreNode> &pn);
int Node_sort_by_dis(const void* a, const void* b);
void n_shortest_path(int n, SqList<character*> &words);
Status retro_back(int cur_node, int dis_index, Stack<int> &temp_stack);

int main(){
    system("chcp 936");
    Trie* word_dict = new Trie;
    if(build_dict(*word_dict)==OK){
        cout<<"词典构建完成！"<<endl;
    }else{
        cout<<"词典构建失败！"<<endl;
        return 0;
    }
    string respond = "y";
    while(true){
        char input_str[3*MAX_INPUT_LEN+1];
        cout<<"请输入待分词的中文语句:"<<endl;
        cout<<">>>";
        cin>>input_str;
        SqList<character*> words;
        char2character(input_str, words);   // 将输入字符串转变为以自定义基本单位character构成的顺序表，便于后续处理
        double WordMatrix[MAX_INPUT_LEN+1][MAX_INPUT_LEN+1]={0};    // 有向无环图的邻接矩阵表示
        dag_construction(*word_dict,words,WordMatrix);      // 构建有向无环图
        for (int i = 0; i <= words.len; i++)    //初始化
        {         
            Nodelist[i]=new SqList<PreNode>;
        }    
        int n;
        cout<<"请输入希望获得的分词类型数(n最短分词):"<<endl;
        cout<<">>>";
        cin>>n;
        get_prenode(words.len,WordMatrix,n);    // 构建PreNode储存路径相关信息
        n_shortest_path(n, words);  // 找到n条符合需求的路径
        for(int i = 0; i <= words.len; i++){    //清理内存占用
            delete Nodelist[i];
        }
        path_id = 0;
        memset(trajectory, 0, sizeof(trajectory));
        int pro_end = 0;
        while(true){
            cout<<"是否继续?[y/n]"<<endl;
            cout<<">>>";
            cin>>respond;
            if(respond == "n"){
                pro_end=1;
                break;
            }else if(respond == "y"){
                break;
            }else{
                cout<<"非法输入！"<<endl;
            }
        }
        if(pro_end){
            break;
        }
    }
    delete word_dict;

    return 0;
}

Trie::Trie()
{
    char root_ch = '#';
    root = new TrieNode(&root_ch, 1);
}

Status Trie::Insert(char* s, int frequency)
{
    TrieNode* node_pt = root;
    int size = strlen(s);
    for(int i = 0; i < size;){
        int len = s[i]&0x80?ENCODE_NUM:1;
        if(node_pt->LNode == nullptr){      // 若无左子节点就用当前词语创建一个
            node_pt->LNode= new TrieNode(&s[i], len);
            node_pt=node_pt->LNode;
        }else{
            int cmp=0;
            node_pt = node_pt->LNode;
            while(!(cmp=cn2en_cmp(node_pt->word,&s[i], len))&&node_pt->RNode!=nullptr){
                node_pt=node_pt->RNode;             // 向右寻找是否有匹配的节点
            }
            if(!cmp&&node_pt->RNode==nullptr){
                node_pt->RNode= new TrieNode(&s[i], len);   // 找不到匹配的节点就在最右边创建一个新节点储存当前词语
                node_pt=node_pt->RNode;
            }
        }
        i+=len;
    }
    if(node_pt->freq!=0){
        cout<<"词组 \""<<s<<"\" 已存在！"<<endl;
        if(frequency>node_pt->freq){
            node_pt->freq= frequency;
        }
        cout<<"使用频率 \""<<node_pt->freq<<"\" !"<<endl;
        return ERROR;
    }else{
        node_pt->freq = frequency; //在词尾处标记该词的频率 
        return OK;
    }
}
int Trie::Search(string str)//如果找到，就返回词频，如果是前缀，就返回-1
{
    TrieNode* node_pt = this->root;
    int size = str.length();
    char s[ENCODE_NUM*MAX_INPUT_LEN+1];
    strcpy(s,str.c_str());
    for(int i = 0; i < size;){
        int len = s[i]&0x80?ENCODE_NUM:1;   // 处理字符为英文的情况 
        if(node_pt->LNode == nullptr){
            return 0;
        }else{
            int cmp=0;
            node_pt = node_pt->LNode;
            while(!(cmp=cn2en_cmp(node_pt->word,&s[i], len))&&node_pt->RNode!=nullptr){
                node_pt=node_pt->RNode;
            }
            if(!cmp&&node_pt->RNode==nullptr){
                return 0;
            }
        }
        i+=len;
    }
    // 可能不存在该单词,只是一段前缀 ，此时返回-1
    if(node_pt->freq) return node_pt->freq;
    else return -1;
}

int Trie::cn2en_cmp(character* a, char* b, int len){
    for(int i = 0; i < len; i++){
        if(a->ch[i]!=b[i]){
            return 0;
        }
    }
    return 1;
}

//--------构建词典--------
Status build_dict(Trie &tree){
    FILE* dict = fopen(filename.c_str(), "r");
    if (dict == NULL) {
        return ERROR;
    }
    fseek(dict,0,0);
    int line_num=0;
    char line_word[MAX_LINE_LEN]={0};
    while(fgets(line_word,255,dict)){
         line_num++;
    }
    fseek(dict, 0L, SEEK_END);
	long end = ftell(dict);
	fseek(dict, 0L, SEEK_SET);
	long start = ftell(dict);
    int line_id = 0;
    char a[4] = {'|','/','-','\\'};	//存储相关的字符
    while (end!=start) {
        // 每次读取词典的一行，储存在line中
        char line[MAX_LINE_LEN];
        fgets(line, MAX_LINE_LEN, dict);

        int freq;
        char word[2*MAX_WORD_LENGTH+1];
        sscanf(line, "%s%d%*s",&word, &freq);
        if(tree.Insert(word, freq)==ERROR){// 将该词和频数加到Trie树上
            cout<<"该问题发生于第\""<<line_id<<"\"行"<<endl;
        };
        total_word_freq += freq;    // 将这一行的词语的频数加到词典总频数中
        total_word_num++;   // 词语总数加1
        start = ftell(dict);
        line_id++;
        if(line_id % 100 == 0){
            double percentage = (double)line_id/(double)line_num;
            cout << " [";	//进度条左边框
            int remain;     // 填充进度条剩余部分
		    for (remain = 0;remain < 50*percentage;remain++) {	//控制加载进度的显示
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | BACKGROUND_INTENSITY);	//设置加载条样式配置
                cout << "_";
            }
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY);	//恢复原配置
            for (int j = 0;j < 50-remain;j++) { //未加载进度显示
                printf("%c", '.'); 
            }	
            cout << "] ";	//进度条右边框
            printf("%3.0lf\%\t",100*percentage);
            int t = (line_id/100)%4;
            cout << a[t] << "字典加载中";	//显示和程序进行中提示和“正在加载”字样提示
            for (int j = 0;j < t; j++){ //实现动态显示
                cout << "."; 
            }	
            for (int j = 0;j < (4-t);j++) { 
                cout<<' '; 
            }
            putchar('\r');
        }
    }
    cout<<endl;
    return OK;
}
//--------构建词典结束--------
//--------定义DAG图必要元素-------
Status char2character(char* s, SqList<character*> &S){
    int len = strlen(s);
    for(int i=0; i < len;){
        character* l = new character;
        int char_len = s[i]&0x80?ENCODE_NUM:1;  // 每个汉字储存在一个character中，每个汉字用ENCODE_NUM个字节表示
        for(int j = 0; j < char_len; j++){
            l->ch[j] = s[i+j];
        }
        i += char_len;
        if(S.push(l)!=OK){
            return ERROR;
        }
    }
    return OK;
}
//--------DAG图必要元素定义结束--------
//--------利用词典内容构建DAG图--------
Status dag_construction(Trie &tree, SqList<character*> &words, double WordMatrix[MAX_INPUT_LEN+1][MAX_INPUT_LEN+1]){
    int len = words.len;
    for(int i = 0; i < len; i++){
        string temp;
        int len_j = len-i;
        for(int j = 0;j<len_j;j++){
            temp = temp + words.elem[i+j]->ch;
            int res = tree.Search(temp);
            if(res > 0){
                WordMatrix[i][i+j+1]=res;   // 将频数保存在邻接矩阵中
            }else if(res == 0){
                break;  // 如果未找到就跳出循环
            }           // 如果是前缀就继续找
        }
        if(i!=len-1&&temp.length()==strlen(words.elem[i]->ch)){
            WordMatrix[i][i+1]=-1;  //如果某个汉字在词典中不存在，就假设存在并设置为-1
        }
        if(abs_equal(WordMatrix[len-1][len],0)){
            WordMatrix[len-1][len] = -1;    // 在最后一个字后面添加一个假想字便于最后输出
        }
        // 对每个词的频数进行处理，方法为该词在词典中出现的
        // 概率的负自然对数，其中计算概率时采用拉普拉斯Add-one平滑
        for(int j = 0 ; j <= len;j++){
            if(abs_equal(WordMatrix[i][j],0)){
                WordMatrix[i][j]=INFINITY;
            }else if(abs_equal(WordMatrix[i][j],-1)){
                WordMatrix[i][j]=log(total_word_freq + total_word_num);
            }else{
                WordMatrix[i][j]=log(total_word_freq + total_word_num) - log1p(WordMatrix[i][j]);
            }
        }
    }
    return OK;
}
//--------利用词典内容构建DAG图结束--------

void get_prenode(int input_len, double WordMatrix[MAX_INPUT_LEN+1][MAX_INPUT_LEN+1], int n)//注意一下
{    
    int cur_index = 0;          // 当前节点  
    Nodelist[0]->push({-1,0, 0, 0});
    while (cur_index <= input_len){     
        qsort(Nodelist[cur_index]->elem,Nodelist[cur_index]->len, sizeof(Nodelist[cur_index]->elem[0]),Node_sort_by_dis);
        get_index(*Nodelist[cur_index]);                // 对当前节点的所有路径进行排序标号 
        for (int i = cur_index+1; i <= input_len; i++)  // 创建DAG图上的“边”，即不同的词语
        {            
            if (WordMatrix[cur_index][i]<INFINITY){  
                int last_id = 0;
                for(int t = 0 ; t < Nodelist[cur_index]->len&&last_id<=n; t++){
                    if(Nodelist[cur_index]->elem[t].dis_index==last_id){    // 对一条路径在尾节点上创建不同长度路径数目的点
                        continue;       
                    }
                    PreNode temp;
                    temp.all_dis = Nodelist[cur_index]->elem[t].all_dis+WordMatrix[cur_index][i];
                    temp.pre = cur_index;
                    temp.pre_index = Nodelist[cur_index]->elem[t].dis_index;
                    Nodelist[i]->push(temp);   
                    last_id++;
                }
            }               
        }     
        cur_index++;
    }
}
void get_index(SqList<PreNode> &pn){
    double last_dis = pn.elem[0].all_dis;
    int id = 1; //id从1开始
    for(int i = 0; i < pn.len; i++){
        if(!abs_equal(last_dis, pn.elem[i].all_dis)){
            id++;
            last_dis = pn.elem[i].all_dis;
        }
        pn.elem[i].dis_index = id;
    }
}
int Node_sort_by_dis(const void* a, const void* b){
    PreNode* pa = (PreNode*)a;
    PreNode* pb = (PreNode*)b;
    return pa->all_dis>pb->all_dis;
}
//--------PreNode构建完成--------
//--------返回n最短路径--------
void n_shortest_path(int n, SqList<character*> &words){
    int input_len = words.len;
    Stack<int> temp_stack;
    int path_num[MAX_PATH_NUM]={0};
    int index_num = 0;
    for(int i = 1; i <= n; i++){
        int base = -1;
        temp_stack.Push(base);
        int res = retro_back(input_len, i, temp_stack); // 利用递归从后向前进行查找
        if(res == ERROR){
            cout<<"最多只有"<<i-1<<"种分词方法！"<<endl;
            break;
        }else{
            path_num[i]=path_id-path_num[0];
            path_num[0]=path_id;
            index_num++;
        }
        temp_stack.Pop(base);
    }
    cout<<"序号\t分词结果"<<endl;
    int path_id_out = 0;
    for(int i = 1 ; i <= index_num;i++){
        cout<<setw(5)<<left<<i<<"\t";
        for(int j = 0; j < path_num[i];j++){
            int sep = 1;
            for(int k = 0 ; k < words.len; k++){
                if(k == trajectory[path_id_out][sep]){  // 在首字前打印斜线
                    cout<<"/";
                    sep++;
                }
                cout<<words.elem[k]->ch;
            }
            cout<<endl;
        }
        path_id_out+=path_num[i];
    }
}

Status retro_back(int cur_node, int dis_index, Stack<int> &temp_stack){
    if(cur_node == 0){
        if(path_id==MAX_PATH_NUM){
            return ERROR;
        }
        int t = 0;
        int t1 = 0;
        while(temp_stack.Pop(t)==OK){   // 栈中的数字即为路线，用trajectory储存
            trajectory[path_id][t1] = t;
            t1++;
        }
        while(t1>0){
            temp_stack.Push(trajectory[path_id][--t1]); // 再将数组中的数放回，便于递归搜索
        }
        path_id++;
        return OK;
    }else{
        int last = Nodelist[cur_node]->len-1;
        int id_last = Nodelist[cur_node]->elem[last].dis_index;
        if(id_last<dis_index){
            return ERROR;   // 路线数目不足n种
        }
        for(int i = 0; i < Nodelist[cur_node]->len; i++){
            if(Nodelist[cur_node]->elem[i].dis_index==dis_index){   // 将符合序号的前序节点入栈
                temp_stack.Push(Nodelist[cur_node]->elem[i].pre);
                if(retro_back(Nodelist[cur_node]->elem[i].pre,
                    Nodelist[cur_node]->elem[i].pre_index,temp_stack)==ERROR){
                    return ERROR;
                }
                int t;
                temp_stack.Pop(t);
            }else{
                continue;
            }
        }
        return OK;
    }
}
//--------n最短路径返回函数结束--------