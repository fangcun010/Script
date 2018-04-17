//=========================================================
//SToken
//标志类
//
//编写：方寸
//
//
//日期：2013-8-14
//=========================================================
#ifndef _S_TOKEN_
#define _S_TOKEN_

#include "SString.h"

//特殊字符定义
#define						CH_ENDLINE			10
#define						CH_ENDTEXT			0
#define						CH_TAB				9
#define						CH_SPACE			32
#define						CH_STRINGMARK		'\"'

class SToken
{
	//Data
private:
	char *m_pText;//文本
	int m_nCur;//当前位置指针
	int m_nTokenType;//标志类型
	int m_nLineCount;
	int m_nTextLength;
	bool m_bNegative;//负数
	SString m_strToken;
public:
	enum{TYPE_NONE,TYPE_LABEL,TYPE_SYMBOL,TYPE_NUMBER,TYPE_FLOAT,TYPE_STRING,TYPE_ENDLINE,TYPE_ENDTEXT,TYPE_ERROR};
	//Function
public:
	//=========================================================
	//函数：SToken
	//说明：构造函数
	//输入：
	//输出：
	//返回：
	//日期：2013-8-14
	//备注：
	//=========================================================
	SToken()
	{
		m_pText=NULL;
		m_nTokenType=TYPE_NONE;
		m_nCur=0;
		m_nTextLength=0;
		SetNegative(true);
	}

	//=========================================================
	//函数：~SToken
	//说明：解析函数
	//输入：
	//输出：
	//返回：
	//日期：2013-8-14
	//备注：
	//=========================================================
	~SToken()
	{
		if(m_pText)
			delete []m_pText;
	}
	//=========================================================
	//函数：Token
	//说明：从文本当前位置分析出一个标志
	//输入：
	//输出：
	//返回：
	//日期：2013-8-14
	//备注：
	//=========================================================
	SString &Token()
	{
		char *pch;
		char ReadChar;
		SString str;

		if(m_nCur>m_nTextLength)
		{
			m_strToken="";
			m_nTokenType=TYPE_ENDTEXT;
			return m_strToken;
		}

		SkipSpace();//跳过空白字符

		str="";

		pch=&m_pText[m_nCur];

		ReadChar=*pch;

		if(isdigit(ReadChar))//数字
		{
			do
			{
				str+=ReadChar;
				ReadChar=*(++pch);
				m_nCur++;
			}while(isdigit(ReadChar));

			if(isalpha(ReadChar))//数字后紧接字母
			{
				m_nTokenType=TYPE_ERROR;
				m_strToken="";
				return m_strToken;
			}

			if(ReadChar=='.')//数字后紧接小数点
			{
				do
				{
					str+=ReadChar;
					ReadChar=*(++pch);
					m_nCur++;
				}while(isdigit(ReadChar));

				m_nTokenType=TYPE_FLOAT;
				m_strToken=str;

				return m_strToken;
			}

			m_nTokenType=TYPE_NUMBER;
			m_strToken=str;
			return m_strToken;
		}
		else if(m_bNegative && ReadChar=='-')
		{
			ReadChar=*(++pch);
			m_nCur++;

			if(!isdigit(ReadChar))
			{
				m_nTokenType=TYPE_ERROR;
				m_strToken="";
				return m_strToken;
			}

			m_strToken="-";

			while(isdigit(ReadChar))
			{
				m_strToken+=ReadChar;
				ReadChar=*(++pch);
				m_nCur++;
			}

			if(ReadChar=='.')
			{
				m_nTokenType=TYPE_FLOAT;

				m_strToken+='.';
				ReadChar=*(++pch);
				m_nCur++;

				if(!isdigit(ReadChar))
				{
					m_strToken="";
					m_nTokenType=TYPE_ERROR;
					return m_strToken;
				}

				while(isdigit(ReadChar))
				{
					m_strToken+=ReadChar;
					ReadChar=*(++pch);
					m_nCur++;
				}
			}
			else
			{
				m_nTokenType=TYPE_NUMBER;
			}

			if(isalpha(ReadChar))
			{
				m_nTokenType=TYPE_ERROR;
				m_strToken="";
				return m_strToken;
			}

			return m_strToken;
		}
		else if(isalpha(ReadChar))//字母
		{
			do
			{
				str+=ReadChar;
				ReadChar=*(++pch);
				m_nCur++;
			}while(isalpha(ReadChar) || isdigit(ReadChar) || ReadChar=='_');
			m_nTokenType=TYPE_LABEL;
			m_strToken=str;
			return m_strToken;
		}
		else if(ReadChar==CH_STRINGMARK)//双引号
		{
			str = "";
			ReadChar=*(++pch);
			while (ReadChar != CH_STRINGMARK)
			{
				str+=ReadChar;
				ReadChar=*(++pch);
				m_nCur++;
				if(ReadChar==CH_ENDLINE || ReadChar==CH_ENDTEXT || ReadChar==13)//非法字符串字符
				{
					m_nTokenType=TYPE_ERROR;
					m_strToken="";
					
					return m_strToken;
				}
			}
			
			m_nTokenType=TYPE_STRING;
			m_nCur+=2;
			m_strToken=str;
			return m_strToken;
		}
		else if(ispunct(ReadChar))
		{
			m_nCur++;
			m_strToken="";
			m_strToken+=ReadChar;
			m_nTokenType=TYPE_SYMBOL;
			return m_strToken;
		}
		else if(ReadChar==CH_ENDLINE)//行末
		{
			m_nLineCount++;
			m_nCur++;
			m_nTokenType=TYPE_ENDLINE;
			m_strToken="";
			return m_strToken;
		}
		else if(ReadChar==13)//行末
		{
			pch++;
			m_nCur++;
			m_strToken="";
			if(*pch!=CH_ENDLINE)
			{
				m_nTokenType=TYPE_ERROR;
				return m_strToken;
			}
			m_nCur++;
			m_nTokenType=TYPE_ENDLINE;
			m_nLineCount++;
			
			return m_strToken;
		}
		else if(ReadChar==CH_ENDTEXT)
		{
			m_nCur++;
			m_nTokenType=TYPE_ENDTEXT;
			m_strToken="";
			return m_strToken;
		}
		else
		{
			m_nTokenType=TYPE_ERROR;
			m_strToken="";
			return m_strToken;
		}
	}
	//=========================================================
	//函数：GetCurrentLineCount
	//说明：当前指针所在行数
	//输入：
	//输出：
	//返回：
	//日期：2013-8-15
	//备注：
	//=========================================================
	int GetCurrentLineCount()
	{
		return m_nLineCount;
	}
	//=========================================================
	//函数：SetCur
	//说明：设置指针，但不会更改当前行数
	//输入：nPos:位置
	//输出：
	//返回：
	//日期：2013-8-15
	//备注：
	//=========================================================
	void SetCur(int nPos)
	{
		m_nCur=nPos;
	}
	//=========================================================
	//函数：GetCur
	//说明：获取指针，但不会更改当前行数
	//输入：
	//输出：
	//返回：
	//日期：2013-8-15
	//备注：
	//=========================================================
	long GetCur()
	{
		return m_nCur;
	}

	//=========================================================
	//函数：SetNegative
	//说明：设置是否将负数作为一个单位
	//输入：
	//输出：
	//返回：
	//日期：2013-8-15
	//备注：
	//=========================================================
	void SetNegative(bool bNegative)
	{
		m_bNegative=bNegative;
	}

	//=========================================================
	//函数：GoBack
	//说明：指针回退
	//输入：n：回退的长度
	//输出：
	//返回：
	//日期：2013-8-15
	//备注：
	//=========================================================
	void GoBack(int n)
	{
		char *pch;
		int i;

		pch=&m_pText[m_nCur];

		if(m_nCur==0) return;
		for(i=0;i<n;i++)
		{
			pch--;
			if(*pch=='\n')
				m_nLineCount--;
		}

		m_nCur-=n;
	}
	//=========================================================
	//函数：GoBack
	//说明：指针回退一个标志
	//输入：
	//输出：
	//返回：
	//日期：2013-8-15
	//备注：
	//=========================================================
	void GoBack()
	{
		if(m_nTokenType==TYPE_STRING)
			GoBack(m_strToken.GetLength()+2);
		else if(m_nTokenType==TYPE_NUMBER)
			GoBack(m_strToken.GetLength());
		else if(m_nTokenType==TYPE_SYMBOL)
			GoBack(1);
		else if(m_nTokenType==TYPE_FLOAT)
			GoBack(m_strToken.GetLength());
		else if(m_nTokenType==TYPE_LABEL)
			if(m_strToken.GetLength())
				GoBack(m_strToken.GetLength());
			else
				GoBack(1);
		else if(m_nTokenType==TYPE_ENDLINE || m_nTokenType==TYPE_ENDTEXT)
			GoBack(1);
		m_strToken="";
	}

	//=========================================================
	//函数：GetTokenType
	//说明：获取标志类型
	//输入：
	//输出：
	//返回：
	//日期：2013-8-14
	//备注：
	//=========================================================
	int GetTokenType()
	{
		return m_nTokenType;
	}

	//=========================================================
	//函数：GetToken
	//说明：获取标志字符串
	//输入：
	//输出：
	//返回：
	//日期：2013-8-14
	//备注：
	//=========================================================
	SString &GetToken()
	{
		return m_strToken;
	}
	//=========================================================
	//函数：SkipToLineEnd
	//说明：跳到行末
	//输入：
	//输出：
	//返回：
	//日期：2013-8-14
	//备注：
	//=========================================================
	void SkipToLineEnd()
	{
		char *pch;

		pch=&m_pText[m_nCur];

		while(*pch!=CH_ENDLINE && *pch!=CH_ENDTEXT)
		{
			pch++;
			m_nCur++;
		}

		if(*pch==CH_ENDLINE)
			m_nTokenType=TYPE_ENDLINE;
		else
			m_nTokenType=TYPE_ENDTEXT;
	}
	//=========================================================
	//函数：SetText
	//说明：设置文本
	//输入：
	//输出：
	//返回：
	//日期：2013-8-14
	//备注：
	//=========================================================
	void SetText(const char *str)
	{
		if(m_pText)
			delete []m_pText;
		m_nTextLength=strlen(str);
		m_pText=new char[m_nTextLength+1];
		strcpy(m_pText,str);
		m_nCur=0;//初始化位置指针
		m_nLineCount=1;
	}
	//=========================================================
	//函数：SetTokenType
	//说明：设置Token的类型
	//输入：
	//输出：
	//返回：
	//日期：2013-8-14
	//备注：
	//=========================================================
	void SetTokenType(int nType)
	{
		m_nTokenType=nType;
	}
	//=========================================================
	//函数：Clear
	//说明：清空
	//输入：
	//输出：
	//返回：
	//日期：2015-2-22
	//备注：
	//=========================================================
	void Clear()
	{
		if(m_pText)
			delete []m_pText;

		m_nTextLength=0;
		m_nTokenType=TYPE_NONE;
		m_pText=NULL;
	}
private:
	//=========================================================
	//函数：SkipSpace
	//说明：跳过空白字符
	//输入：
	//输出：
	//返回：
	//日期：2013-8-14
	//备注：
	//=========================================================
	void SkipSpace()
	{
		char *pch;

		pch=&m_pText[m_nCur];

		while(*pch==CH_SPACE || *pch==CH_TAB)
		{
			pch++;
			m_nCur++;
		}
	}
};

#endif
