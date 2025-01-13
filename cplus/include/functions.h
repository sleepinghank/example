//
// Created by hank on 2024/11/27.
//

#ifndef CPLUS_FUNCTIONS_H
#define CPLUS_FUNCTIONS_H
using namespace std;

class Complex{
public:
    Complex(double real = 0.0,double imag = 0.0):m_real(real),m_imag(imag){};
    Complex(): m_real(0.0),m_imag(0.0){};
public:
    friend Complex operator+(const Complex& A, const Complex& B);
    friend Complex operator-(const Complex& A, const Complex& B);
    friend Complex operator*(const Complex& A, const Complex& B);
    friend Complex operator/(const Complex& A, const Complex& B);
    friend istream& operator>>(istream& in, Complex& A);
    friend ostream& operator<<(ostream& out, Complex& A);
    operator double() const { return m_real;} // 类型转换函数
private:
    double m_real;
    double m_imag;
};

#endif //CPLUS_FUNCTIONS_H
