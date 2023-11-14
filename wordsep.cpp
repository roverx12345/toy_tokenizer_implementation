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
#define ERROR 0     //״̬��

#define LIST_INIT_SIZE      10  //˳����ʼ��С
#define LIST_INCREMENT       5  //˳���ÿ������

#define STACK_INIT_SIZE 100     //˳��ջ��ʼ��С
#define STACKINCREMENT 10       //˳��ջÿ������

#define MAX_INPUT_LEN 100   // �ִ����������
#define MAX_LINE_LEN 100     // �ʵ�һ�е���󳤶�
#define MAX_PATH_NUM 30     // ·�����������
#define MAX_WORD_LENGTH 25  // ����������������

#define ENCODE_NUM 2    //ʹ�õı����ʽ������ֽ�������GBKΪ2��UTF-8Ϊ4

#define abs_equal(a,b) (fabs(a-b)<1e-5? 1:0)    // �������ȽϺ���

typedef int Status;     // ����ֵ����ʾ״̬

string filename = "dict.txt";

struct character{       // �Զ������ݽṹ����ʾÿ���֣������ַ�������д洢
    char ch[ENCODE_NUM+1];
    character(){
        memset(ch, 0, ENCODE_NUM+1);
    }
};

int total_word_num;     // �ʱ��ܴ���
long long total_word_freq;  // �ʱ��ܴ�Ƶ
//---------˳���������--------
template <class T>      // ����˳���SqList�࣬����push����ÿ���ڱ�β���Ԫ��
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
        printf("�б���ʧ�ܣ������Ƿ����ôʵ��ļ�dict.txt��\n");
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
//--------˳��������ֽ���--------
//--------����ջ--------
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
//--------����ջ���ֽ���--------
//--------������Trie���Ķ�������ز���--------
struct TrieNode{        // �Զ���Trie���Ļ�����λ����Trie�����ö���ṹ
    character* word;    // ÿ���ڵ����������
    int freq;           // ���ýڵ���һ�������β��ʱ��freq��ʾ�ôʵĴ�Ƶ
    TrieNode* LNode;    // ���ӽڵ㣬��ʾ��һ����
    TrieNode* RNode;    // ���ӽڵ㣬��ʾ��ô�ͬ�����ֵܽڵ�
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
        int cn2en_cmp(character* a, char* b, int len);  // �Զ����ַ����ȽϺ���
	public:
	    TrieNode* root;
	    Trie();
	    Status Insert(char* s, int frequency);    // ��Trie������Ӵ���
	    int Search(string s);       // ����Trie�����Ƿ���ĳһ������
};

//--------Trie����ض������������--------
typedef struct{
    int pre;        // ָ��ýڵ����һ���ڵ�
    int pre_index;  // ��·�������
    int dis_index;  // ����·�����ܾ���
    double all_dis; // ��·���ڵ���õ������·���е�����
}PreNode;   // PreNode�ڵ��¼·�������Ϣ

int path_id = 0;    //·�����
SqList<PreNode>* Nodelist[MAX_INPUT_LEN];   //ÿ���ڵ�·�������Ϣ��
int trajectory[MAX_PATH_NUM][MAX_INPUT_LEN]={0};    //����·��

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
        cout<<"�ʵ乹����ɣ�"<<endl;
    }else{
        cout<<"�ʵ乹��ʧ�ܣ�"<<endl;
        return 0;
    }
    string respond = "y";
    while(true){
        char input_str[3*MAX_INPUT_LEN+1];
        cout<<"��������ִʵ��������:"<<endl;
        cout<<">>>";
        cin>>input_str;
        SqList<character*> words;
        char2character(input_str, words);   // �������ַ���ת��Ϊ���Զ��������λcharacter���ɵ�˳������ں�������
        double WordMatrix[MAX_INPUT_LEN+1][MAX_INPUT_LEN+1]={0};    // �����޻�ͼ���ڽӾ����ʾ
        dag_construction(*word_dict,words,WordMatrix);      // ���������޻�ͼ
        for (int i = 0; i <= words.len; i++)    //��ʼ��
        {         
            Nodelist[i]=new SqList<PreNode>;
        }    
        int n;
        cout<<"������ϣ����õķִ�������(n��̷ִ�):"<<endl;
        cout<<">>>";
        cin>>n;
        get_prenode(words.len,WordMatrix,n);    // ����PreNode����·�������Ϣ
        n_shortest_path(n, words);  // �ҵ�n�����������·��
        for(int i = 0; i <= words.len; i++){    //�����ڴ�ռ��
            delete Nodelist[i];
        }
        path_id = 0;
        memset(trajectory, 0, sizeof(trajectory));
        int pro_end = 0;
        while(true){
            cout<<"�Ƿ����?[y/n]"<<endl;
            cout<<">>>";
            cin>>respond;
            if(respond == "n"){
                pro_end=1;
                break;
            }else if(respond == "y"){
                break;
            }else{
                cout<<"�Ƿ����룡"<<endl;
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
        if(node_pt->LNode == nullptr){      // �������ӽڵ���õ�ǰ���ﴴ��һ��
            node_pt->LNode= new TrieNode(&s[i], len);
            node_pt=node_pt->LNode;
        }else{
            int cmp=0;
            node_pt = node_pt->LNode;
            while(!(cmp=cn2en_cmp(node_pt->word,&s[i], len))&&node_pt->RNode!=nullptr){
                node_pt=node_pt->RNode;             // ����Ѱ���Ƿ���ƥ��Ľڵ�
            }
            if(!cmp&&node_pt->RNode==nullptr){
                node_pt->RNode= new TrieNode(&s[i], len);   // �Ҳ���ƥ��Ľڵ�������ұߴ���һ���½ڵ㴢�浱ǰ����
                node_pt=node_pt->RNode;
            }
        }
        i+=len;
    }
    if(node_pt->freq!=0){
        cout<<"���� \""<<s<<"\" �Ѵ��ڣ�"<<endl;
        if(frequency>node_pt->freq){
            node_pt->freq= frequency;
        }
        cout<<"ʹ��Ƶ�� \""<<node_pt->freq<<"\" !"<<endl;
        return ERROR;
    }else{
        node_pt->freq = frequency; //�ڴ�β����Ǹôʵ�Ƶ�� 
        return OK;
    }
}
int Trie::Search(string str)//����ҵ����ͷ��ش�Ƶ�������ǰ׺���ͷ���-1
{
    TrieNode* node_pt = this->root;
    int size = str.length();
    char s[ENCODE_NUM*MAX_INPUT_LEN+1];
    strcpy(s,str.c_str());
    for(int i = 0; i < size;){
        int len = s[i]&0x80?ENCODE_NUM:1;   // �����ַ�ΪӢ�ĵ���� 
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
    // ���ܲ����ڸõ���,ֻ��һ��ǰ׺ ����ʱ����-1
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

//--------�����ʵ�--------
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
    char a[4] = {'|','/','-','\\'};	//�洢��ص��ַ�
    while (end!=start) {
        // ÿ�ζ�ȡ�ʵ��һ�У�������line��
        char line[MAX_LINE_LEN];
        fgets(line, MAX_LINE_LEN, dict);

        int freq;
        char word[2*MAX_WORD_LENGTH+1];
        sscanf(line, "%s%d%*s",&word, &freq);
        if(tree.Insert(word, freq)==ERROR){// ���ôʺ�Ƶ���ӵ�Trie����
            cout<<"�����ⷢ���ڵ�\""<<line_id<<"\"��"<<endl;
        };
        total_word_freq += freq;    // ����һ�еĴ����Ƶ���ӵ��ʵ���Ƶ����
        total_word_num++;   // ����������1
        start = ftell(dict);
        line_id++;
        if(line_id % 100 == 0){
            double percentage = (double)line_id/(double)line_num;
            cout << " [";	//��������߿�
            int remain;     // ��������ʣ�ಿ��
		    for (remain = 0;remain < 50*percentage;remain++) {	//���Ƽ��ؽ��ȵ���ʾ
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | BACKGROUND_INTENSITY);	//���ü�������ʽ����
                cout << "_";
            }
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY);	//�ָ�ԭ����
            for (int j = 0;j < 50-remain;j++) { //δ���ؽ�����ʾ
                printf("%c", '.'); 
            }	
            cout << "] ";	//�������ұ߿�
            printf("%3.0lf\%\t",100*percentage);
            int t = (line_id/100)%4;
            cout << a[t] << "�ֵ������";	//��ʾ�ͳ����������ʾ�͡����ڼ��ء�������ʾ
            for (int j = 0;j < t; j++){ //ʵ�ֶ�̬��ʾ
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
//--------�����ʵ����--------
//--------����DAGͼ��ҪԪ��-------
Status char2character(char* s, SqList<character*> &S){
    int len = strlen(s);
    for(int i=0; i < len;){
        character* l = new character;
        int char_len = s[i]&0x80?ENCODE_NUM:1;  // ÿ�����ִ�����һ��character�У�ÿ��������ENCODE_NUM���ֽڱ�ʾ
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
//--------DAGͼ��ҪԪ�ض������--------
//--------���ôʵ����ݹ���DAGͼ--------
Status dag_construction(Trie &tree, SqList<character*> &words, double WordMatrix[MAX_INPUT_LEN+1][MAX_INPUT_LEN+1]){
    int len = words.len;
    for(int i = 0; i < len; i++){
        string temp;
        int len_j = len-i;
        for(int j = 0;j<len_j;j++){
            temp = temp + words.elem[i+j]->ch;
            int res = tree.Search(temp);
            if(res > 0){
                WordMatrix[i][i+j+1]=res;   // ��Ƶ���������ڽӾ�����
            }else if(res == 0){
                break;  // ���δ�ҵ�������ѭ��
            }           // �����ǰ׺�ͼ�����
        }
        if(i!=len-1&&temp.length()==strlen(words.elem[i]->ch)){
            WordMatrix[i][i+1]=-1;  //���ĳ�������ڴʵ��в����ڣ��ͼ�����ڲ�����Ϊ-1
        }
        if(abs_equal(WordMatrix[len-1][len],0)){
            WordMatrix[len-1][len] = -1;    // �����һ���ֺ������һ�������ֱ���������
        }
        // ��ÿ���ʵ�Ƶ�����д�������Ϊ�ô��ڴʵ��г��ֵ�
        // ���ʵĸ���Ȼ���������м������ʱ����������˹Add-oneƽ��
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
//--------���ôʵ����ݹ���DAGͼ����--------

void get_prenode(int input_len, double WordMatrix[MAX_INPUT_LEN+1][MAX_INPUT_LEN+1], int n)//ע��һ��
{    
    int cur_index = 0;          // ��ǰ�ڵ�  
    Nodelist[0]->push({-1,0, 0, 0});
    while (cur_index <= input_len){     
        qsort(Nodelist[cur_index]->elem,Nodelist[cur_index]->len, sizeof(Nodelist[cur_index]->elem[0]),Node_sort_by_dis);
        get_index(*Nodelist[cur_index]);                // �Ե�ǰ�ڵ������·������������ 
        for (int i = cur_index+1; i <= input_len; i++)  // ����DAGͼ�ϵġ��ߡ�������ͬ�Ĵ���
        {            
            if (WordMatrix[cur_index][i]<INFINITY){  
                int last_id = 0;
                for(int t = 0 ; t < Nodelist[cur_index]->len&&last_id<=n; t++){
                    if(Nodelist[cur_index]->elem[t].dis_index==last_id){    // ��һ��·����β�ڵ��ϴ�����ͬ����·����Ŀ�ĵ�
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
    int id = 1; //id��1��ʼ
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
//--------PreNode�������--------
//--------����n���·��--------
void n_shortest_path(int n, SqList<character*> &words){
    int input_len = words.len;
    Stack<int> temp_stack;
    int path_num[MAX_PATH_NUM]={0};
    int index_num = 0;
    for(int i = 1; i <= n; i++){
        int base = -1;
        temp_stack.Push(base);
        int res = retro_back(input_len, i, temp_stack); // ���õݹ�Ӻ���ǰ���в���
        if(res == ERROR){
            cout<<"���ֻ��"<<i-1<<"�ִַʷ�����"<<endl;
            break;
        }else{
            path_num[i]=path_id-path_num[0];
            path_num[0]=path_id;
            index_num++;
        }
        temp_stack.Pop(base);
    }
    cout<<"���\t�ִʽ��"<<endl;
    int path_id_out = 0;
    for(int i = 1 ; i <= index_num;i++){
        cout<<setw(5)<<left<<i<<"\t";
        for(int j = 0; j < path_num[i];j++){
            int sep = 1;
            for(int k = 0 ; k < words.len; k++){
                if(k == trajectory[path_id_out][sep]){  // ������ǰ��ӡб��
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
        while(temp_stack.Pop(t)==OK){   // ջ�е����ּ�Ϊ·�ߣ���trajectory����
            trajectory[path_id][t1] = t;
            t1++;
        }
        while(t1>0){
            temp_stack.Push(trajectory[path_id][--t1]); // �ٽ������е����Żأ����ڵݹ�����
        }
        path_id++;
        return OK;
    }else{
        int last = Nodelist[cur_node]->len-1;
        int id_last = Nodelist[cur_node]->elem[last].dis_index;
        if(id_last<dis_index){
            return ERROR;   // ·����Ŀ����n��
        }
        for(int i = 0; i < Nodelist[cur_node]->len; i++){
            if(Nodelist[cur_node]->elem[i].dis_index==dis_index){   // ��������ŵ�ǰ��ڵ���ջ
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
//--------n���·�����غ�������--------