#ifndef _PCG_DEF_H_
#define _PCG_DEF_H_


struct LduMatrix {
	double *upper;           //存储上三角部分
	double *lower;           //存储下三角部分
	double *diag;            //存储对角线部分
	int *uPtr;               //每个指针指向 upper 数组中的一个位置，表示每一列中上三角部分的起始索引。
	int *lPtr;               //每个指针指向 lower 数组中的一个位置，表示每一列中下三角部分的起始索引。
	int faces;               //非零元总数
	int cells;               //表示矩阵中的单元数。单元数是指矩阵的维度，即矩阵的行数或列数。
};

struct CsrMatrix {
    int rows;               //表示矩阵的行数。
    int *row_off;           //存储每一行的起始偏移量。偏移量表示每一行在 cols 和 data 数组中的起始位置。
    int *cols;              //存储矩阵中非零元素的列索引。数组的长度由 row_off 中的偏移量确定。
    double *data;           //存储矩阵中非零元素的值。数组的长度由 row_off 中的偏移量确定。
    int data_size;          //表示 cols 和 data 数组的长度，即矩阵中非零元素的总数。
};

struct PCG {
    double *r;              // 残差向量
    double *z;              // 预条件向量
    double *p;              // 搜索方向向量
    double *Ax;             // 矩阵与搜索方向向量的乘积
    double sumprod;         // 向量运算中的中间结果
    double sumprod_old;     // 向量运算中的中间结果的旧值
    double residual;        // 当前迭代步骤的残差
    double alpha;           // 共轭梯度法中的步长因子
    double beta;            // 共轭梯度法中的方向调整因子

    double *x;              // 线性方程组的解向量
    double *source;         // 线性方程组的右侧源向量
};

struct Precondition{
    double *pre_mat_val;
    double *preD;
};

struct PCGReturn{
    double residual;
    int iter;
};

#include <time.h>
#include <stdio.h>
#define INFO(M, ...) {  time_t t; \
                        struct tm *tmif; \
                        t = time(NULL); \
                        tmif = localtime(&t); \
                        printf("[%d-%2d-%2d] [%2d:%2d:%2d] [INFO] " M "",  \
                                tmif->tm_year + 1900, \
                                tmif->tm_mon + 1, \
                                tmif->tm_mday, \
                                tmif->tm_hour, \
                                tmif->tm_min, \
                                tmif->tm_sec, ##__VA_ARGS__); \
                    }

#endif
