#pragma once
#ifndef __BIGINTEGER_H__
#define __BIGINTEGER_H__

#include <string>
#include <vector>
#include <ostream>


class BigInteger
{

public:
	typedef long long long_t;
	typedef unsigned base_t;
	BigInteger() : is_negative(false) { data.push_back(0); }// Ĭ��Ϊ0
	BigInteger(const BigInteger&);    // ���ø����Ĵ�������ʼ��
	BigInteger(const std::string&);// ���ø�����ʮ�������ַ�����ʼ��
	BigInteger(const long_t&);        // ���ø�����long_t�������ݳ�ʼ��
	~BigInteger() {}

	BigInteger add(const BigInteger&);        // �������ӷ�
	BigInteger subtract(const BigInteger&);// ����������
	BigInteger multiply(const BigInteger&) const;// �������˷�
	BigInteger divide(const BigInteger&);    // ����������
	BigInteger remainder(const BigInteger&);    // ������ȡ��
	BigInteger mod(const BigInteger&);        // ������ȡģ
	BigInteger divideAndRemainder(const BigInteger&, BigInteger&);// ������������ȡ��
	BigInteger pow(const BigInteger&);        // �������ݳ�
	BigInteger modPow(const BigInteger&, const BigInteger&) const;// ��������ģ����
	BigInteger modInverse(const BigInteger&);// ����չŷ������㷨��˷���Ԫ

	BigInteger shiftLeft(const unsigned);    // ��λ����,����
	BigInteger shiftRight(const unsigned);    // ��λ����,����

	int compareTo(const BigInteger&) const;// �Ƚ�����
	bool equals(const BigInteger&) const;// �ж��Ƿ���ڸ�����
	static BigInteger valueOf(const long_t&);// ��������ת��Ϊ������������
	std::string toString() const;    // ��������ת��Ϊʮ�������ַ���
	BigInteger abs() const;        // ��������ľ���ֵ
protected:
	// ������������غ�����Ҫ�������������һ��ʹ�ô���������
	friend BigInteger operator + (const BigInteger&, const BigInteger&);
	friend BigInteger operator - (const BigInteger&, const BigInteger&);
	friend BigInteger operator * (const BigInteger&, const BigInteger&);
	friend BigInteger operator / (const BigInteger&, const BigInteger&);
	friend BigInteger operator % (const BigInteger&, const BigInteger&);
	friend bool operator < (const BigInteger&, const BigInteger&);
	friend bool operator > (const BigInteger&, const BigInteger&);
	friend bool operator == (const BigInteger&, const BigInteger&);
	friend bool operator <= (const BigInteger&, const BigInteger&);
	friend bool operator >= (const BigInteger&, const BigInteger&);
	friend bool operator != (const BigInteger&, const BigInteger&);

	// ���ذ汾,ʹ��������long_t����
	friend BigInteger operator + (const BigInteger&, const long_t&);
	friend BigInteger operator - (const BigInteger&, const long_t&);
	friend BigInteger operator * (const BigInteger&, const long_t&);
	friend BigInteger operator / (const BigInteger&, const long_t&);
	friend BigInteger operator % (const BigInteger&, const long_t&);
	friend bool operator < (const BigInteger&, const long_t&);
	friend bool operator > (const BigInteger&, const long_t&);
	friend bool operator == (const BigInteger&, const long_t&);
	friend bool operator <= (const BigInteger&, const long_t&);
	friend bool operator >= (const BigInteger&, const long_t&);
	friend bool operator != (const BigInteger&, const long_t&);

	friend std::ostream& operator << (std::ostream&, const BigInteger&);
	BigInteger operator = (const std::string& str) { return (*this) = BigInteger(str); }
	BigInteger operator = (const long_t& num) { return (*this) = BigInteger(num); }
private:
	BigInteger trim();    // ȥ����λ���õ�0
	int hexToNum(char);    // ʮ�������ַ�ת��Ϊʮ������
public:
	static const int base_bit = 5;    // 2^5=32,������ÿλ�洢�Ķ�����λ��
	static const int base_char = 8;    // ��ɴ�������һλ��Ҫ��ʮ������λ��
	static const int base_int = 32;    // ������һλ��Ӧ�Ķ�����λ��
	static const int base_num = 0xffffffff;// ��ȡ��λ�ĸ���
	static const int base_temp = 0x1f;    // ��ȡģ32�������ĸ���
	static const BigInteger ZERO;    // ����������0
	static const BigInteger ONE;    // ����������1
	static const BigInteger TWO;    // ����������2
	static const BigInteger TEN;    // ����������10
private:
	bool is_negative;// �Ƿ�Ϊ����
	std::vector<base_t> data;// ��λ���ݴ洢,��λ�ں�
	class bit {    // ���ڴ���������Ķ����ƴ�����
	public:
		bit(const BigInteger&);// ���ݴ�������ʼ��

		size_t size() { return length; }    // ���ش�������Ӧ�Ķ�����λ��
		bool at(size_t);    // ���ص�iλ�������Ƿ�Ϊ1
	private:
		std::vector<base_t> bit_vector;    // ���������ݴ洢,ÿһ��Ԫ�ض�Ӧ32λ������
		size_t length;    // �����Ƶ���λ��

	};
	friend class RSA;    // RSAΪ����Ԫ��
};

#endif // __BIGINTEGER_H__