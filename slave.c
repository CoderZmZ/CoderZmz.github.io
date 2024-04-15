#include <slave.h>
#include "pcg_def.h"

#include <crts.h>

typedef struct{
	double *p;
	double *z;
	double beta;
	int cells;
} Para;

typedef struct{
    int       rows;   
    int   *row_off;     
    int      *cols;
    double   *data;
    double    *vec;
    double *result;
	double    *val;
}cs_sp;

#define dataBufferSize 2000



void slave_example(Para* para){
	crts_rply_t DMARply = 0;
	unsigned int DMARplyCount = 0;
	double p[dataBufferSize] __attribute__ ((aligned(64)));
	double z[dataBufferSize] __attribute__ ((aligned(64)));

	Para slavePara;
	//接收结构体数据
	CRTS_dma_iget(&slavePara, para, sizeof(Para), &DMARply);
	DMARplyCount++;
	CRTS_dma_wait_value(&DMARply, DMARplyCount);
	double beta = slavePara.beta;
	int cells = slavePara.cells;
	//分核
	//计算从核接收数组数据长度和接收位置
	int len = cells / 64;
	int rest = cells % 64;
	int addr;
	if(CRTS_tid < rest){
		len++;
		addr = CRTS_tid * len;
	}else{
		addr = CRTS_tid * len + rest;
	}
	//接收数组数据  双缓冲
	CRTS_dma_iget(&p, slavePara.p + addr, len * sizeof(double), &DMARply);
	CRTS_dma_iget(&z, slavePara.z + addr, len * sizeof(double), &DMARply);
	DMARplyCount += 2;
	CRTS_dma_wait_value(&DMARply, DMARplyCount);
			
	//计算
	int i = 0;
	//len
	//simd加速，  cache  ,ldm  私有   ，ldm 共享   ，cache 三者分块，数据对齐
	for(; i < len; i++){
		p[i] = z[i] + beta * p[i];
	}
	//传回计算结果
	CRTS_dma_iput(slavePara.p+addr, &p, len * sizeof(double), &DMARply);
	DMARplyCount++;
	CRTS_dma_wait_value(&DMARply, DMARplyCount);
}

// for(int i = 0; i < csr_matrix.rows; i++) {
//         int start = csr_matrix.row_off[i];
//         int num = csr_matrix.row_off[i+1] - csr_matrix.row_off[i];
//         double temp = 0;
//         for(int j = 0; j < num; j++) {                      
//             temp += vec[csr_matrix.cols[start+j]] * csr_matrix.data[start+j]; 
//         }
//         result[i]=temp;
//     }


void slave_csr_spmv(cs_sp* cs){
	crts_rply_t DMARply = 0;
	unsigned int DMARplyCount = 0;
	double result[dataBufferSize] __attribute__ ((aligned(64)));
	int row_off[dataBufferSize] __attribute__ ((aligned(64)));
	cs_sp slavePara;
	CRTS_dma_iget(&slavePara, cs, sizeof(cs_sp), &DMARply);
	DMARplyCount++;
	CRTS_dma_wait_value(&DMARply, DMARplyCount);

	int rows = slavePara.rows;
	int len = rows / 64;
	int rest = rows % 64;
	int addr;
	if(CRTS_tid < rest){
		len++;
		addr = CRTS_tid * len;
	}else{
		addr = CRTS_tid * len + rest;
	}

	CRTS_dma_iget(&row_off, slavePara.row_off + addr, (len+1) * sizeof(int), &DMARply);

	DMARplyCount++;
	CRTS_dma_wait_value(&DMARply, DMARplyCount);
	
	int i=0;
	for( ; i < len; i++) {
        int start = row_off[i];
        int num   = row_off[i+1] - row_off[i];
        double temp = 0;
        for(int j = 0; j < num; j++) {                      
            temp += slavePara.vec[slavePara.cols[start+j]] * slavePara.data[start+j]; 
        }
        result[i]=temp;
    }

	CRTS_dma_iput(slavePara.result+addr, &result, len * sizeof(double), &DMARply);
	DMARplyCount++;
	CRTS_dma_wait_value(&DMARply, DMARplyCount);
}

// for(int i = 0; i < csr_matrix.rows; i++) {
    //     int start = csr_matrix.row_off[i];
    //     int num = csr_matrix.row_off[i+1] - csr_matrix.row_off[i];
    //     double temp = 0;
    //     for(int j = 0; j < num; j++) {                      
    //         temp += vec[csr_matrix.cols[start+j]] * val[start+j]; 
    //     }
    //     result[i]=temp;
    // }
void slave_csr_pre_spmv(cs_sp* cs){
	crts_rply_t DMARply = 0;
	unsigned int DMARplyCount = 0;
	double result[dataBufferSize] __attribute__ ((aligned(64)));
	int row_off[dataBufferSize] __attribute__ ((aligned(64)));
	cs_sp slavePara;
	CRTS_dma_iget(&slavePara, cs, sizeof(cs_sp), &DMARply);
	DMARplyCount++;
	CRTS_dma_wait_value(&DMARply, DMARplyCount);

	int rows = slavePara.rows;
	int len = rows / 64;
	int rest = rows % 64;
	int addr;
	if(CRTS_tid < rest){
		len++;
		addr = CRTS_tid * len;
	}else{
		addr = CRTS_tid * len + rest;
	}

	CRTS_dma_iget(&row_off, slavePara.row_off + addr, (len+1) * sizeof(int), &DMARply);

	DMARplyCount++;
	CRTS_dma_wait_value(&DMARply, DMARplyCount);
	
	int i=0;
	for( ; i < len; i++) {
        int start = row_off[i];
        int num   = row_off[i+1] - row_off[i];
        double temp = 0;
        for(int j = 0; j < num; j++) {                      
            temp += slavePara.vec[slavePara.cols[start+j]] * slavePara.val[start+j]; 
        }
        result[i]=temp;
    }

	CRTS_dma_iput(slavePara.result+addr, &result, len * sizeof(double), &DMARply);
	DMARplyCount++;
	CRTS_dma_wait_value(&DMARply, DMARplyCount);
}