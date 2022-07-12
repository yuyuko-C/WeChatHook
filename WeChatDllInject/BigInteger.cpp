/**
 * @Name    : BigInteger.cpp
 * @Date    : 2017-04-11-22.16.42
 * @Author  : Silenceneo (silenceneo_xw@163.com)
 * @Link    : http://www.cnblogs.com/Silenceneo-xw/
 * @Version : 2.0
 */

#include <algorithm>
#include <cassert>
#include <cctype>
#include "BigInteger.h"

 // 以下表示为静态常量赋值
const BigInteger BigInteger::ZERO = BigInteger(0);
const BigInteger BigInteger::ONE = BigInteger(1);
const BigInteger BigInteger::TWO = BigInteger(2);
const BigInteger BigInteger::TEN = BigInteger(10);

/**
 * 函数功能:根据给定的大整数构造一个新的大整数
 * 参数含义:val代表给定的大整数
 */
BigInteger::BigInteger(const BigInteger& val) {
	*this = val;
}

/**
 * 函数功能:根据给定的十六进制字符串数据构造一个大整数
 * 参数含义:str代表给定的数据
 */
BigInteger::BigInteger(const std::string& str) : is_negative(false) {
	std::string t(str);
	if (t.size() && t.at(0) == '-') {
		if (t.size() > 1)
			is_negative = true;
		t = t.substr(1);
	}
	int cnt = (8 - (t.size() % 8)) % 8;// 数的长度不是8的倍数,补足0
	std::string temp;

	for (int i = 0; i < cnt; ++i)
		temp.push_back('0');

	t = temp + t;

	for (size_t i = 0; i < t.size(); i += base_char) {
		base_t sum = 0;
		for (int j = 0; j < base_char; ++j) {    // 8位十六进制组成大整数的一位
			char ch = t[i + j];
			int num = hexToNum(ch);
			sum = ((sum << 4) | (num));
		}
		data.push_back(sum);
	}
	reverse(data.begin(), data.end());// 高位在后
	*this = trim();// 去除高位的0
}

/**
 * 函数功能:根据给定的long_t类型数据构造一个大整数
 * 参数含义:num代表给定的数据
 */
BigInteger::BigInteger(const long_t& num) : is_negative(false) {
	long_t t = num;
	if (t < 0) {
		is_negative = true;
		t = -t;
	}
	do {
		base_t temp = (t & base_num);    // 每次截取低32位
		data.push_back(temp);
		t >>= base_int;
	} while (t);
}

/**
 * 函数功能:大整数加法运算
 * 参数含义:val代表加数
 */
BigInteger BigInteger::add(const BigInteger& val) {
	BigInteger ans(*this);
	if (ans.is_negative == val.is_negative) {// 同号
		int len = val.data.size() - ans.data.size();

		while ((len--) > 0)    // 被加数位数少,高位补0
			ans.data.push_back(0);

		int carry = 0;    // 进位
		for (size_t i = 0; i < val.data.size(); ++i) {
			base_t temp = ans.data[i];
			ans.data[i] += val.data[i] + carry;    // 无符号数相加,超出取其余数
			// 进位:一种是有无进位都超出,一种是有进位才超出(比如十进制相加,9+9+1,得9,9+0+0,得9)
			carry = (temp > ans.data[i] ? 1 : (temp > (temp + val.data[i]) ? 1 : 0));
		}

		for (size_t i = val.data.size(); i < ans.data.size() && carry != 0; ++i) {// 还有进位
			base_t temp = ans.data[i];
			ans.data[i] += carry;
			carry = temp > ans.data[i];
		}

		if (carry)    // 还有进位
			ans.data.push_back(carry);
	}
	else {    // 异号
		BigInteger a = abs();
		BigInteger b = val.abs();
		int flag = a.compareTo(b);
		// 绝对值相等,则结果为0,否则用绝对值大的减去小的,符号随绝对值大的
		if (flag == -1) {
			ans = b.subtract(a);
			ans.is_negative = val.is_negative;
		}
		else if (flag == 0)
			ans = ZERO;
		else {
			ans = a.subtract(b);
			ans.is_negative = is_negative;
		}
	}
	return ans;
}

/**
 * 函数功能:大整数减法运算
 * 参数含义:val代表减数
 */
BigInteger BigInteger::subtract(const BigInteger& val) {
	BigInteger ans(*this);
	BigInteger a = abs();
	BigInteger b = val.abs();
	if (ans.is_negative == val.is_negative) {// 同号
		int flag = a.compareTo(b);
		if (flag == 1) {// a的绝对值大于b的绝对值,直接减
			int borrow = 0;    // 借位
			// 大数减小数
			for (size_t i = 0; i < val.data.size(); ++i) {
				base_t temp = ans.data[i];
				ans.data[i] -= val.data[i] + borrow;
				// 借位:一种是有无借位都超出,另一种是有借位才超出(比如十进制相减,9-0-0,得9,9-9-1,得9)
				borrow = temp < ans.data[i] ? 1 : (temp - borrow < val.data[i] ? 1 : 0);
			}
			for (size_t i = val.data.size(); i < ans.data.size() && borrow != 0; ++i) {// 还有借位
				base_t temp = ans.data[i];
				ans.data[i] -= borrow;
				borrow = temp < (base_t)borrow;
			}
			ans = ans.trim();// 去掉高位多余的0
		}
		else if (flag == 0)
			ans = ZERO;
		else {// a的绝对值小于b的绝对值
			ans = b.subtract(a);
			ans.is_negative = !is_negative;
		}
	}
	else {    // 异号
		ans = a.add(b);    // 转换为加法
		ans.is_negative = is_negative;
	}
	return ans;
}

/**
 * 函数功能:大整数乘法运算
 * 参数含义:val代表乘数
 */
BigInteger BigInteger::multiply(const BigInteger& val) const {
	if (equals(ZERO) || val.equals(ZERO))
		return ZERO;
	// 将位数少的作为乘数
	const BigInteger& big = data.size() > val.data.size() ? (*this) : val;
	const BigInteger& small = (&big) == (this) ? val : (*this);

	BigInteger ans;
	bit t(small);    // 转换为二进制进行运算

	for (int i = t.size() - 1; i >= 0; --i)
		if (t.at(i)) {
			BigInteger temp(big);
			temp.is_negative = false;
			temp = temp.shiftLeft(i);    // 移位对齐
			ans = ans.add(temp);
		}
	ans.is_negative = !(is_negative == val.is_negative);
	return ans;
}

/**
 * 函数功能:大整数整除运算
 * 参数含义:val代表除数
 */
BigInteger BigInteger::divide(const BigInteger& val) {
	BigInteger temp;
	BigInteger ans = divideAndRemainder(val, temp);
	return ans;
}

/**
 * 函数功能:大整数取余运算
 * 参数含义:val代表除数
 */
BigInteger BigInteger::remainder(const BigInteger& val) {
	BigInteger ans;
	divideAndRemainder(val, ans);
	return ans;
}

/**
 * 函数功能:大整数取模运算(不同于取余,该函数总是返回正余数)
 * 参数含义:m代表模数
 */
BigInteger BigInteger::mod(const BigInteger& m) {
	BigInteger ans = remainder(m);
	if (ans.is_negative)
		ans = ans.add(m);
	return ans;
}

/**
 * 函数功能:大整数整除运算和取余运算,整除结果直接返回,取余结果由m传回
 * 参数含义:val表示除数,m表示取余结果
 */
BigInteger BigInteger::divideAndRemainder(const BigInteger& val, BigInteger& m) {
	assert(!val.equals(ZERO));
	BigInteger a = abs();
	BigInteger b = val.abs();
	int flag = a.compareTo(b);
	if (flag == 0)// 绝对值相等
		return (is_negative == val.is_negative) ? BigInteger(1) : BigInteger(-1);
	if (flag == -1) {
		m = *this;
		return ZERO;
	}
	BigInteger ans;

	bit bit_b(b);
	// 位数对齐
	while (true) {// a的绝对值大于b的绝对值
		bit bit_a(a);
		int len = bit_a.size() - bit_b.size();
		BigInteger temp;
		// 找到移位
		while (len >= 0) {
			temp = b.shiftLeft(len);
			if (temp.compareTo(a) != 1)// 找到最大的左移位数使得当前的a大于等于b
				break;
			--len;
		}
		if (len < 0)    // 当前的a小于b了
			break;
		base_t num = 0;
		while (temp.compareTo(a) != 1) {
			a = a.subtract(temp);
			++num;    // 统计当前的a最多大于等于几个移位后的b
		}
		temp = BigInteger(num);
		if (len)
			temp = temp.shiftLeft(len);// 移位后表明当前的a是b的几倍
		ans = ans.add(temp);
	}
	ans.is_negative = !(is_negative == val.is_negative);
	m.data = a.data;
	m.is_negative = is_negative;
	return ans;
}

/**
 * 函数功能:大整数幂乘运算
 * 参数含义:exponent代表指数
 */
BigInteger BigInteger::pow(const BigInteger& exponent) {
	BigInteger ans(1);
	bit t(exponent);    // 转化为二进制,快速求幂
	for (int i = t.size() - 1; i >= 0; --i) {
		ans = ans.multiply(ans);
		if (t.at(i))
			ans = multiply(ans);// 从高位开始,位权累加效应
	}
	return ans;
}

/**
 * 函数功能:大整数模幂运算
 * 参数含义:exponent代表指数,m代表模数
 */
BigInteger BigInteger::modPow(const BigInteger& exponent, const BigInteger& m) const {
	assert(!m.equals(ZERO));
	BigInteger ans(1);
	bit t(exponent);
	for (int i = t.size() - 1; i >= 0; --i) {
		ans = ans.multiply(ans).mod(m);
		if (t.at(i))
			ans = multiply(ans).mod(m);
	}
	return ans;
}

/**
 * 函数功能:扩展欧几里得算法求乘法逆元
 * 参数含义:m代表求逆元时的模数
 */
BigInteger BigInteger::modInverse(const BigInteger& m) {
	assert(!is_negative);    // 当前大整数为正数
	assert(!m.is_negative);    // m为正数
	if (equals(ZERO) || m.equals(ZERO))
		return ZERO;    // 有一个数为0,就不存在乘法逆元
	BigInteger a[3], b[3], t[3];
	// 以下进行初等变换
	a[0] = 0; a[1] = 1; a[2] = *this;
	b[0] = 1; b[1] = 0; b[2] = m;

	for (t[2] = a[2].mod(b[2]); !t[2].equals(ZERO); t[2] = a[2].mod(b[2])) {
		BigInteger temp = a[2].divide(b[2]);
		for (int i = 0; i < 3; ++i) {
			t[i] = a[i].subtract(temp.multiply(b[i]));// 不超过一次a[2]-temp*b[2]就变为大数减小数
			a[i] = b[i];
			b[i] = t[i];
		}
	}
	if (b[2].equals(ONE)) {// 最大公约数为1,存在乘法逆元
		if (b[1].is_negative)// 逆元为负数
			b[1] = b[1].add(m);// 变为正数,使其在m的剩余集中
		return b[1];
	}
	return ZERO;// 最大公约数不为1,无乘法逆元
}

/**
 * 函数功能:移位运算,左移
 * 参数含义:len代表移位的位数
 */
BigInteger BigInteger::shiftLeft(const unsigned len) {
	int index = len >> base_bit;    // 大整数每一位需要移动多少位
	int shift = len & base_temp;    // 还剩下多少位
	BigInteger ans(*this);

	int inc = (shift == 0) ? index : index + 1;// 有多余的位要多开大整数的一位
	for (int i = 0; i < inc; ++i)
		ans.data.push_back(0);    // 高位补0

	if (index) {
		inc = (shift == 0) ? 1 : 2;// 有多余的位要预留一位
		for (int i = ans.data.size() - inc; i >= index; --i)
			ans.data[i] = ans.data[i - index];
		for (int i = 0; i < index; ++i)
			ans.data[i] = 0;
	}
	if (shift) {
		base_t t = base_num;
		t <<= base_int - shift;    // 用于截取高位
		// 左移
		base_t temp = 0;
		for (size_t i = 0; i < ans.data.size(); ++i) {
			base_t tmp = ans.data[i];
			ans.data[i] = (tmp << shift) | temp;// 左移后加上大整数低位的高位
			temp = (tmp & t) >> (base_int - shift);// 获取该大整数位的高位
		}
	}
	ans = ans.trim();
	return ans;
}

/**
 * 函数功能:移位运算,右移
 * 参数含义:len代表移位的位数
 */
BigInteger BigInteger::shiftRight(const unsigned len) {
	bit val(*this);
	if (len >= val.size())// 当前大整数位数小于等于移位位数,返回0
		return ZERO;
	int index = len >> base_bit;// 大整数每一位需要移动多少位
	int shift = len & base_temp;// 还剩下多少位
	BigInteger ans(*this);

	if (index) {
		for (int i = 0; i < index; ++i)
			ans.data[i] = ans.data[i + index];
		for (int i = 0; i < index; ++i)
			ans.data.pop_back();    // 高位删除
	}
	if (shift) {
		base_t t = base_num;
		t >>= base_int - shift;    // 用于截取低位
		// 右移
		base_t temp = 0;
		for (int i = ans.data.size() - 1; i >= 0; --i) {
			base_t tmp = ans.data[i];
			ans.data[i] = (tmp >> shift) | temp;// 右移后加上大整数高位的低位
			temp = (tmp & t) << (base_int - shift);// 获取该大整数位的低位
		}
	}
	ans = ans.trim();
	return ans;
}

/**
 * 函数功能:大整数比较函数,-1表示本大整数要小,0表示相等,1表示本大整数要大
 * 参数含义:val代表要与之比较的大整数
 */
int BigInteger::compareTo(const BigInteger& val) const {
	if (is_negative != val.is_negative) {// 符号不同,负数必小
		if (is_negative == true)
			return -1;
		return 1;
	}
	int flag = 0;
	if (data.size() < val.data.size())// 位数较小
		flag = -1;
	else if (data.size() > val.data.size())// 位数较大
		flag = 1;
	else {    // 位数相等,从高位开始一一比较
		for (std::vector<base_t>::const_reverse_iterator it = data.rbegin(), ite = val.data.rbegin(); it != data.rend(); ++it, ++ite)
			if ((*it) != (*ite)) {
				flag = (*it) < (*ite) ? -1 : 1;    // 高位小,则小
				break;
			}
	}
	if (is_negative)    // 如为负数,小的反而大
		flag = -flag;
	return flag;
}

/**
 * 函数功能:大整数是否相等函数
 * 参数含义:val表示要与之比较的大整数
 */
bool BigInteger::equals(const BigInteger& val) const {
	return (is_negative == val.is_negative) && (data == val.data);// 符号和数据都要相等
}

/**
 * 函数功能:将一个long_t类型的数据转换为大整数并返回
 * 参数含义:num表示给定的数
 */
BigInteger BigInteger::valueOf(const long_t& num) {
	return BigInteger(num);
}

/**
 * 函数功能:将大整数转换为十六进制字符串并返回
 */
std::string BigInteger::toString() const {
	std::string ans;
	base_t t = base_num;
	t <<= base_int - 4;    // 用于截取高4位
	for (int i = data.size() - 1; i >= 0; --i) {
		base_t temp = data[i];
		for (int j = 0; j < base_char; ++j) {
			base_t num = t & temp;// 每次截取高4位
			num >>= base_int - 4;    // 将高4位移到低4位
			temp <<= 4;
			if (num < 10)
				ans.push_back((char)('0' + num));
			else
				ans.push_back((char)('A' + num - 10));
		}
	}
	while (ans.size() > 0 && ans.at(0) == '0')// 去掉高位无用的0
		ans = ans.substr(1);
	if (ans.empty())    // 空串说明为0
		ans.push_back('0');
	if (is_negative)    // 为负数加上负号
		ans = "-" + ans;
	return ans;
}

/**
 * 函数功能:返回大整数的绝对值
 */
BigInteger BigInteger::abs() const {
	BigInteger ans;
	ans.data = data;// 只复制数据,符号默认为正
	return ans;
}

// 以下运算符重载函数主要是为了使得能使用
// 大整数类型像使用基本类型一样,不一一介绍
BigInteger operator + (const BigInteger& a, const BigInteger& b) {
	BigInteger t(a);
	return t.add(b);
}

BigInteger operator - (const BigInteger& a, const BigInteger& b) {
	BigInteger t(a);
	return t.subtract(b);
}

BigInteger operator * (const BigInteger& a, const BigInteger& b) {
	BigInteger t(a);
	return t.multiply(b);
}

BigInteger operator / (const BigInteger& a, const BigInteger& b) {
	BigInteger t(a);
	return t.divide(b);
}

BigInteger operator % (const BigInteger& a, const BigInteger& b) {
	BigInteger t(a);
	return t.remainder(b);
}

bool operator < (const BigInteger& a, const BigInteger& b) {
	return a.compareTo(b) == -1;
}

bool operator > (const BigInteger& a, const BigInteger& b) {
	return b < a;
}

bool operator == (const BigInteger& a, const BigInteger& b) {
	return a.equals(b);
}

bool operator <= (const BigInteger& a, const BigInteger& b) {
	return !(a > b);
}

bool operator >= (const BigInteger& a, const BigInteger& b) {
	return !(a < b);
}

bool operator != (const BigInteger& a, const BigInteger& b) {
	return !(a == b);
}

BigInteger operator + (const BigInteger& a, const BigInteger::long_t& b) {
	return a + BigInteger(b);
}

BigInteger operator - (const BigInteger& a, const BigInteger::long_t& b) {
	return a - BigInteger(b);
}

BigInteger operator * (const BigInteger& a, const BigInteger::long_t& b) {
	return a * BigInteger(b);
}

BigInteger operator / (const BigInteger& a, const BigInteger::long_t& b) {
	return a / BigInteger(b);
}

BigInteger operator % (const BigInteger& a, const BigInteger::long_t& b) {
	return a % BigInteger(b);
}

bool operator < (const BigInteger& a, const BigInteger::long_t& b) {
	return a < BigInteger(b);
}

bool operator > (const BigInteger& a, const BigInteger::long_t& b) {
	return a > BigInteger(b);
}

bool operator == (const BigInteger& a, const BigInteger::long_t& b) {
	return a == BigInteger(b);
}

bool operator <= (const BigInteger& a, const BigInteger::long_t& b) {
	return a <= BigInteger(b);
}

bool operator >= (const BigInteger& a, const BigInteger::long_t& b) {
	return a >= BigInteger(b);
}

bool operator != (const BigInteger& a, const BigInteger::long_t& b) {
	return a != BigInteger(b);
}

std::ostream& operator << (std::ostream& out, const BigInteger& val) {
	out << val.toString();
	return out;
}

/**
 * 函数功能:创建该大整数的一个副本,去除掉高位无用的0后并返回
 */
BigInteger BigInteger::trim() {
	size_t cnt = 0;
	// 检查高位为0的元素的数量
	for (std::vector<base_t>::const_reverse_iterator it = data.rbegin(); it != data.rend(); ++it) {
		if ((*it) == 0)
			++cnt;
		else
			break;
	}
	if (cnt == data.size() && cnt > 0)    // 只有零的情况保留
		--cnt;
	BigInteger ans(*this);
	for (size_t i = 0; i < cnt; ++i)
		ans.data.pop_back();
	return ans;
}

/**
 * 函数功能:根据给定的字符确定它所对应的十进制数
 * 参数含义:ch代表给定的字符
 */
int BigInteger::hexToNum(char ch) {
	int ans = 0;
	if (isdigit(ch))
		ans = ch - '0';
	else if (islower(ch))
		ans = ch - 'a' + 10;
	else
		ans = ch - 'A' + 10;
	return ans;
}

/**
 * 函数功能:根据给定的大整数初始化
 * 参数含义:val代表给定的大整数
 */
BigInteger::bit::bit(const BigInteger& val) {
	bit_vector = val.data;
	base_t temp = bit_vector[bit_vector.size() - 1];// 大整数最高位
	length = bit_vector.size() << base_bit;    // 大整数一位占二进制32位
	base_t t = 1 << (base_int - 1);    // 用于截取一个数的二进制最高位

	if (temp == 0)    // 大整数最高位为0,减去32
		length -= base_int;
	else {
		while (!(temp & t)) {// 从高位开始检测大整数的二进制位,为0长度减一
			--length;
			t >>= 1;    // 右移一位表示检测下一位
		}
	}
}

/**
 * 函数功能:检测大整数的第id位二进制位是否为1
 * 参数含义:id代表第id位
 */
bool BigInteger::bit::at(size_t id) {
	size_t index = id >> base_bit;// 确定其在大整数第几位
	size_t shift = id & base_temp;// 确定其在大整数那一位的二进制第几位
	base_t t = bit_vector[index];
	return (t & (1 << shift));
}