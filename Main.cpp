#include <windows.h>
#include <iostream>
#include "SChain.h"
#include "SString.h"
#include "SToken.h"
#include "SBinCode.h"

using namespace std;

//����

//ȫ�ֱ���
struct ParamInfo//������Ϣ
{
	enum{PARAM_LABEL=2,PARAM_STRING=1,PARAM_INTEGER=0};
	int nType;//����
};
struct OptInfo//��������Ϣ
{
	SString strName;
	SChain<ParamInfo> ParamChain;//������
};
struct LabelInfo//��ǩ��Ϣ
{
	SString strName;
	int nAdderss;//��ǩ��ַ
};
struct LabelBackfillInfo//��ǩ������Ϣ
{
	SString strName;
	SChain<int> AddressChain;//Ҫ����ĵ�ַ��
};
SChain<LabelBackfillInfo>			g_LabelBackfillChain;
SChain<LabelInfo>					g_LabelChain;//��ǩ��
SChain<OptInfo>						g_OptChain;//��������
int									g_CurOptID;//��ǰ������ID
char								*g_strText;
FILE								*g_DestFile;
SToken								g_Token;

//�ӳ�ʽ����
void								ReadSettings();//��ȡ����
void								Compile(const char *strSrcFile,const char *strDestFile);//����
void								Help();//����
void								LabelBackfill();//��ǩ��ַ����
void								PrintOptID();//��ӡ������ID
//�ű��ļ��ݹ��½�����
void								Program();
//�����ļ��ݹ��½�����
void								sProgram();

//=========================================================
//������Main
//˵����������
//���룺
//�����
//���أ�
//���ڣ�2013-8-14
//��ע��
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
		cout<<"������......"<<endl;
		Compile(argv[1],argv[2]);
		cout<<"����ɹ���"<<endl;
	}

	return 0;
}

//=========================================================
//������PrintOptID
//˵������ӡ������ID
//���룺
//�����
//���أ�
//���ڣ�2013-8-14
//��ע��
//=========================================================
void PrintOptID()
{
	int i;

	cout<<"���в�������"<<endl;
	for(i=0;i<g_OptChain.GetCount();i++)
		cout<<g_OptChain.Get(i)->strName.c_str()<<"\t\t\t"<<i<<endl;
}

//=========================================================
//������Compile
//˵��������
//���룺
//�����
//���أ�
//���ڣ�2013-8-14
//��ע��
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
		cout<<"���ļ�"<<strSrcFile<<"ʧ�ܣ�"<<endl;
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
		cout<<"�����ļ�"<<strDestFile<<"ʧ�ܣ�"<<endl;
		exit(1);
	}
	g_LabelChain.Clear();

	Program();

	LabelBackfill();
	
	fclose(g_DestFile);
}

//=========================================================
//������LabelBackfill
//˵������ǩ��ַ����
//���룺
//�����
//���أ�
//���ڣ�2013-8-15
//��ע��
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
			if(strName==g_LabelChain.Get(j)->strName)//�ҵ�
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
		if(!bFind)//û���ҵ�
		{
			cout<<"���󣺱�ǩ"<<strName.c_str()<<"δ�ҵ���"<<endl;
			exit(1);
		}
	}
}

//=========================================================
//������Comment
//˵�����ű��ļ��ݹ��½�����
//���룺
//�����
//���أ�
//���ڣ�2013-8-15
//��ע��
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
		cout<<"����Line("<<g_Token.GetCurrentLineCount()<<
				"):�Ƿ�ע��"<<endl;
			exit(1);
	}
}

//=========================================================
//������ParameterList
//˵�����ű��ļ��ݹ��½�����
//���룺
//�����
//���أ�
//���ڣ�2013-8-15
//��ע��
//=========================================================
void ParameterList()
{
	int i;
	int nNum;
	int nAddress;
	int nType;
	int nCount;//��������
	int nParamType;//��������
	SString strToken;
	LabelBackfillInfo labelbackfillinfo;
	OptInfo *pOpt;

	nCount=0;

	pOpt=g_OptChain.Get(g_CurOptID);
	do{
		nCount++;
		if(nCount>pOpt->ParamChain.GetCount())
		{
		cout<<"����Line("<<g_Token.GetCurrentLineCount()<<
				"):��������"<<endl;
			exit(1);
		}
		g_Token.Token();
		nType=g_Token.GetTokenType();
		strToken=g_Token.GetToken();

		nParamType=pOpt->ParamChain.Get(nCount-1)->nType;
	
		if(nType==SToken::TYPE_NUMBER && nParamType==ParamInfo::PARAM_INTEGER)//����
		{
			nNum=atoi(strToken.c_str());
			fwrite(&nNum,sizeof(nNum),1,g_DestFile);
		}
		else if(nType==SToken::TYPE_STRING && nParamType==ParamInfo::PARAM_STRING)//�ַ���
		{
			fwrite(strToken.c_str(),strToken.GetLength()+1,1,g_DestFile);
		}
		else if(nType==SToken::TYPE_LABEL && nParamType==ParamInfo::PARAM_LABEL)//��ǩ
		{
			nNum=0;
			strToken.ToUpper();
			//������ǩ��������
			for(i=0;i<g_LabelBackfillChain.GetCount();i++)
			{
				if(strToken==g_LabelBackfillChain.Get(i)->strName)//����ҵ�
				{
					nAddress=ftell(g_DestFile);
					g_LabelBackfillChain.Get(i)->AddressChain.Add(nAddress);//��������
					break;
				}
			}
			if(i==g_LabelBackfillChain.GetCount())//δ�ҵ�
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
			cout<<"����Line("<<g_Token.GetCurrentLineCount()<<
				"):���Ϸ��Ĳ���"<<strToken.c_str()<<endl;
			exit(1);
		}
		g_Token.Token();
		nType=g_Token.GetTokenType();
		strToken=g_Token.GetToken();
	}while(strToken==",");

	if (nCount < pOpt->ParamChain.GetCount())
	{
		cout << "����Line(" << g_Token.GetCurrentLineCount() <<
			"):" <<pOpt->strName.c_str()<<"������"  << endl;
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
//������Statement
//˵�����ű��ļ��ݹ��½�����
//���룺
//�����
//���أ�
//���ڣ�2013-8-15
//��ע��
//=========================================================
void Statement()
{
	int i;
	int nOptID;
	int nType;
	LabelInfo labelinfo;
	SString strToken;
	SString strSrcToken;//δ��д���ı�ʾ��

	g_Token.Token();
	OptInfo optinfo;

	strToken=g_Token.GetToken();
	strSrcToken=g_Token.GetToken();
	strToken.ToUpper();
	nType=g_Token.GetTokenType();
	if(nType==SToken::TYPE_LABEL)//������
	{
		//��������������
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
			if(g_Token.GetToken()==":")//��ǩ
			{
				labelinfo.strName=strToken;
				labelinfo.nAdderss=ftell(g_DestFile);
				g_LabelChain.Add(labelinfo);
				
			}
			else{
				g_Token.GoBack();
				cout<<"����Line("<<g_Token.GetCurrentLineCount()<<
								"):"<<strSrcToken.c_str()<<"������δ����"<<endl;
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
			cout << "����Line(" << g_Token.GetCurrentLineCount() <<
				"):" << strSrcToken.c_str() << "�������в���" << endl;
			exit(1);
		}
		else if (g_Token.GetToken()!="@")
		{
			g_Token.GoBack();
		}
	}
	else if(nType==SToken::TYPE_ENDLINE)//����
	{
		g_Token.GoBack();
	}
	else if(nType==SToken::TYPE_ENDTEXT)
	{
		g_Token.GoBack();
	}
//	else
//	{
//		cout<<"����Line("<<g_Token.GetCurrentLineCount()<<
//				"):�Ƿ����"<<endl;
//		exit(1);
//	}
	if(g_Token.GetToken()=="@")
	{
		g_Token.GoBack(1);
		Comment();
	}
}

//=========================================================
//������Program
//˵�����ű��ļ��ݹ��½�����
//���룺
//�����
//���أ�
//���ڣ�2013-8-15
//��ע��
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
			cout<<"����Line("<<g_Token.GetCurrentLineCount()<<
				"):�Ƿ����"<<endl;
			exit(1);
		}
	}
}


//=========================================================
//������Help
//˵��������
//���룺
//�����
//���أ�
//���ڣ�2013-8-14
//��ע��
//=========================================================
void Help()
{
	cout<<"�÷���"<<endl;
	cout<<"Script SrcFile DestFile"<<endl;
	cout<<"ʹ��ǰ������Settings�ļ�"<<endl;
}


//=========================================================
//������ReadSettings
//˵������ȡ����
//���룺
//�����
//���أ�
//���ڣ�2013-8-14
//��ע��
//=========================================================
void ReadSettings()
{
	char *strText;
	char strPath[MAX_PATH];
	FILE *fp;
	int nLength;
	SString strToken;
	OptInfo OptInfo;//��ʱ��������Ϣ


	GetModuleFileName(NULL,strPath,MAX_PATH);

	strcpy(&strPath[strlen(strPath)-3],"settings");

	fp=fopen(strPath,"rb");

	if(!fp)
	{
		cout<<"�򿪽ű������ļ�ʧ�ܣ�"<<endl;
		exit(1);
	}
	fseek(fp,0,SEEK_END);
	nLength=ftell(fp);
	fseek(fp,0,SEEK_SET);
	strText=new char[nLength+1];

	if(!strText)
	{
		printf("���������ı��洢�ڴ�ʧ�ܣ�");
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
//������sComment
//˵���������ļ��ݹ��½�����
//���룺
//�����
//���أ�
//���ڣ�2013-8-15
//��ע��
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
		cout<<"����Line("<<g_Token.GetCurrentLineCount()<<
				"):�Ƿ�ע��"<<endl;
		exit(1);
	}
}

//=========================================================
//������sParameterList
//˵���������ļ��ݹ��½�����
//���룺
//�����
//���أ�
//���ڣ�2013-8-15
//��ע��
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
	
		if(strToken=="STRING")//�ַ���
		{
			paraminfo.nType=ParamInfo::PARAM_STRING;
			g_OptChain.Get(g_OptChain.GetCount()-1)->ParamChain.Add(paraminfo);
		}
		else if(strToken=="LABEL")//��ǩ
		{
			paraminfo.nType=ParamInfo::PARAM_LABEL;
			g_OptChain.Get(g_OptChain.GetCount()-1)->ParamChain.Add(paraminfo);
		}
		else if(strToken=="INTEGER")//����
		{
			paraminfo.nType=ParamInfo::PARAM_INTEGER;
			g_OptChain.Get(g_OptChain.GetCount()-1)->ParamChain.Add(paraminfo);
		}
		else
		{
			cout<<"����Line("<<g_Token.GetCurrentLineCount()<<
				"):���Ϸ������ͷ�"<<endl;
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
//������sStatement
//˵���������ļ��ݹ��½�����
//���룺
//�����
//���أ�
//���ڣ�2013-8-15
//��ע��
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
	else if(nType==SToken::TYPE_ENDLINE)//����
	{
		g_Token.GoBack();
	}
	else if(nType==SToken::TYPE_ENDTEXT)
	{
		g_Token.GoBack();
	}
//	else
//	{
//		cout<<"����Line("<<g_Token.GetCurrentLineCount()<<
//				"):�Ƿ����"<<endl;
//		exit(1);
//	}
	if(g_Token.GetToken()=="@")
	{
		g_Token.GoBack(1);
		sComment();
	}
}

//=========================================================
//������sProgram
//˵���������ļ��ݹ��½�����
//���룺
//�����
//���أ�
//���ڣ�2013-8-15
//��ע��
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
			cout<<"�����ļ�����Line("<<g_Token.GetCurrentLineCount()<<
				"):�Ƿ����"<<endl;
			exit(1);
		}
	}
}
