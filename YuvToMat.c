/***
记录YUV格式转opencv Mat结构
----------------------------------
[yuv nv12图像保存颜色偏紫]

如果resize先于cvtColor做：
Mat imgYUV = Mat(NV12_height * 3 / 2, NV12_width, CV_8UC1, NV12_data);//NV12_data存储了YUV数据，为unsigned char*的缓存
resize(imgYUV, imgYUV, Size(imgYUV.cols / 4, imgYUV.rows / 4), 0, 0, INTER_LINEAR);
Mat bgrImage;
cvtColor(imgYUV, bgrImage, CV_YUV2BGR_NV12);
保存的结果图像会偏紫，看上去蓝色或红色都变成了紫色，原因是resize把nv12格式中的U分量和V分量合并了。正确写法如下：
Mat imgYUV = Mat(NV12_height * 3 / 2, NV12_width, CV_8UC1, NV12_data);
Mat bgrImage;
cvtColor(imgYUV, bgrImage, CV_YUV2BGR_NV12);
resize(bgrImage, bgrImage, Size(bgrImage.cols / 4, bgrImage.rows / 4), 0, 0, INTER_LINEAR);
imwrite("/tmp/test.jpg", bgrImage);
**/

/**
JNI Mat

可以在java部分创建一个Mat，用于保存图像处理结果图像，获取Mat 的本地地址传入jni函数中：
// java
Mat res = new Mat();
jni_fun(res.getNativeObjAddr());
c++部分新建Mat指针指向java传入的内存区域，将处理后的结果图像的Mat数据复制到这块内存区域，这样java中的创建的Mat就变为结果图像：

//jni c++
void jni_fun(jlong Mataddr){
   Mat* res = (Mat*)MatAddr;
   Mat image = ImgProcess();
   res->create(image.rows,image.cols,image.type());
   memcpy(res->data,image.data,image.rows*image.step);
}
   
**/

/**

JNIEXPORT int JNICALL Java_com_ProjectName_nativecaller_ClassName_readYUV420SP(JNIEnv *env, jclass clz, 
		jbyteArray yuv,jint len,jint height,jint width){
  
    jbyte * pBuf = (jbyte*)env->GetByteArrayElements(yuv, 0);
 
    Mat image(height + height/2,width,CV_8UC1,(unsigned char *)pBuf);   //注意这里是height+height/2
    Mat mBgr;
    cvtColor(image, mBgr, CV_YUV2BGR_NV21);
    imwrite("/mnt/sdcard/readYuv.jpg",mBgr);
    env->ReleaseByteArrayElements(yuv, pBuf, 0);  
    return 0;
}
**/

/*

如何用OpenCV的Mat类型来存储YUV图像也是经常遇到的问题
对于YUV444图像来说，就很简单。YUV的三个分量的采样方法一致，因此YUV三个分量的大小一致，可以用Mat的三个channel分别表示YUV即可。
假设src是OpenCV默认的BGR三通道图像，和YUV444的转换如下，图像大小不变。

// If src is CV_8UC3, dest is CV_8UC3
cvtColor(src, dest, COLOR_BGR2YUV);
cvtColor(dest, src, COLOR_YUV2BGR);
YUV422用的不多（其实我没用过），先说YUV420。
YUV420图像的U/V分量在水平和垂直方向上downsample，在水平和垂直方向上的数据都只有Y分量的一半。
因此总体来说，U/V分量的数据量分别只有Y分量的1/4，不能作为Mat类型的一个channel。
所以通常YUV420图像的全部数据存储在Mat的一个channel，比如CV_8UC1，这样对于Mat来说，图像的大小就有变化。
对于MxN（rows x cols，M行N列）的BGR图像（CV_8UC3)，其对应的YUV420图像大小是(3M/2)xN（CV_8UC1）。
前MxN个数据是Y分量，后(M/2)xN个数据是U/V分量，UV数据各占一半。

U/V分量如何存储，和YUV420的格式有关。YUV420有所谓的420p（420planar/420面）和420sp（420 semi-planar/420半面）格式。
所谓420面格式，YUV三个分量按顺序存储完一个分量所有图像数据，称为一个面，再存储下一个分量的面，因此有三个面数据。
420半面格式下，只有Y分量是作为一个单独的面存储，U/V分量按照像素排列顺序交错存储，算作一个面，因此称为半面。

            420p                            420sp

   YUV顺序     YVU顺序              UVUV交错      VUVU交错
   I420/IYUV   YV12                 NV12          NV21

420p或者420sp都是先存储Y分量的面，然后根据UV分量的存储顺序，又各分为两种格式。
420p按照YUV的顺序存储三个面，是I420格式，或者叫IYUV格式。按照YVU的顺序存储三个面，叫YV12格式。
420sp的U/V交错面，如果按照UVUV的顺序交错存储，称为NV12格式。反之，按照VUVU的顺序交错存储，称为NV21格式。

OpenCV现在从BGR到YUV420的颜色空间变化仅支持转换到420p的两种格式，不支持转换到420sp。
但可以支持420p或者420sp转换到BGR。假设src是OpenCV默认的BGR三通道图像，和420p的转换如下。

// If src is BGR CV_8UC3 with size 640x960, dest is CV_8UC1 with 960x960
cvtColor(src, dest, COLOR_BGR2YUV_I420);　　　　// dest is I420
cvtColor(dest, src, COLOR_YUV2BGR_I420);

cvtColor(src, dest, COLOR_BGR2YUV_YV12);　　　　// dest is YV12
cvtColor(dest, src, COLOR_YUV2BGR_YV12);

*/

