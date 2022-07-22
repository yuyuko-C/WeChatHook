/**
 * @Name    : RSA.cpp
 * @Date    : 2017-04-11-22.27.51
 * @Author  : Silenceneo (silenceneo_xw@163.com)
 * @Link    : http://www.cnblogs.com/Silenceneo-xw/
 * @Version : 2.0
 */

#include <cassert>
#include <sstream>
#include <ctime>
#include "RSA.h"

void RSA::SetPair(RSAPair& rsa)
{
	n = rsa.n;
	e = rsa.e;
	d = rsa.d;
}

/// <summary>
/// ����RSA�Ĺ�Կ˽Կ��
/// </summary>
/// <param name="len">��ʾ�������Ķ�����λ��</param>
/// <param name="rsa">���ýṹ��,���ڴ���</param>
void RSA::CreatePair(const unsigned len, RSAPair& rsa)
{
	//�����������
	srand((unsigned)time(NULL));
	// ����������p��q
	BigInteger p = createPrime(len, 15);// �������Ϊ(1/4)^15
	BigInteger q = createPrime(len, 15);
	// �����n��ŷ������
	BigInteger eul = (p - 1) * (q - 1);
	// �����n
	rsa.n = p * q;
	// �����ṩ��ŷ�������ɹ�Կ��˽Կָ��
	rsa.e = 65537;
	rsa.d = rsa.e.modInverse(eul);
}

/**
 * ��������:ʹ�ù�Կ���м���
 * ��������:m��ʾҪ���ܵ�����
 */
BigInteger RSA::encryptByPublic(const BigInteger& m) {
	return m.modPow(e, n);
}

/**
 * ��������:ʹ��˽Կ���н���
 * ��������:c��ʾҪ���ܵ�����
 */
BigInteger RSA::decryptByPrivate(const BigInteger& c) {
	return c.modPow(d, n);
}

/**
 * ��������:ʹ��˽Կ���м���
 * ��������:m��ʾҪ���ܵ�����
 */
BigInteger RSA::encryptByPrivate(const BigInteger& m) {
	return decryptByPrivate(m);
}

/**
 * ��������:ʹ�ù�Կ���н���
 * ��������:c��ʾҪ���ܵ�����
 */
BigInteger RSA::decryptByPublic(const BigInteger& c) {
	return encryptByPublic(c);
}

/**
 * ��������:���RSA�������
 * ��������:out��ʾ�����,rsa��ʾҪ�����RSA����
 */
std::ostream& operator << (std::ostream& out, const RSA& rsa) {
	out << "n: " << rsa.n << "\n";
	out << "p: " << rsa.p << "\n";
	out << "q: " << rsa.q << "\n";
	out << "e: " << rsa.e << "\n";
	out << "d: " << rsa.d;
	return out;
}

/**
 * ��������:����һ������Ϊlen������
 * ��������:len���������Ķ����Ƴ���
 */
BigInteger RSA::createOddNum(unsigned len) {
	static const char hex_table[] = { '0', '1', '2', '3', '4', '5', '6', '7',
									'8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	len >>= 2;    // ʮ����������,ÿλռ4λ������
	if (len) {
		std::ostringstream oss;
		for (size_t i = 0; i < len - 1; ++i)
			oss << hex_table[rand() % 16];
		oss << hex_table[1];// ���һλΪ����
		return BigInteger(oss.str());
	}
	return BigInteger("F");
}

/**
 * ��������:�ж�һ�����Ƿ�Ϊ����,����������������������㷨,ʧ����Ϊ(1/4)^k
 * ��������:num����Ҫ�ж�����,k������Դ���
 */
bool RSA::isPrime(const BigInteger& num, const unsigned k) {
	assert(num != BigInteger::ZERO);// ����num�Ƿ�Ϊ0
	if (num == BigInteger::ONE)
		return false;    // 1��������
	if (num == BigInteger::TWO)
		return true;    // 2������

	BigInteger t = num - 1;
	BigInteger::bit b(t);// ��������
	if (b.at(0) == 1)    // ��һ֮��Ϊ����,ԭ��Ϊż��
		return false;
	// num-1 = 2^s*d
	size_t s = 0;    // ͳ�ƶ�����ĩβ�м���0
	BigInteger d(t);
	for (size_t i = 0; i < b.size(); ++i) {
		if (!b.at(i)) {
			++s;
			d = d.shiftRight(1);// �����d
		}
		else
			break;
	}

	for (size_t i = 0; i < k; ++i) {// ����k��
		BigInteger a = createRandomSmaller(num);// ����һ������[1,num-1]֮��������a
		BigInteger x = a.modPow(d, num);
		if (x == BigInteger::ONE)// ����Ϊ����
			continue;
		bool ok = true;
		// ��������0<=j<s,a^(2^j*d) mod num != -1
		for (size_t j = 0; j < s && ok; ++j) {
			if (x == t)
				ok = false;    // ��һ�����,����Ϊ����
			x = x.multiply(x).mod(num);
		}
		if (ok)    // ȷʵ������,һ��Ϊ����
			return false;
	}
	return true;    // ͨ�����в���,����Ϊ����
}

/**
 * ��������:�������һ����valС����
 * ��������:val����Ƚϵ��Ǹ���
 */
BigInteger RSA::createRandomSmaller(const BigInteger& val) {
	BigInteger::base_t t = 0;
	do {
		t = rand();
	} while (t == 0);// ������ɷ�0��

	BigInteger mod(t);
	BigInteger ans = mod % val;    // ��valҪС
	if (ans == BigInteger::ZERO)// �������
		ans = val - BigInteger::ONE;
	return ans;
}

/**
 * ��������:����һ�������Ƴ���Ϊlen�Ĵ�����
 * ��������:len����������ĳ���,k�����������Ĵ���
 */
BigInteger RSA::createPrime(unsigned len, const unsigned k) {
	assert(k > 0);
	BigInteger ans = createOddNum(len);// ������һ������
	while (!isPrime(ans, k)) {// ���Լ��
		ans = ans.add(BigInteger::TWO);// ��һ������
	}
	return ans;
}
