#pragma once
/**
 * @Name    : RSA.h
 * @Date    : 2017-04-11-22.25.57
 * @Author  : Silenceneo (silenceneo_xw@163.com)
 * @Link    : http://www.cnblogs.com/Silenceneo-xw/
 * @Version : 2.0
 */

#ifndef __RSA_H__
#define __RSA_H__

#include <ostream>
#include "BigInteger.h"

struct RSAPair
{
	BigInteger n, e, d;
	RSAPair()
	{}

	RSAPair(BigInteger& n, BigInteger& e, BigInteger& d)
	{
		this->n = n;
		this->e = e;
		this->d = d;
	}

	std::string ToString()
	{
		std::string abc;
		abc += "n:" + n.toString() + "\n";
		abc += "e:" + e.toString() + "\n";
		abc += "d:" + d.toString() + "\n";
		return abc;
	}
};


class RSA {
public:
	RSA() {}
	RSA(const unsigned len) { RSAPair pair; CreatePair(len, pair); SetPair(pair); }    // ����len��ʼ������
	~RSA() {}

	void SetPair(RSAPair&); //���ù�Կ˽Կ
	void CreatePair(const unsigned, RSAPair&);//���ɹ�Կ˽Կ

	BigInteger encryptByPublic(const BigInteger&);    // ��Կ����
	BigInteger decryptByPrivate(const BigInteger&);// ˽Կ����

	// ������Ҫ��������ǩ��
	BigInteger encryptByPrivate(const BigInteger&);// ˽Կ����
	BigInteger decryptByPublic(const BigInteger&);    // ��Կ����
protected:
	friend std::ostream& operator << (std::ostream&, const RSA&);// ����������
private:
	BigInteger createOddNum(unsigned);// ����һ��������,����Ϊ�䳤��
	bool isPrime(const BigInteger&, const unsigned);// �ж��Ƿ�Ϊ����
	BigInteger createRandomSmaller(const BigInteger&);// �������һ����С����
	BigInteger createPrime(unsigned, const unsigned);// ����һ��������,����Ϊ�䳤��
public:
	BigInteger n, e;// ��Կ
private:
	BigInteger d;// ˽Կ
	BigInteger p, q;// ������p��q
	BigInteger eul;// n��ŷ������
};

#endif // __RSA_H__
