#include <windows.h>
#include <iostream>
#include "SChain.h"
#include "SString.h"
#include "SToken.h"
#include "SBinCode.h"

using namespace std;

//常量

//全局变量
struct ParamInfo//参数信息
{
	enum{PARAM_LABEL=2,PARAM_STRING=1,PARAM_INTEGER=0};
	int nType;//类型
};
struct OptInfo//操作符信息
{
	SString strName;
	SChain<ParamInfo> ParamChain;//参数链
};
struct LabelInfo//标签信息
{
	SString strName;
	int nAdderss;//标签地址
};
struct LabelBackfillInfo//标签回填信息
{
	SString strName;
	SChain<int> AddressChain;//要回填的地址链
};
SChain<LabelBackfillInfo>			g_LabelBackfillChain;
SChain<LabelInfo>					g_LabelChain;//标签链
SChain<OptInfo>						g_OptChain;//操作符链
int									g_CurOptID;//当前操作符ID
char								*g_strText;
FILE								*g_DestFile;
SToken								g_Token;

//子程式声明
void								ReadSettings();//读取设置
void								Compile(const char *strSrcFile,const char *strDestFile);//编译
void								Help();//帮助
void								LabelBackfill();//标签地址回填
void								PrintOptID();//打印操作符ID
//脚本文件递归下降分析
void								Program();
//设置文件递归下降分析
void								sProgram();

//=========================================================
//函数：Main
//说明：主函数
//输入：
//输出：
//返回：
//日期：2013-8-14
//备注：
//=========================================================
int main(int argc,char *argv[])
{
	cout<<"Script v1.0"<<endl;
	ReadSettings();

	if(argc!=3)
	{
		Help();
		PrintOptID();
	}
	else
	{
		cout<<"编译中......"<<endl;
		Compile(argv[1],argv[2]);
		cout<<"编译成功！"<<endl;
	}

	return 0;
}

//=========================================================
//函数：PrintOptID
//说明：打印操作符ID
//输入：
//输出：
//返回：
//日期：2013-8-14
//备注：
//=========================================================
void PrintOptID()
{
	int i;

	cout<<"所有操作符："<<endl;
	for(i=0;i<g_OptChain.GetCount();i++)
		cout<<g_OptChain.Get(i)->strName.c_str()<<"\t\t\t"<<i<<endl;
}

//=========================================================
//函数：Compile
//说明：编译
//输入：
//输出：
//返回：
//日期：2013-8-14
//备注：
//=========================================================
void Compile(const char *strSrcFile,const char *strDestFile)
{
	FILE *fp;
	int nLength;
	char *pText;
	SString strToken;
	SString strOpt;

	fp=fopen(strSrcFile,"rb");
	if(!fp)
	{
		cout<<"打开文件"<<strSrcFile<<"失败！"<<endl;
		exit(0);
	}
	fseek(fp,0,SEEK_END);
	nLength=ftell(fp);
	fseek(fp,0,SEEK_SET);
	pText=new char[nLength+1];
	fread(pText,nLength,1,fp);
	pText[nLength]='\0';

	g_Token.SetText(pText);

	delete []pText;
	fclose(fp);

	g_DestFile=fopen(strDestFile,"wb");

	if(!g_DestFile)
	{
		cout<<"创建文件"<<strDestFile<<"失败！"<<endl;
		exit(1);
	}
	g_LabelChain.Clear();

	Program();

	LabelBackfill();
	
	fclose(g_DestFile);
}

//=========================================================
//函数：LabelBackfill
//说明：标签地址回填
//输入：
//输出：
//返回：
//日期：2013-8-15
//备注：
//=========================================================
void LabelBackfill()
{
	int i,j,k;
	int nCount;
	int nLabelCount;
	int nLength;
	int nAddress;
	bool bFind;
	SString strName;
	LabelInfo *plinfo;
	LabelBackfillInfo *pbinfo;

	nCount=g_LabelBackfillChain.GetCount();
	nLabelCount=g_LabelChain.GetCount();

	for(i=0;i<nCount;i++)
	{
		pbinfo=g_LabelBackfillChain.Get(i);
		strName=pbinfo->strName;
		nLength=pbinfo->AddressChain.GetCount();
		bFind=false;
		for(j=0;j<g_LabelChain.GetCount();j++)
		{
			if(strName==g_LabelChain.Get(j)->strName)//找到
			{
				plinfo=g_LabelChain.Get(j);
				for(k=0;k<nLength;k++)
				{
					nAddress=*pbinfo->AddressChain.Get(k);
					fseek(g_DestFile,nAddress,SEEK_SET);
					fwrite(&plinfo->nAdderss,sizeof(plinfo->nAdderss),1,g_DestFile);
				}
				bFind=true;
			}
		}
		if(!bFind)//没有找到
		{
			cout<<"错误：标签"<<strName.c_str()<<"未找到！"<<endl;
			exit(1);
		}
	}
}

//=========================================================
//函数：Comment
//说明：脚本文件递归下降分析
//输入：
//输出：
//返回：
//日期：2013-8-15
//备注：
//=========================================================
void Comment()
{
	g_Token.Token();

	if(g_Token.GetToken()=="@")
	{
		g_Token.SkipToLineEnd();
	}
	else
	{
		cout<<"错误：Line("<<g_Token.GetCurrentLineCount()<<
				"):非法注释"<<endl;
			exit(1);
	}
}

//=========================================================
//函数：ParameterList
//说明：脚本文件递归下降分析
//输入：
//输出：
//返回：
//日期：2013-8-15
//备注：
//=========================================================
void ParameterList()
{
	int i;
	int nNum;
	int nAddress;
	int nType;
	int nCount;//参数计数
	int nParamType;//参数类型
	SString strToken;
	LabelBackfillInfo labelbackfillinfo;
	OptInfo *pOpt;

	nCount=0;

	pOpt=g_OptChain.Get(g_CurOptID);
	do{
		nCount++;
		if(nCount>pOpt->ParamChain.GetCount())
		{
		cout<<"错误：Line("<<g_Token.GetCurrentLineCount()<<
				"):参数多余"<<endl;
			exit(1);
		}
		g_Token.Token();
		nType=g_Token.GetTokenType();
		strToken=g_Token.GetToken();

		nParamType=pOpt->ParamChain.Get(nCount-1)->nType;
	
		if(nType==SToken::TYPE_NUMBER && nParamType==ParamInfo::PARAM_INTEGER)//数字
		{
			nNum=atoi(strToken.c_str());
			fwrite(&nNum,sizeof(nNum),1,g_DestFile);
		}
		else if(nType==SToken::TYPE_STRING && nParamType==ParamInfo::PARAM_STRING)//字符串
		{
			fwrite(strToken.c_str(),strToken.GetLength()+1,1,g_DestFile);
		}
		else if(nType==SToken::TYPE_LABEL && nParamType==ParamInfo::PARAM_LABEL)//标签
		{
			nNum=0;
			strToken.ToUpper();
			//遍历标签回填链表
			for(i=0;i<g_LabelBackfillChain.GetCount();i++)
			{
				if(strToken==g_LabelBackfillChain.Get(i)->strName)//如果找到
				{
					nAddress=ftell(g_DestFile);
					g_LabelBackfillChain.Get(i)->AddressChain.Add(nAddress);//加入链表
					break;
				}
			}
			if(i==g_LabelBackfillChain.GetCount())//未找到
			{
				labelbackfillinfo.strName=strToken;
				labelbackfillinfo.AddressChain.Clear();
				nAddress=ftell(g_DestFile);
				labelbackfillinfo.AddressChain.Add(nAddress);
				g_LabelBackfillChain.Add(labelbackfillinfo);
			}
			fwrite(&nNum,sizeof(nNum),1,g_DestFile);
		}
		else
		{
			cout<<"错误：Line("<<g_Token.GetCurrentLineCount()<<
				"):不合法的参数"<<strToken.c_str()<<endl;
			exit(1);
		}
		g_Token.Token();
		nType=g_Token.GetTokenType();
		strToken=g_Token.GetToken();
	}while(strToken==",");

	if (nCount < pOpt->ParamChain.GetCount())
	{
		cout << "错误：Line(" << g_Token.GetCurrentLineCount() <<
			"):" <<pOpt->strName.c_str()<<"参数少"  << endl;
		exit(1);
	}

	//g_Token.GoBack();
	if (g_Token.GetTokenType() == SToken::TYPE_ENDLINE
		|| g_Token.GetTokenType() == SToken::TYPE_ENDTEXT)
	{
		g_Token.GoBack();
	}
}

//=========================================================
//函数：Statement
//说明：脚本文件递归下降分析
//输入：
//输出：
//返回：
//日期：2013-8-15
//备注：
//=========================================================
void Statement()
{
	int i;
	int nOptID;
	int nType;
	LabelInfo labelinfo;
	SString strToken;
	SString strSrcToken;//未大写化的标示符

	g_Token.Token();
	OptInfo optinfo;

	strToken=g_Token.GetToken();
	strSrcToken=g_Token.GetToken();
	strToken.ToUpper();
	nType=g_Token.GetTokenType();
	if(nType==SToken::TYPE_LABEL)//操作符
	{
		//遍历操作符链表
		nOptID=-1;
		for(i=0;i<g_OptChain.GetCount();i++)
		{
			if(strToken==g_OptChain.Get(i)->strName)
			{
				nOptID=i;
				g_CurOptID=nOptID;
				fwrite(&nOptID,sizeof(nOptID),1,g_DestFile);
			}
		}
		if(nOptID==-1)
		{
			g_Token.Token();
			if(g_Token.GetToken()==":")//标签
			{
				labelinfo.strName=strToken;
				labelinfo.nAdderss=ftell(g_DestFile);
				g_LabelChain.Add(labelinfo);
				
			}
			else{
				g_Token.GoBack();
				cout<<"错误：Line("<<g_Token.GetCurrentLineCount()<<
								"):"<<strSrcToken.c_str()<<"操作符未定义"<<endl;
						exit(1);
			}
		}
		g_Token.Token();
		if(g_Token.GetTokenType()==SToken::TYPE_NUMBER ||
			g_Token.GetTokenType()==SToken::TYPE_LABEL ||
			g_Token.GetTokenType()==SToken::TYPE_STRING)
		{
			g_Token.GoBack();
			ParameterList();
		}
		else if (nOptID!=-1 && g_OptChain.Get(nOptID)->ParamChain.GetCount())
		{
			cout << "错误：Line(" << g_Token.GetCurrentLineCount() <<
				"):" << strSrcToken.c_str() << "操作符有参数" << endl;
			exit(1);
		}
		else if (g_Token.GetToken()!="@")
		{
			g_Token.GoBack();
		}
	}
	else if(nType==SToken::TYPE_ENDLINE)//空行
	{
		g_Token.GoBack();
	}
	else if(nType==SToken::TYPE_ENDTEXT)
	{
		g_Token.GoBack();
	}
//	else
//	{
//		cout<<"错误：Line("<<g_Token.GetCurrentLineCount()<<
//				"):非法语句"<<endl;
//		exit(1);
//	}
	if(g_Token.GetToken()=="@")
	{
		g_Token.GoBack(1);
		Comment();
	}
}

//=========================================================
//函数：Program
//说明：脚本文件递归下降分析
//输入：
//输出：
//返回：
//日期：2013-8-15
//备注：
//=========================================================
void Program()
{
	g_Token.Token();
	g_Token.SetCur(0);

	while(g_Token.GetTokenType()!=SToken::TYPE_ENDTEXT)
	{
		Statement();
		g_Token.Token();
		if(g_Token.GetTokenType()!=SToken::TYPE_ENDLINE &&
			g_Token.GetTokenType()!=SToken::TYPE_ENDTEXT)
		{
			cout<<"错误：Line("<<g_Token.GetCurrentLineCount()<<
				"):非法语句"<<endl;
			exit(1);
		}
	}
}


//=========================================================
//函数：Help
//说明：帮助
//输入：
//输出：
//返回：
//日期：2013-8-14
//备注：
//=========================================================
void Help()
{
	cout<<"用法："<<endl;
	cout<<"Script SrcFile DestFile"<<endl;
	cout<<"使用前先设置Settings文件"<<endl;
}


//=========================================================
//函数：ReadSettings
//说明：读取设置
//输入：
//输出：
//返回：
//日期：2013-8-14
//备注：
//=========================================================
void ReadSettings()
{
	char *strText;
	char strPath[MAX_PATH];
	FILE *fp;
	int nLength;
	SString strToken;
	OptInfo OptInfo;//临时操作符信息


	GetModuleFileName(NULL,strPath,MAX_PATH);

	strcpy(&strPath[strlen(strPath)-3],"settings");

	fp=fopen(strPath,"rb");

	if(!fp)
	{
		cout<<"打开脚本配置文件失败！"<<endl;
		exit(1);
	}
	fseek(fp,0,SEEK_END);
	nLength=ftell(fp);
	fseek(fp,0,SEEK_SET);
	strText=new char[nLength+1];

	if(!strText)
	{
		printf("申请设置文本存储内存失败！");
		Help();
		exit(1);
	}
	fread(strText,nLength,1,fp);
	strText[nLength]='\0';
	g_Token.SetText(strText);

	delete []strText;

	fclose(fp);

	g_OptChain.Clear();

	sProgram();
}

//=========================================================
//函数：sComment
//说明：设置文件递归下降分析
//输入：
//输出：
//返回：
//日期：2013-8-15
//备注：
//=========================================================
void sComment()
{
	g_Token.Token();

	if(g_Token.GetToken()=="@")
	{
		g_Token.SkipToLineEnd();
	}
	else
	{
		cout<<"错误：Line("<<g_Token.GetCurrentLineCount()<<
				"):非法注释"<<endl;
		exit(1);
	}
}

//=========================================================
//函数：sParameterList
//说明：设置文件递归下降分析
//输入：
//输出：
//返回：
//日期：2013-8-15
//备注：
//=========================================================
void sParameterList()
{
	int nType;
	SString strToken;
	ParamInfo paraminfo;

	do{
		g_Token.Token();
		nType=g_Token.GetTokenType();
		strToken=g_Token.GetToken();
		strToken.ToUpper();
	
		if(strToken=="STRING")//字符串
		{
			paraminfo.nType=ParamInfo::PARAM_STRING;
			g_OptChain.Get(g_OptChain.GetCount()-1)->ParamChain.Add(paraminfo);
		}
		else if(strToken=="LABEL")//标签
		{
			paraminfo.nType=ParamInfo::PARAM_LABEL;
			g_OptChain.Get(g_OptChain.GetCount()-1)->ParamChain.Add(paraminfo);
		}
		else if(strToken=="INTEGER")//整型
		{
			paraminfo.nType=ParamInfo::PARAM_INTEGER;
			g_OptChain.Get(g_OptChain.GetCount()-1)->ParamChain.Add(paraminfo);
		}
		else
		{
			cout<<"错误：Line("<<g_Token.GetCurrentLineCount()<<
				"):不合法的类型符"<<endl;
			exit(1);
		}
		g_Token.Token();
		nType=g_Token.GetTokenType();
		strToken=g_Token.GetToken();

	}while(strToken==",");

	if (g_Token.GetTokenType() == SToken::TYPE_ENDLINE ||
		g_Token.GetTokenType() == SToken::TYPE_ENDTEXT)
	{
		g_Token.GoBack(1);
	}
}

//=========================================================
//函数：sStatement
//说明：设置文件递归下降分析
//输入：
//输出：
//返回：
//日期：2013-8-15
//备注：
//=========================================================
void sStatement()
{
	int nType;
	SString strToken;
	OptInfo optinfo;

	g_Token.Token();
	strToken=g_Token.GetToken();
	strToken.ToUpper();

	nType=g_Token.GetTokenType();

	if(nType==SToken::TYPE_LABEL)
	{
		optinfo.strName=strToken;
		g_OptChain.Add(optinfo);
		g_Token.Token();
		if(g_Token.GetTokenType()==SToken::TYPE_LABEL)
		{
			g_Token.GoBack();
			sParameterList();
		}
	}
	else if(nType==SToken::TYPE_ENDLINE)//空行
	{
		g_Token.GoBack();
	}
	else if(nType==SToken::TYPE_ENDTEXT)
	{
		g_Token.GoBack();
	}
//	else
//	{
//		cout<<"错误：Line("<<g_Token.GetCurrentLineCount()<<
//				"):非法语句"<<endl;
//		exit(1);
//	}
	if(g_Token.GetToken()=="@")
	{
		g_Token.GoBack(1);
		sComment();
	}
}

//=========================================================
//函数：sProgram
//说明：设置文件递归下降分析
//输入：
//输出：
//返回：
//日期：2013-8-15
//备注：
//=========================================================
void sProgram()
{
	g_Token.Token();
	g_Token.SetCur(0);

	while(g_Token.GetTokenType()!=SToken::TYPE_ENDTEXT)
	{
		sStatement();
		
		g_Token.Token();
		if(g_Token.GetTokenType()!=SToken::TYPE_ENDLINE &&
			g_Token.GetTokenType()!=SToken::TYPE_ENDTEXT)
		{
			cout<<"设置文件错误：Line("<<g_Token.GetCurrentLineCount()<<
				"):非法语句"<<endl;
			exit(1);
		}
	}
}
