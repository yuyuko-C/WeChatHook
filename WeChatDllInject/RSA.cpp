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
/// 生成RSA的公钥私钥对
/// </summary>
/// <param name="len">表示大素数的二进制位数</param>
/// <param name="rsa">引用结构体,用于传参</param>
void RSA::CreatePair(const unsigned len, RSAPair& rsa)
{
	//更新随机种子
	srand((unsigned)time(NULL));
	// 产生大素数p和q
	BigInteger p = createPrime(len, 15);// 出错概率为(1/4)^15
	BigInteger q = createPrime(len, 15);
	// 计算出n的欧拉函数
	BigInteger eul = (p - 1) * (q - 1);
	// 计算出n
	rsa.n = p * q;
	// 根据提供的欧拉数生成公钥、私钥指数
	rsa.e = 65537;
	rsa.d = rsa.e.modInverse(eul);
}

/**
 * 函数功能:使用公钥进行加密
 * 参数含义:m表示要加密的明文
 */
BigInteger RSA::encryptByPublic(const BigInteger& m) {
	return m.modPow(e, n);
}

/**
 * 函数功能:使用私钥进行解密
 * 参数含义:c表示要解密的密文
 */
BigInteger RSA::decryptByPrivate(const BigInteger& c) {
	return c.modPow(d, n);
}

/**
 * 函数功能:使用私钥进行加密
 * 参数含义:m表示要加密的明文
 */
BigInteger RSA::encryptByPrivate(const BigInteger& m) {
	return decryptByPrivate(m);
}

/**
 * 函数功能:使用公钥进行解密
 * 参数含义:c表示要解密的密文
 */
BigInteger RSA::decryptByPublic(const BigInteger& c) {
	return encryptByPublic(c);
}

/**
 * 函数功能:输出RSA相关数据
 * 参数含义:out表示输出流,rsa表示要输出的RSA对象
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
 * 函数功能:生成一个长度为len的奇数
 * 参数含义:len代表奇数的二进制长度
 */
BigInteger RSA::createOddNum(unsigned len) {
	static const char hex_table[] = { '0', '1', '2', '3', '4', '5', '6', '7',
									'8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	len >>= 2;    // 十六进制数据,每位占4位二进制
	if (len) {
		std::ostringstream oss;
		for (size_t i = 0; i < len - 1; ++i)
			oss << hex_table[rand() % 16];
		oss << hex_table[1];// 最后一位为奇数
		return BigInteger(oss.str());
	}
	return BigInteger("F");
}

/**
 * 函数功能:判断一个数是否为素数,采用米勒拉宾大素数检测算法,失误率为(1/4)^k
 * 参数含义:num代表要判定的数,k代表测试次数
 */
bool RSA::isPrime(const BigInteger& num, const unsigned k) {
	assert(num != BigInteger::ZERO);// 测试num是否为0
	if (num == BigInteger::ONE)
		return false;    // 1不是素数
	if (num == BigInteger::TWO)
		return true;    // 2是素数

	BigInteger t = num - 1;
	BigInteger::bit b(t);// 二进制数
	if (b.at(0) == 1)    // 减一之后为奇数,原数为偶数
		return false;
	// num-1 = 2^s*d
	size_t s = 0;    // 统计二进制末尾有几个0
	BigInteger d(t);
	for (size_t i = 0; i < b.size(); ++i) {
		if (!b.at(i)) {
			++s;
			d = d.shiftRight(1);// 计算出d
		}
		else
			break;
	}

	for (size_t i = 0; i < k; ++i) {// 测试k次
		BigInteger a = createRandomSmaller(num);// 生成一个介于[1,num-1]之间的随机数a
		BigInteger x = a.modPow(d, num);
		if (x == BigInteger::ONE)// 可能为素数
			continue;
		bool ok = true;
		// 测试所有0<=j<s,a^(2^j*d) mod num != -1
		for (size_t j = 0; j < s && ok; ++j) {
			if (x == t)
				ok = false;    // 有一个相等,可能为素数
			x = x.multiply(x).mod(num);
		}
		if (ok)    // 确实都不等,一定为合数
			return false;
	}
	return true;    // 通过所有测试,可能为素数
}

/**
 * 函数功能:随机生成一个比val小的数
 * 参数含义:val代表比较的那个数
 */
BigInteger RSA::createRandomSmaller(const BigInteger& val) {
	BigInteger::base_t t = 0;
	do {
		t = rand();
	} while (t == 0);// 随机生成非0数

	BigInteger mod(t);
	BigInteger ans = mod % val;    // 比val要小
	if (ans == BigInteger::ZERO)// 必须非零
		ans = val - BigInteger::ONE;
	return ans;
}

/**
 * 函数功能:生成一个二进制长度为len的大素数
 * 参数含义:len代表大素数的长度,k代表素数检测的次数
 */
BigInteger RSA::createPrime(unsigned len, const unsigned k) {
	assert(k > 0);
	BigInteger ans = createOddNum(len);// 先生成一个奇数
	while (!isPrime(ans, k)) {// 素性检测
		ans = ans.add(BigInteger::TWO);// 下一个奇数
	}
	return ans;
}
