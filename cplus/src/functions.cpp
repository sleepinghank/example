//
// Created by hank on 2024/11/27.
//
#include <iostream>
#include <string>
#include "functions.h"

Complex operator+(const Complex& A, const Complex& B){
    return Complex(A.m_real + B.m_real, A.m_imag + B.m_imag);
}

Complex operator-(const Complex& A, const Complex& B){
    return Complex(A.m_real - B.m_real, A.m_imag - B.m_imag);
}

Complex operator*(const Complex& A, const Complex& B){
    return Complex(A.m_real * B.m_real - A.m_imag * B.m_imag, A.m_real * B.m_imag + A.m_imag * B.m_real);
}

Complex operator/(const Complex& A,const Complex & B){
    double real = (A.m_real * B.m_real + A.m_imag * B.m_imag) / (B.m_real * B.m_real + B.m_imag * B.m_imag);
    double imag = (A.m_imag * B.m_real - A.m_real * B.m_imag) / (B.m_real * B.m_real + B.m_imag * B.m_imag);
    return Complex(real,imag);
}

istream & operator>>(istream& in, Complex& A){
    in >> A.m_real >> A.m_imag;
    return in;
}
ostream & operator<<(ostream& out, Complex& A){
    out << A.m_real << " + " << A.m_imag << "i";
    return out;
}



template <typename T>
T max(T a,T b,T c);

// 模版引用 交换
template<typename T>
void Swap(T &a,T &b){
    T temp = a;
    a = b;
    b = temp;
}

template<typename T1, typename T2>
class Point{
public:
    Point(T1 x, T2 y):m_x(x),m_y(y){};
public:
    T1 getX() const;
    void setX(T1 x);
    T2 getY() const;
    void setY(T2 y);
private:
    T1 m_x;
    T2 m_y;
};

template<typename T1, typename T2>
T1 Point<T1, T2>::getX() const{
    return m_x;
}

template<typename T1,typename T2>
void Point<T1, T2>::setX(T1 x){
    m_x = x;
}

template<typename T1, typename T2>
T2 Point<T1, T2>::getY() const{
    return m_y;
}


template<typename T> T max(T a,T b,T c){
    T max = a;
    if(b > max){
        max = b;
    }
    if(c > max){
        max = c;
    }
    return max;
}


#include "exception"


#include <cstdlib>
#include <fstream>


using namespace std;

// 自定义的异常类型
class OutOfRange{
public:
    OutOfRange(): m_flag(1) {};
    OutOfRange(int len,int index):m_len(len),m_index(index),m_flag(2) {};
public:
    void what() const; // 获取具体的错误信息
private:
    int m_flag; // 不同的flag表示不同的错误
    int m_len;  // 当前数组长度
    int m_index; // 当前使用的数组下标
};
void OutOfRange::what() const{
    if(m_flag == 1){
        cout<< "Error:empty array,no elements to pop." << endl;
    } else if (m_flag == 2){
        cout << "Error: out of range ( array length " << m_len << ",access index "<< m_index<< ")" << endl;
    } else {
        cout << "Unknown exception." << endl;
    }
}

class Array{
private:
    int m_len; // 数组长度
    int m_capacity; // 当前的内存能容纳多少个元素
    int *m_p; // 内存纸质
private:
    static const int m_stepSize = 50; // 每次扩容的大小
public:
    Array(); // 构造函数
    ~Array(){free(m_p);}; // 释放对象
public:
    int operator[](int i) const; // 获取数组元素（读取）返回的是const
    int &operator[](int i){return m_p[i];} // 获取数组元素（写入） 返回的是引用，可以修改值
    Array & operator=(const Array &arr); // 重载赋值运算符
    int push(int ele); // 在末尾插入数组元素
    int pop(); // 在末尾删除数组元素
    int length() const {return m_len;}; // 获取数组长度
};

Array::Array(){
    m_p=(int*)malloc(sizeof(int)*m_stepSize);
    m_capacity = m_stepSize;
    m_len = 0;
}

int Array::operator[](int index) const{
    if (index < 0 || index >= m_len){
        throw OutOfRange(m_len,index);
    }
    return *(m_p + index);
}
Array &Array::operator=(const Array &arr){ // 重载赋值运算符
    if (this != &arr){ // 判读是否是给自己赋值
        this ->m_len = arr.m_len;
        free(this->m_p); // 释放原有的内存
        this->m_p = (int*)calloc(this->m_len,sizeof(int)); // 重新分配内存
        memcpy(this->m_p,arr.m_p,sizeof(int)*m_len); // 复制数组元素
    }
    return *this;
}


int Array::push(int ele){
    if(m_len >= m_capacity){  //如果容量不足就扩容
        m_capacity += m_stepSize;
        m_p = (int*)realloc( m_p, sizeof(int) * m_capacity );  //扩容
    }
    *(m_p + m_len) = ele;
    m_len++;
    return m_len-1;
}
int Array::pop(){
    if(m_len == 0){
        throw OutOfRange();  //抛出异常（创建一个匿名对象）
    }
    m_len--;
    return *(m_p + m_len);
}
//打印数组元素
void printArray(Array &arr){
    int len = arr.length();
    //判断数组是否为空
    if(len == 0){
        cout<<"Empty array! No elements to print."<<endl;
        return;
    }
    for(int i=0; i<len; i++){
        if(i == len-1){
            cout<<arr[i]<<endl;
        }else{
            cout<<arr[i]<<", ";
        }
    }
}




using namespace std;

class Student{
public:
    Student(string name ="", int age = 0, float score = 0.0f);
    Student(const Student &stu);
public:
    void display();
private:
    string m_name;
    int m_age;
    float m_score;
};

Student::Student(string name, int age, float score): m_name(name),m_age(age),m_score(score){};

Student::Student(const Student &stu) {
    m_name = stu.m_name;
    this->m_age = stu.m_age;
    this->m_score = stu.m_score;

    cout<<"Copy constructor called."<<endl;
}

void Student::display(){
    cout << m_name << "的年龄是"<< m_age << "成绩是" << m_score << endl;
}

int file_test(){
    int x,sum = 0;
    ifstream srcFile(R"(D:\Code\VScode\example\cplus\data\test.txt)",ios::in);
    if(!srcFile){
        cout << "Error opening file!" << endl;
        return 0;
    }
    ofstream destFile("out.txt",ios::out | ios::binary);
    if (!destFile){
        srcFile.close();
        cout << "Error opening file!" << endl;
        return 0;
    }
    while(srcFile >> x){
        sum += x;
        destFile << x << " ";
    }
    cout << "The sum is: " << sum << endl;
    destFile.close();
    srcFile.close();
    return 0;
}
class CStudent{
public:
    char szName[20];
    int age;
};
int read_write(){
    CStudent s{};
//    ofstream outFile("students.dat",ios::out|ios::binary);
//    while(cin >> s.szName >> s.age){
//        if(s.age == 0){
//            break;
//        }
//        outFile.write((char*)&s,sizeof(s));
//    }
//    outFile.close();

    ifstream inFile("students.dat",ios::in | ios::binary);
    if(!inFile){
        cout << "error" << endl;
        return 0;
    }
    while(inFile.read((char*)&s,sizeof(s))){
        cout << "read count:" << inFile.gcount() << endl;
        cout << s.szName << " " << s.age << endl;
    }
    inFile.close();
    return 0;
}