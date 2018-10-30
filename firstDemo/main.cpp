#include <iostream>
using namespace std;
#include ".\gdal\gdal_priv.h"
#pragma comment(lib, "gdal_i.lib")

enum BoundaryCondition
{
	zero
	, bound
	, period
};

enum Method
{
	full
	, same
	, valid
};

//��������
const int A_Row = 256;
const int A_Col = 256;
const int B_Row = 5;
const int B_Col = 5;
BoundaryCondition Bc = zero;
Method method = full;

float A[A_Row][A_Col];
float B[B_Row][B_Col] = {{0.00048,0.005012,0.010944,0.005012,0.00048},
						{0.005012,0.052216,0.103256,0.052216,0.005012},
						{0.010944,0.103256,0.249104,0.103256,0.010944},
						{0.005012,0.052216,0.103256,0.052216,0.005012},
						{0.00048,0.005012,0.010944,0.005012,0.00048} };
float ** C = 0;
int cR;
int cC;

template <typename T>
T GetA_Ele(int row, int col);
template <typename T>
void conv(const T a[][A_Col], const T b[][B_Col], T**& c);

//����A��B�ľ��  
template<typename T>
void conv(const T a[][A_Col], const T b[][B_Col], T**& c)
{
	int offsetR = 0;
	int offsetC = 0;

	switch (method)
	{
	case full:
		cR = A_Row + B_Row - 1;
		cC = A_Col + B_Col - 1;
		break;
	case same:
		cR = A_Row;
		cC = A_Col;
		offsetR = B_Row / 2;
		offsetC = B_Col / 2;
		break;
	case valid:
		cR = A_Row - B_Row + 1;
		cC = A_Col - B_Col + 1;
		if ((cR < 1) | (cC < 1))
			return;
		offsetR = B_Row - 1;
		offsetC = B_Col - 1;
		break;
	default:
		return;
	}
	c = new T*[cR];             //����ά�������ռ�
	for (int i = 0; i < cR; i++)
		c[i] = new T[cC];
	for (int i = 0; i < cR; i++)
	{
		for (int j = 0; j < cC; j++)
		{
			c[i][j] = 0;
			for (int k1 = 0; k1 < B_Row; k1++)
			{
				for (int k2 = 0; k2 < B_Col; k2++)
					c[i][j] += b[k1][k2] * GetA_Ele<float>(i - k1 + offsetR, j - k2 + offsetC);
			}
		}
	}
}

//���ݱ߽�������ȡA�����Ԫ��
template <typename T>
T GetA_Ele(int row, int col)
{
	switch (Bc)
	{
	case zero:      //��������������Ϊ0
		if ((row < 0) | (row > A_Row) | (col < 0) | (col > A_Col))
			return 0;
	case bound:     //�����������ֺͱ߽�ֵ���
		if (row < 0)
			row = 0;
		else if (row >= A_Row)
			row = A_Row - 1;
		if (col < 0)
			col = 0;
		else if (col >= A_Col)
			col = A_Col - 1;
		return A[row][col];
	case period:
		while ((row < 0) | (row >= A_Row))
		{
			if (row < 0)
				row += A_Row;
			else
				row -= A_Row;
		}
		while ((col < 0) | (col >= A_Col))
		{
			if (col < 0)
				col += A_Col;
			else
				col -= A_Col;
		}
		return A[row][col];
	default:
		return T(0);
	}
}

int main()
{

	GDALDataset* poSrcDS;
	GDALDataset* poDstDS;
	int imgXlen, imgYlen;
	char* srcPath = "Lena.jpg";
	char* dstPath = "lena6.tif";
	GByte* buffTmp;
	int i, j, bandNum, k;
	int I, J;


	GDALAllRegister();
	poSrcDS = (GDALDataset*)GDALOpenShared(srcPath, GA_ReadOnly);
	imgXlen = poSrcDS->GetRasterXSize();
	imgYlen = poSrcDS->GetRasterYSize();
	bandNum = poSrcDS->GetRasterCount();
	cout << "Image X Length" << imgXlen << endl;
	cout << "Image Y Length" << imgYlen << endl;
	cout << "Band number" << bandNum << endl;
	buffTmp = (GByte*)CPLMalloc(imgXlen*imgXlen * sizeof(GByte));
	
	poDstDS = GetGDALDriverManager()->GetDriverByName("GTiff")->Create(dstPath, imgXlen, imgYlen, bandNum, GDT_Byte, NULL);

	for (I = 0; I< bandNum; I++) {
		poSrcDS->GetRasterBand(I + 1)->RasterIO(GF_Read,
			0, 0, imgXlen, imgYlen, buffTmp, imgXlen, imgYlen, GDT_Byte, 0, 0);//��������
		
		for (j = 0; j < 256; j++) {
			for (i = 0; i < 256; i++) {
				A[j][i] = (GByte)buffTmp[j * 256 + i];//�õ����ظ�ֵ��A����
				}
			}
		conv(A, B, C);//���о����B�������ˣ���ͬ�������Ҫ���иı�
		/*for (int i = 0; i < cR; i++)
	{
		for (int j = 0; j < cC; j++)
			cout << C[i][j] << "\t";
		cout << endl;
	}*/
		for (j = 0; j < 256; j++) {
			for (i = 0; i < 256; i++) {
			buffTmp[j * 256 + i] = C[j][i];//�Ѿ��������ֵ��ֵ��buffTmp
			}
		}
		delete[] C;
		poDstDS->GetRasterBand(I + 1)->RasterIO(GF_Write,   
			0, 0, imgXlen, imgYlen, buffTmp, imgXlen, imgYlen, GDT_Byte, 0, 0);
}
			cout << "aaa" << endl;
			/*for (int j = 0; j < bandNum; j++) {
				poDstDS->GetRasterBand(j + 1)->RasterIO(GF_Write,0, 0, imgXlen, imgYlen, buffTmp, imgXlen, imgYlen, GDT_Byte, 0, 0);
			}*/
			CPLFree(buffTmp);
			GDALClose(poDstDS);
			GDALClose(poSrcDS);
			system("PAUSE");
			return 0;
		}