#include "inheritance.h"
#include <iostream>
#include <string>

using namespace std;

class People{
        public:
        void set_name(string name);
        void set_age(int age);
        string get_name();
        int get_age();
        private:
        string m_name;
        int m_age;
};
void People::set_name(string name){
    m_name = name;
}
void People::set_age(int age){
    m_age = age;
}
string People::get_name(){
    return m_name;
}

int People::get_age(){
    return m_age;
}

class Student: public People{
        public:
        void set_score(float score);
        float get_score();
        private:
        float m_score;
};

void Student::set_score(float score){
    m_score = score;
}
float Student::get_score(){
    return m_score;
}