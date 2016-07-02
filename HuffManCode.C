//-----------------------------------------------------
// Coded By @�����˭
//-----------------------------------------------------
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<windows.h>
#define  GET_TREE  (1)
#define  GET_CODE  (0)
//ʹ������WIN32����
#define  hFree(hMem) HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE + HEAP_ZERO_MEMORY, hMem)
#define  hAlloc(bSize) HeapAlloc(GetProcessHeap(), HEAP_NO_SERIALIZE + HEAP_ZERO_MEMORY, bSize)
//��Ҫ�ṹ�Ľ��Ͷ���<compress.inc>�ļ���
struct HNODE{
	unsigned  weight;
	unsigned  isLeaf;
	unsigned char bValue;
	void*lpParent;
	void*lpLeft;
	void*lpRight;
};
struct HUFFCODE{
	unsigned char bValue;
	unsigned char bDepth;
	unsigned  bitVal;
};
struct SUITELEM{
	unsigned char sByte;
	unsigned  sFreq;
};
struct HUFFTABL{
	unsigned tablElems;
	struct SUITELEM elem[256];
};
//C����һ�η��ؼ���ֵ��ֻ���Զ���һ���ṹ����
struct POINTERS{     
	void*p1;        
	void*p2;
	void*p3;
};

int FindBiggest(unsigned*,void*);
struct POINTERS HuffManBuild(void*, int);
void* CreatHuffManTabl(void*, unsigned);
int FindSmallest(unsigned*, unsigned);
//unsigned Compress(void*, void*, unsigned);
void FailAlloc(void);
void RCR(unsigned*, unsigned);
/*
 *        ��Ϊ�������Ѱַ�����ǵ�ֵַ���������ṹƫ�ƶ�����C����������ʽ�����Խṹָ���������Ӧ���� struct XXX aaa��������Ѱ����ַ
 *        �����������������������n*sizeof(struct xx)����structXX[n+n*sizeof(struct xx)],�����һ��أ����Գ������������� void*xxx
 *        ��Ҫ����Ϊʲô��ô�鷳�����ü�����û��C����ʵ���������鶼��������<_<������������DEBUGʱ����n*sizeof(struct HUFFCODE)Ӧ����n*8
 *        ƫ��ȴ�������n*8*8���������룬ԭ����������ù������о�����ֻ��һ����ѧ�ߣ�ע�Ϳ���ֻ�����Լ��ܿ�������~
 *        ************************************************************************************************************************
 *        ���� MAIN���� ���ֵĴ������ڲ��Թ�������������
*/
int main(void){
	unsigned Carry,index,lineCnt;
	void*lpHc;
	struct POINTERS x;          //����ṹ�������շ���ֵ
	char test[500];
	puts("Enter A String [English]:");
	gets(test); 
	x = HuffManBuild(CreatHuffManTabl(test, strlen(test)),GET_CODE);
	printf("���в��ظ��ֽ�%3d��\n",(int)x.p2);
	lineCnt = (unsigned)x.p2;	   //��Ȼx.p2��һ��VOIDָ�룬����ѭ���ǿ��Եã��Ͼ���Ҳ��һ����ֵ
	for (index = 0; lineCnt;index++,lineCnt--)
	{   //��ѭ�����������Ϣ
		lpHc = (void*)x.p1 + index*sizeof(struct HUFFCODE);
		printf("ֵ=%3d |  ���볤��=%2d | ���� = ", (*(struct HUFFCODE*)lpHc).bValue,(*(struct HUFFCODE*)lpHc).bDepth);
		Carry = (unsigned)1 << ((*(struct HUFFCODE*)lpHc).bDepth - 1);        //������볤��bDepth��5����ô��Ҫ�ӵ���λ��ʼ���ԣ�������λ������һ������0��ʼ��
		while ((*(struct HUFFCODE*)lpHc).bDepth--)
		{   //��ѭ���������
			putchar(((*(struct HUFFCODE*)lpHc).bitVal & Carry) ? '1' : '0');  //�����Ӧλ��1�����1����0�����0
			Carry >>= 1;                                                      //Carry����1λ��׼�������һλ
		}  
		putchar('\n');  //���������һ����Ϣ
	}
	getchar();
	return 0;
}

struct POINTERS HuffManBuild(void*lpHuffTabl, int MODE)
{
	unsigned Quene[300] = { 0 };   	//���У����ÿ��[������ݵ�HNODE�ڵ�]�ĵ�ַ
	//��ʵ��Win32������unsigned��ָ��ͬ������32λ��ֵ����Ϊָ�뱾��Ҳ��ֵ�����Կ��Դ洢��unsigned�����У����ʲô���ͣ�ֻҪ��32λ���У���void*,int*��Щָ������Ҳ���ԣ�
	unsigned offset,totWeight;
	unsigned indexQE, indexSE,indexHN,elemCnt,eCnt;
	void*lpHnode, *lpTmpHnode, *RootPtr;   //lpTmpHnode��ָ��������������ӽڵ����飩��ָ�룬lpHnode��ָ��������������ݽڵ����飩��ָ��
	void*hSeek,*tmphSeek;
	void*lpHuffCode;                       
	struct POINTERS iPointer;
	if ((lpHnode = hAlloc(300 * sizeof(struct HNODE))) == NULL)	FailAlloc();  //ʹ��WIN32������ҪΪ�˷��㣬ֱ��ȫ0�ڴ�飬��ȻMalloc��Ҫ��ʼ��
	//indexQE  ��������
	//indexSE  ����HUFFTABL�е�SUITELEM�ṹ����
	//indexHN  ����HNODE�ṹ����
	eCnt = elemCnt = (*(struct HUFFTABL*)lpHuffTabl).tablElems;             //��ȡ����Ԫ�ظ���
	for (indexQE = 0, indexSE = 0, indexHN = 0; eCnt>0; eCnt--, indexSE++,indexHN++)
	{
		Quene[indexQE++] = (unsigned)lpHnode + indexHN*sizeof(struct HNODE);   //ÿһ��HNODEԪ�صĵ�ַ���� HNODE�ڴ���ָ��+Ԫ������*һ��Ԫ��ռ�е��ֽ�����������������ÿһ�ζ�����Malloc()��
		((struct HNODE*)lpHnode)[indexHN].bValue = (*(struct HUFFTABL*)lpHuffTabl).elem[indexSE].sByte;
		((struct HNODE*)lpHnode)[indexHN].weight = (*(struct HUFFTABL*)lpHuffTabl).elem[indexSE].sFreq;
		((struct HNODE*)lpHnode)[indexHN].lpLeft = NULL;            
		((struct HNODE*)lpHnode)[indexHN].lpRight = NULL;           
		((struct HNODE*)lpHnode)[indexHN].lpParent = NULL;            
		((struct HNODE*)lpHnode)[indexHN].isLeaf = TRUE;           //�����Ƿ�Ϊ���ݽڵ㣨�棩
	}
	//���²�����ҪΪ���������Ľ���
	if ((lpTmpHnode = hAlloc(300 * sizeof(struct HNODE))) == NULL) FailAlloc(); //�������һ��HNODE���顾300��
	//eCnt = elemCnt  ѭ������
	//eCnt > 1        �ϲ�����=Ԫ������-1��
	//offset=0        ��Ҫ���ڼ���RootPtr��ÿ�ν�������ԭָ���sizeof(struct HNODE)���ֽڣ����൱������
	for (eCnt = elemCnt, offset = 0; eCnt > 1; offset++, eCnt--)
	{
		RootPtr = (void*)lpTmpHnode + offset*sizeof(struct HNODE);   //��һ���ӽڵ�HNODE���ȡһ���ڵ�HNODEָ��,����˵���÷���
		indexQE=FindSmallest(Quene, elemCnt);                        //�Ӷ����л�ȡһ����СȨԪ������
		(*(struct HNODE*)Quene[indexQE]).lpParent = (void*)RootPtr;  //����Ԫ�ظ��ڵ�
		totWeight = (*(struct HNODE*)Quene[indexQE]).weight;         //����һ����Ԫ�ص�Ȩֵ
		(*(struct HNODE*)RootPtr).lpLeft = (void*)Quene[indexQE];    //����Ԫ�ظ��ڵ�
		Quene[indexQE] = 0;                                          //�����Ԫ�شӶ����Ƴ�����0�Ļ��ͻᱻFindSmallest����
		indexQE = FindSmallest(Quene, elemCnt);                      //�Ӷ��л�ȡ��һ����СȨԪ��
		(*(struct HNODE*)Quene[indexQE]).lpParent = (void*)RootPtr;  //����Ԫ�ظ��ڵ�
		totWeight += (*(struct HNODE*)Quene[indexQE]).weight;        //��������СȨԪ�ص�Ȩֵ
		(*(struct HNODE*)RootPtr).lpRight = (void*)Quene[indexQE];   //��Ԫ�����ӵ����ڵ���֧
		(*(struct HNODE*)RootPtr).lpParent = NULL;                   //�������ӽڵ㸸�ڵ�Ϊ��
		(*(struct HNODE*)RootPtr).weight = totWeight;                //��������СȨԪ�ص�Ȩд�븸�ڵ�
		(*(struct HNODE*)RootPtr).isLeaf = FALSE;                    //���ӽڵ�����ݽڵ�
		Quene[indexQE] = (unsigned)RootPtr;                         //���ºϲ��Ľڵ�������
	}
	//�����GET_TREEģʽ����ô����һ���� 
	if (MODE)            
	{
		iPointer.p1 = (void*)RootPtr;                                //RootPtrָ����ڵ�
		iPointer.p2 = (void*)lpTmpHnode;                             //��������Ҫ��Ϊ�ͷ��ڴ棬��Ϊ����û���ã���ʱ�����ͷš�
		iPointer.p3 = (void*)lpHnode;                             
		return iPointer;
	}
	//����HUFFCODE�ṹ�����ڴ��
	if ((lpHuffCode = hAlloc(300 * sizeof(struct HUFFCODE))) == NULL) FailAlloc();
	for (offset = 0, indexHN = 0, eCnt = elemCnt; eCnt--; indexHN++, offset++)
	{
		hSeek = (void*)lpHnode + indexHN*sizeof(struct HNODE);
		(*(struct HUFFCODE*)(lpHuffCode + offset*sizeof(struct HUFFCODE))).bValue = (*(struct HNODE*)hSeek).bValue;
		while ((*(struct HNODE*)hSeek).lpParent)
		{
			tmphSeek = hSeek;                                  //����ָ�����ڱȽ�����߻����ұ�
			hSeek = (*(struct HNODE*)hSeek).lpParent;          //p=p->next;
			if ((*(struct HNODE*)hSeek).lpLeft == tmphSeek)
			{   //����ߣ�д��0
				RCR(&((*(struct HUFFCODE*)(lpHuffCode + offset*sizeof(struct HUFFCODE))).bitVal), 0);
			}
			else
			{   //���ұߣ�д��1
				RCR(&((*(struct HUFFCODE*)(lpHuffCode + offset*sizeof(struct HUFFCODE))).bitVal), 1);
			}
			++(*(struct HUFFCODE*)(lpHuffCode + offset*sizeof(struct HUFFCODE))).bDepth;  //���볤��++
		}
		(*(struct HUFFCODE*)(lpHuffCode + offset*sizeof(struct HUFFCODE))).bitVal >>= (32 - (*(struct HUFFCODE*)(int)(lpHuffCode + offset*sizeof(struct HUFFCODE))).bDepth);
	}
	iPointer.p1 = (void*)lpHuffCode;                                               //����HUFFCODE����ָ��
	iPointer.p2 = (void*)elemCnt;                                                  //����Ԫ�ظ��������ǿ��Եã�
	hFree(lpTmpHnode);
	hFree(lpHnode);
	return iPointer;
}
//RCR�����������ǣ�����һ��unsigned(dest)ֵ������һ�����ر�־ 0 or 1��flag)���������־λ��λ������ֵ�������һλ
//����һ��ֵ iVal=8�������� 0000 1000������������RCR(&iVal,1),��ôiVal����� 132��������1000 0100��
void RCR(unsigned*dest, unsigned flag)
{
	*dest >>= 1;
	*dest |= (flag << 31);
}

void* CreatHuffManTabl(void*lpData, unsigned bCnt)
{
	unsigned tmpCnt = 0;
	unsigned validCnt, index;
	unsigned*lpFreqTabl;
	void*lpHuffTabl;
	struct SUITELEM iSuit;
	if ((lpFreqTabl = hAlloc(300 * sizeof( unsigned))) == NULL) FailAlloc();
	if ((lpHuffTabl = hAlloc(sizeof(struct HUFFTABL))) == NULL) FailAlloc();
	while (tmpCnt++<bCnt) index = *(unsigned char*)lpData++, lpFreqTabl[index]++;
	//����������ȡһ���ֽ���Ϊ��������ָ����һ���ֽ�
    //������ֽ���126����ô�ֽ�126�ĳ���Ƶ�ʼ� 1
	for (validCnt = 256, index = 255; index !=(unsigned)-1; index--)
	{
		if (!lpFreqTabl[index])
			--validCnt;         //������ֽڶ�Ӧ��Ƶ��Ϊ0����ô����ֽ�δ���ֹ�������
	}
	(*(struct HUFFTABL*)lpHuffTabl).tablElems = validCnt;
	for (index = 0; validCnt ; validCnt--)
	{
		FindBiggest(lpFreqTabl, &iSuit);
		(*(struct HUFFTABL*)lpHuffTabl).elem[index++] = iSuit;
	}
	hFree(lpFreqTabl);
	return lpHuffTabl;

}
int FindBiggest(unsigned*lpFreqTabl, void*lpSuitByte)
{
	unsigned forComp,suitByte = 0;
	int index = 0;
	for (forComp = lpFreqTabl[index++]; index < 256; index++)
	{
		if (lpFreqTabl[index]>forComp)
		{
			forComp = lpFreqTabl[index];
			suitByte = index;
		}
	}
	(*(struct SUITELEM*)lpSuitByte).sByte = suitByte;
	(*(struct SUITELEM*)lpSuitByte).sFreq = forComp;
	lpFreqTabl[suitByte] = 0;
	return 0;
}

int FindSmallest(unsigned*lpQuene, unsigned elemCnt)
{
	unsigned index = 0;
	unsigned setNull;
	unsigned forComp;
	while (!lpQuene[index] && elemCnt) index++,elemCnt--;
	forComp = (*(struct HNODE*)lpQuene[index]).weight;
	setNull = index++;
	while (elemCnt--)
	{
		if (lpQuene[index])
		{
			if (forComp > ((*(struct HNODE*)lpQuene[index]).weight))
			{
				forComp = (*(struct HNODE*)lpQuene[index]).weight;
				setNull = index;
			}
		}
	    ++index;
	}
	return setNull;
}
void FailAlloc(void)
{
	puts("Mem Alloc Failed!");
	exit(EXIT_FAILURE);
}





