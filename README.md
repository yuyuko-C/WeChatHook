# WeChatHook
逆向微信的学习记录. 主要用于批量发送. 

上传到github仅用作学习记录, 任何利用此代码进行违法的行为与本人无关. 



### 实现原理

注入DLL建立socket服务端接受来自客户端的指令, 客户端通过socket操控服务端实现指令. 最开始是打算给dll做个界面实现, 但最后发现在Windows消息机制里无法在一个消息循环中让微信执行For循环多次发送消息. 最终使用socket解决问题.



### 找Call记录

* #### 确定当前接收人的微信id

  使用ce查找filehelper,切换当前窗口重复搜索目标缩小范围, 然后搜索wxid_进一步缩小范围. 获取某人的微信id记录下来. 最后使用二分法修改当前wxid确定最终目标.(UTF-16的字符串模式下搜索)

  

* #### 确定文字发送的关键call

  在od中对当前接收人的内存地址下内存断点, 此时会有多个断点, 记录下来并找到只有发送的时候才会断下的断点. 在发送断点时在堆栈窗口返回到EBP寄存器, EBP寄存器存储了当前函数的返回地址, 用于快速返回到上级. 层层跳出排查返回位置附近的call, 发送call至少需要有两个参数, 收信人的微信id与发送内容.

  

* #### 确定其他发送的关键call

  微信的发送方式分为三种, 文字, 图片 与文件. 其他两种就在文字call的下方. 其结构是一个switch跳转, 所以找到了文字call就找到了所有发送方式的call.

  ```c++
  void SendMessage(E_MessageType msgType, WeChatStr wxid, WeChatStr msg)
  {
  	switch(msgType)
      {
          case E_MessageType.Text:
              SendTextMessage(wxid,msg);
              break;
          case E_MessageType.Image:
              SendImageMessage(wxid,msg);
              break;
          case E_MessageType.File:
              SendFileMessage(wxid,msg);
              break;
      }
  }
  ```

  

* #### 写代码实现发送功能

  微信的几乎全部功能都在WeChatWin.dll中, 所以只需要找到一个模块基址即可.

  汇编中所有函数的地址都是临时地址, 是不稳定的, 只有模块地址在每一次运行中都保持一致. 因此汇编中的所有函数都需要与其基址相减获得偏移.

  ```c++
  DWORD moduleBaseAddr = (DWORD)LoadLibrary(L"WeChatWin.dll");
  DWORD sendTextCall = moduleBaseAddr + 0x54BB1D30 - 0x54690000;
  ```

  微信的字符串是一个结构体

  ```C++
  struct WeChatStr
  {
  	const WCHAR* pStr;
  	int len;
  	int maxLen;
  	int fill[2] = { 0 };
  };
  ```

  在此版本中, 如不添加fill这个数组则会内存的非法越界访问. 但值永远是0, 想必是专门为了增加逆向门槛设立的.

  

  最后, 构建参数, 执行汇编后即可实现对应的功能. 图片与文件都是连环Call(需要多个Call组合才能实现, 从汇编视角来看这两个越来越难看出来了)

  ```C++
  	// 发送消息的call
  	DWORD sendCall = moduleBaseAddr + 0x54BB1D30 - 0x54690000;
  
  	// 组装格式
  	WeChatStr id = { wxid.c_str(), (int)wxid.length(),(int)wxid.length() };
  	WeChatStr msg = { message.c_str(),(int)message.length(),(int)message.length() };
  
  	// 缓冲区
  	WCHAR at[0x38] = { 0 };
  	WCHAR buff[0x3B0] = { 0 };
  
  	__asm
  	{
  		pushad;
  
  		lea eax, at;
  		lea edi, msg;
  		push 0x1;
  		push eax;
  		push edi;
  		lea edx, id;
  		lea ecx, buff;
  		call sendCall;
  		add esp, 0xC;
  
  		popad;
  	}
  ```

  

### 经验之谈

* 文字发送call每个版本都有个push 0x1;
* 文字发送call每个版本都是外平栈, 最后都接着 add esp, 0xC;

