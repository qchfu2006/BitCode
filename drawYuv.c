/*
几种常用YUV格式图像内存分布 (以 4x2 共8个像素为例)：
1）YUV444I (interleaved)  每个Y分量都有一个UV分量：
YUV	YUV	YUV	YUV
YUV	YUV	YUV	YUV

2）YUV444P (planar) 每个Y分量都有一个UV分量                                          

Y	Y	Y	Y
Y	Y	Y	Y
U	U	U	U
U	U	U	U
V	V	V	V
V	V	V	V

3）YUV422P   每两个Y分量公用一个UV分量

Y	Y	Y	Y
Y	Y	Y	Y
U	U	U	U
V	V	V	V
 
4）YUV422I_UYVY / YUYV / VYUY / YVYU   每两个Y分量公用一个UV分量

U	Y	V	Y	U	Y	V	Y
U	Y	V	Y	U	Y	V	Y
 

5）YUV420SP_NV12 / NV21  每四个Y分量公用一个UV分量

Y	Y	Y	Y
Y	Y	Y	Y
U	V	U	V

原文链接：https://blog.csdn.net/u010312436/article/details/101379177
*/

#include <stdio.h>
#define READ_MAX (1024)
 
typedef unsigned char  uInt8;
typedef unsigned short uInt16;
typedef unsigned int uInt32;
typedef char Int8;
typedef short Int16;
typedef int Int32;
 
typedef enum
{
	TYPE_YUV422I_UYVY,
	TYPE_YUV422I_YUYV,
	TYPE_YUV420SP_NV12,
	TYPE_YUV420SP_NV21,
	TYPE_YUV422P,
	TYPE_YUV444I,
	TYPE_YUV444P,
}enYuvType;
 
typedef enum
{
	YUV_GREEN,
	YUV_RED,
	YUV_BLUE,
	YUV_PURPLE,
	YUV_DARK_GREEN,
	YUV_YELLOW,
	YUV_LIGHT_BLUE,
	YUV_LIGHT_PURPLE,
	YUV_DARK_BLACK,
	YUV_GRAY,
	YUV_WHITE,
	YUV_COLOR_MAX,
}enYuvColorIdx;
 
typedef struct
{
	uInt8 Y;
	uInt8 U;
	uInt8 V;
}stYuvColor;
 
typedef struct
{
	uInt16 x;
	uInt16 y;
}stPoint;
 
typedef struct
{
	stPoint startPoint;
	stPoint endPoint;
	uInt16 lineWidth;
	enYuvColorIdx clrIdx;
}stDrawLineInfo;
 
typedef struct
{
	enYuvType yuvType;
	uInt8 *pYuvBuff;
	uInt16 width;
	uInt16 height;
}stYuvBuffInfo;
 
static stYuvColor s_color_table[YUV_COLOR_MAX] = {
	{0x00, 0x00, 0x00}, // green
	{0x00, 0x00, 0xff}, // red
	{0x00, 0xff, 0x00},	// blue
	{0x00, 0xff, 0xff},	// purple
	{0xff, 0x00, 0x00}, // dark green
	{0xff, 0x00, 0xff}, // yellow
	{0xff, 0xff, 0x00}, // light blue
	{0xff, 0xff, 0xff}, // light purple
	{0x00, 0x80, 0x80}, // dark black
	{0x80, 0x80, 0x80}, // gray
	{0xff, 0x80, 0x80}, // white
};
 
Int32 read_file(const char* file, uInt8 *pOut)
{
	Int32 size = 0;
	uInt8 *p = pOut;
	FILE* fp = fopen(file, "rb");
	if(fp && pOut) {
		int n = 0;
		while((n = fread(p, 1, READ_MAX, fp)) > 0) {
			size += n;
			p += n;
			if(n < READ_MAX)
				break;
		}
		fclose(fp);
		printf("	%s, size = %d\n", file, size);
	}
	else {
		printf("Open %s png file fail\n", file);
	}
 
	return size;
}
 
 
void yuv_setdata(
	uInt8* YBuff,
	uInt8* UVBuff,
	enYuvType yuvType,
	uInt16 width,
	uInt16 height,
	stPoint draw_point,
	enYuvColorIdx clrIdx)
{
	switch(yuvType)
	{
		case TYPE_YUV422I_UYVY:
		case TYPE_YUV422I_YUYV:
		{
			/*
				UYVY UYVY UYVY UYVY
			*/
			uInt32 tmp = draw_point.y * width * 2;
			uInt32 y_offset = 0, u_offset = 0, v_offset = 0;
			if(yuvType == TYPE_YUV422I_UYVY) {
				u_offset = tmp + draw_point.x / 2 * 4;
				v_offset = u_offset + 2;
				y_offset = u_offset + 1;
			}
			else {
				y_offset = tmp + draw_point.x / 2 * 4;
				u_offset = y_offset + 1;
				v_offset = u_offset + 2;
			}
			YBuff[y_offset] = s_color_table[clrIdx].Y;
			YBuff[y_offset + 2] = s_color_table[clrIdx].Y;
			YBuff[u_offset] = s_color_table[clrIdx].U;
			YBuff[v_offset] = s_color_table[clrIdx].V;
		}break;
		case TYPE_YUV420SP_NV12:
		case TYPE_YUV420SP_NV21:
		{
			/*
				YY YY
				YY YY
				UV UV
			*/
			uInt32 y_offset = draw_point.y * width + draw_point.x;
			uInt32 u_offset = 0, v_offset = 0;
			YBuff[y_offset] = s_color_table[clrIdx].Y;
			#if 0
			Int32 x_flag = 1, y_flag = 1;
			if(draw_point.y % 2 == 0) {
				YBuff[y_offset + width] = s_color_table[clrIdx].Y;
				y_flag = 1;
			}
			else {
				YBuff[y_offset - width] = s_color_table[clrIdx].Y;
				y_flag = -1;
			}
 
			if(draw_point.x % 2 == 0) {
				YBuff[y_offset + 1] = s_color_table[clrIdx].Y;
				x_flag = 1;
			}
			else {
				YBuff[y_offset - 1] = s_color_table[clrIdx].Y;
				x_flag = -1;
			}
			YBuff[y_offset + width * y_flag + 1 * x_flag] = s_color_table[clrIdx].Y;
			#endif
			
			if(yuvType == TYPE_YUV420SP_NV12) {
				u_offset = (draw_point.y / 2) * width + draw_point.x / 2 * 2;
				v_offset = u_offset + 1;
			}
			else {
				v_offset = (draw_point.y / 2) * width + draw_point.x / 2 * 2;
				u_offset = v_offset + 1;
			}
			UVBuff[u_offset] = s_color_table[clrIdx].U;
			UVBuff[v_offset] = s_color_table[clrIdx].V;
			//printf("[%d, %d]: y_offset = %d, u_offset = %d, v_offset = %d\n",
			//	draw_point.x, draw_point.y, y_offset, u_offset, v_offset);
		}break;
		case TYPE_YUV444P:
		{
			/*
				YYYYYYYY
				UUUUUUUU
				VVVVVVVV
			*/
			uInt32 y_offset = 0, u_offset = 0, v_offset = 0;
			uInt32 plane_size = width * height;
			y_offset = draw_point.y * width + draw_point.x;
			u_offset = y_offset;
			v_offset = plane_size + u_offset;
			YBuff[y_offset] = s_color_table[clrIdx].Y;
			UVBuff[u_offset] = s_color_table[clrIdx].U;
			UVBuff[v_offset] = s_color_table[clrIdx].V;
		}break;
		case TYPE_YUV444I:
		{
			/*
				YUV YUV YUV YUV YUV YUV YUV YUV
			*/
			uInt32 y_offset = 0, u_offset = 0, v_offset = 0;
			y_offset = draw_point.y * width * 3 + draw_point.x * 3;
			u_offset = y_offset + 1;
			v_offset = u_offset + 1;
			YBuff[y_offset] = s_color_table[clrIdx].Y;
			YBuff[u_offset] = s_color_table[clrIdx].U;
			YBuff[v_offset] = s_color_table[clrIdx].V;
		}break;
		case TYPE_YUV422P:
		{
			/*
				YYYYYYYY
				UUUU
				VVVV
			*/
			uInt32 y_offset = 0, u_offset = 0, v_offset = 0;
			uInt32 plane_size = width * height / 2;
			y_offset = draw_point.y * width + draw_point.x;
			u_offset = (draw_point.y / 2) * width + draw_point.x / 2;
			v_offset = plane_size + u_offset;
			YBuff[y_offset] = s_color_table[clrIdx].Y;
			UVBuff[u_offset] = s_color_table[clrIdx].U;
			UVBuff[v_offset] = s_color_table[clrIdx].V;
		}break;
	}
}
 
void yuv_drawline(stYuvBuffInfo *pYuvBuffInfo, stDrawLineInfo *pDrawLineInfo)
{
	if(!pYuvBuffInfo || !pYuvBuffInfo->pYuvBuff) return;
 
	uInt8 *YBuff = NULL, *UVBuff = NULL;
	uInt16 x0 = pDrawLineInfo->startPoint.x, y0 = pDrawLineInfo->startPoint.y;
	uInt16 x1 = pDrawLineInfo->endPoint.x, y1 = pDrawLineInfo->endPoint.y;
 
	if(pDrawLineInfo->lineWidth == 0) pDrawLineInfo->lineWidth = 1;
	x0 = (x0 >= pYuvBuffInfo->width) ? (x0 - pDrawLineInfo->lineWidth) : x0;
	x1 = (x1 >= pYuvBuffInfo->width) ? (x1 - pDrawLineInfo->lineWidth) : x1;
	y0 = (y0 >= pYuvBuffInfo->height) ? (y0 - pDrawLineInfo->lineWidth) : y0;
	y1 = (y1 >= pYuvBuffInfo->height) ? (y1 - pDrawLineInfo->lineWidth) : y1;
 
	uInt16 dx = (x0 > x1) ? (x0 - x1) : (x1 - x0);
	uInt16 dy = (y0 > y1) ? (y0 - y1) : (y1 - y0);
 
	Int16 xstep = (x0 < x1) ? 1 : -1;
	Int16 ystep = (y0 < y1) ? 1 : -1;
	Int16 nstep = 0, eps = 0;
 
	stPoint draw_point;
	draw_point.x = x0;
	draw_point.y = y0;
 
	switch(pYuvBuffInfo->yuvType)
	{
		case TYPE_YUV422I_UYVY:
		case TYPE_YUV422I_YUYV:
		case TYPE_YUV444I:
		{
			YBuff = pYuvBuffInfo->pYuvBuff;
			UVBuff = NULL;
		}break;
		case TYPE_YUV420SP_NV12:
		case TYPE_YUV420SP_NV21:
		case TYPE_YUV444P:
		case TYPE_YUV422P:
		{
			YBuff = pYuvBuffInfo->pYuvBuff;
			UVBuff = pYuvBuffInfo->pYuvBuff + pYuvBuffInfo->width * pYuvBuffInfo->height;
		}break;
		default:
			return;
	}
 
	// 布雷森汉姆算法画线
	if(dx > dy){
		while(nstep <= dx) {
			yuv_setdata(YBuff, UVBuff, pYuvBuffInfo->yuvType, pYuvBuffInfo->width, pYuvBuffInfo->height, draw_point, pDrawLineInfo->clrIdx);
			eps += dy;
			if( (eps << 1) >= dx ) {
				draw_point.y += ystep;
				eps -= dx;
			}
			draw_point.x += xstep;
			nstep++;
		}
	}else {
		while(nstep <= dy){	
			yuv_setdata(YBuff, UVBuff, pYuvBuffInfo->yuvType, pYuvBuffInfo->width, pYuvBuffInfo->height, draw_point, pDrawLineInfo->clrIdx);
			eps += dx;
			if( (eps << 1) >= dy ) {
				draw_point.x += xstep;
				eps -= dy;
			}
			draw_point.y += ystep;
			nstep++;
		}
	}
}
 
void draw_rect(stYuvBuffInfo* yuvBuffInfo)
{
	stDrawLineInfo drawLineInfo;
	drawLineInfo.clrIdx = YUV_RED;
	drawLineInfo.lineWidth = 1;
	drawLineInfo.startPoint.x = 160;
	drawLineInfo.startPoint.y = 140;
	drawLineInfo.endPoint.x = 560;
	drawLineInfo.endPoint.y = 340;
	yuv_drawline(yuvBuffInfo, &drawLineInfo);
 
	drawLineInfo.clrIdx = YUV_PURPLE;
	drawLineInfo.lineWidth = 1;
	drawLineInfo.startPoint.x = 560;
	drawLineInfo.startPoint.y = 140;
	drawLineInfo.endPoint.x = 160;
	drawLineInfo.endPoint.y = 340;
	yuv_drawline(yuvBuffInfo, &drawLineInfo);
 
	drawLineInfo.clrIdx = YUV_YELLOW;
	drawLineInfo.lineWidth = 1;
	drawLineInfo.startPoint.x = 160;
	drawLineInfo.startPoint.y = 140;
	drawLineInfo.endPoint.x = 560;
	drawLineInfo.endPoint.y = 140;
	yuv_drawline(yuvBuffInfo, &drawLineInfo);
 
	drawLineInfo.clrIdx = YUV_GREEN;
	drawLineInfo.lineWidth = 1;
	drawLineInfo.startPoint.x = 160;
	drawLineInfo.startPoint.y = 140;
	drawLineInfo.endPoint.x = 160;
	drawLineInfo.endPoint.y = 340;
	yuv_drawline(yuvBuffInfo, &drawLineInfo);
 
	drawLineInfo.clrIdx = YUV_BLUE;
	drawLineInfo.lineWidth = 1;
	drawLineInfo.startPoint.x = 160;
	drawLineInfo.startPoint.y = 340;
	drawLineInfo.endPoint.x = 560;
	drawLineInfo.endPoint.y = 340;
	yuv_drawline(yuvBuffInfo, &drawLineInfo);
 
	drawLineInfo.clrIdx = YUV_WHITE;
	drawLineInfo.lineWidth = 1;
	drawLineInfo.startPoint.x = 560;
	drawLineInfo.startPoint.y = 140;
	drawLineInfo.endPoint.x = 560;
	drawLineInfo.endPoint.y = 340;
	yuv_drawline(yuvBuffInfo, &drawLineInfo);
}
 
void main(int argc, char** argv)
{
	stYuvBuffInfo yuvBuffInfo;
	uInt8 *pBuff = (uInt8*)malloc(sizeof(uInt8) * 10 * 0x100000); // 10M
	// 测试 NV12 格式
	Int32 size = read_file("yuv_data.nv12", pBuff);
	yuvBuffInfo.pYuvBuff = pBuff;
	yuvBuffInfo.width = 720;
	yuvBuffInfo.height = 480;
	yuvBuffInfo.yuvType = TYPE_YUV420SP_NV12;
	draw_rect(&yuvBuffInfo);
	FILE* fp_save = fopen("./yuv_data_line.nv12", "wb+");
	fwrite(pBuff, size, 1, fp_save);
	fclose(fp_save);
 
	// 测试UYVY 格式
	memset(pBuff, 0, sizeof(uInt8) * 10 * 0x100000);
	size = read_file("yuv_data.uyvy", pBuff);
	yuvBuffInfo.pYuvBuff = pBuff;
	yuvBuffInfo.width = 720;
	yuvBuffInfo.height = 480;
	yuvBuffInfo.yuvType = TYPE_YUV422I_UYVY;
	draw_rect(&yuvBuffInfo);
	fp_save = fopen("./yuv_data_line.uyvy", "wb+");
	fwrite(pBuff, size, 1, fp_save);
 
	// 测试 NV21 格式
	size = read_file("yuv_data.nv21", pBuff);
	yuvBuffInfo.pYuvBuff = pBuff;
	yuvBuffInfo.width = 720;
	yuvBuffInfo.height = 480;
	yuvBuffInfo.yuvType = TYPE_YUV420SP_NV21;
	draw_rect(&yuvBuffInfo);
	fp_save = fopen("./yuv_data_line.nv21", "wb+");
	fwrite(pBuff, size, 1, fp_save);
	fclose(fp_save);
 
	// 测试YUYV 格式
	memset(pBuff, 0, sizeof(uInt8) * 10 * 0x100000);
	size = read_file("yuv_data.yuyv", pBuff);
	yuvBuffInfo.pYuvBuff = pBuff;
	yuvBuffInfo.width = 720;
	yuvBuffInfo.height = 480;
	yuvBuffInfo.yuvType = TYPE_YUV422I_YUYV;
	draw_rect(&yuvBuffInfo);
	fp_save = fopen("./yuv_data_line.yuyv", "wb+");
	fwrite(pBuff, size, 1, fp_save);
 
	// 测试YUV444P 格式
	memset(pBuff, 0, sizeof(uInt8) * 10 * 0x100000);
	size = read_file("yuv_data.yuv444p", pBuff);
	yuvBuffInfo.pYuvBuff = pBuff;
	yuvBuffInfo.width = 720;
	yuvBuffInfo.height = 480;
	yuvBuffInfo.yuvType = TYPE_YUV444P;
	draw_rect(&yuvBuffInfo);
	fp_save = fopen("./yuv_data_line.yuv444p", "wb+");
	fwrite(pBuff, size, 1, fp_save);
 
	// 测试YUV444I 格式
	memset(pBuff, 0, sizeof(uInt8) * 10 * 0x100000);
	size = read_file("yuv_data.yuv444i", pBuff);
	yuvBuffInfo.pYuvBuff = pBuff;
	yuvBuffInfo.width = 720;
	yuvBuffInfo.height = 480;
	yuvBuffInfo.yuvType = TYPE_YUV444I;
	draw_rect(&yuvBuffInfo);
	fp_save = fopen("./yuv_data_line.yuv444i", "wb+");
	fwrite(pBuff, size, 1, fp_save);
 
	// 测试YUV422P 格式
	memset(pBuff, 0, sizeof(uInt8) * 10 * 0x100000);
	size = read_file("yuv_data.yuv422p", pBuff);
	yuvBuffInfo.pYuvBuff = pBuff;
	yuvBuffInfo.width = 720;
	yuvBuffInfo.height = 480;
	yuvBuffInfo.yuvType = TYPE_YUV422P;
	draw_rect(&yuvBuffInfo);
	fp_save = fopen("./yuv_data_line.yuv422p", "wb+");
	fwrite(pBuff, size, 1, fp_save);
	
	fclose(fp_save);
	free(pBuff);
	return;
}
