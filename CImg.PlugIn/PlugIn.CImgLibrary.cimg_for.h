#ifndef PLUGIN_CIMGLIBRARY_CIMGFOR
#define PLUGIN_CIMGLIBRARY_CIMGFOR

//************************************BEGINING OF THE MACROS ON THE NON-HOMOGENEOUS DIRECTIOPNS*******************************
/**  \def cimg_forXNotHomogeneous(img,is_homogeneous,nx,nx0)
  *
  * img is the image to postprocess \n
  * is_homogeneous is a vector of boolean which informs us about the status of each direction \n 
  * nx is the iterator of the X-direction, nx0 is the start of nx \n
  * cimg_forXNotHomogeneous makes a loop on the X-direction if it isn't homogeneous and makes the treatment only for nx=nx0 if it is homogeneous.
  *
  */
#define cimg_forXifFalse(img,is_homogeneous,nx,nx0) \
        for (int nx=nx0; nx<nx0+((!is_homogeneous[0])?(img.width):1); nx++)

/**  \def cimg_forYNotHomogeneous(img,is_homogeneous,ny,ny0)
  *
  * img is the image to postprocess.\n
  * is_homogeneous is a vector of boolean which informs us about the status of each direction \n 
  * ny is the iterator of the Y-direction, ny0 is the start of ny.\n
  * cimg_forYNotHomogeneous makes a loop on the Y-direction if it isn't homogeneous and make the treatment only for ny=ny0 if it is homogeneous.
  *
  */
#define cimg_forYifFalse(img,is_homogeneous,ny,ny0) \
        for (int ny=ny0; ny<ny0+((!is_homogeneous[1])?(img.height):1); ny++)


/**  \def cimg_forZNotHomogeneous(img,is_homogeneous,nz,nz0)
  *
  * img is the image to postprocess \n
  * is_homogeneous is a vector of boolean which informs us about the status of each direction \n 
  * nz is the iterator of the Z-direction, nz0 is the start of nz.\n
  * cimg_forZNotHomogeneous makes a loop on the Z-direction if it isn't homogeneous and makes the treatment only for nz=nz0 if it is homogeneous.
  *
  */
#define cimg_forZifFalse(img,is_homogeneous,nz,nz0)  \
	for (int nz=nz0; nz<nz0+((!is_homogeneous[2])?(img.depth):1); nz++)

/**  \def cimg_forVNotHomogeneous(img,is_homogeneous,nv,nv0)
  *
  * img is the image to postprocess.\n
  * is_homogeneous is a vector of boolean which informs us about the status of each direction \n 
  * nv is the iterator of the V-direction, nv0 is the start of nv.\n
  * cimg_forVNotHomogeneous makes a loop on the V-direction if it isn't homogeneous and makes the treatment only for nv=nv0 if it is homogeneous.
  *
  */
#define cimg_forVifFalse(img,is_homogeneous,nv,nv0)  \
	for (int nv=nv0; nv<nv0+((!is_homogeneous[3])?(img.dimv()):1); nv++)

/**  \def cimg_forXYZVNotHomogeneous(img,is_homogeneous,nx,ny,nz,nv,nx0,ny0,nz0,nv0)
  *
  * img is the image to postprocess.\n
  * is_homogeneous is a vector of boolean which informs us about the status of each direction \n 
  * n(i) is the iterator of the (i)-direction, nv(i) is the start of n(i).\n
  * cimg_forXYZVNotHomogeneous makes a loop on each direction if it isn't homogeneous \n 
  * 			       makes the treatment only for the start of each direction if it is homogeneous.
  *
  */
#define cimg_forXYZVifFalse(img,is_homogeneous,nx,ny,nz,nv,nx0,ny0,nz0,nv0) \
        cimg_forVifFalse(img,is_homogeneous,nv,nv0) \
        cimg_forZifFalse(img,is_homogeneous,nz,nz0) \
        cimg_forYifFalse(img,is_homogeneous,ny,ny0) \
        cimg_forXifFalse(img,is_homogeneous,nx,nx0)

//***********************************END OF THE MACROS ON THE NON-HOMOGENEOUS DIRECTIONS**************************************

//***********************************BEGINING OF THE MACROS ON THE HOMOGENEOUS DIRECTIONS*************************************




//***********************************END OF THE MACROS ON THE NON-HOMOGENEOUS DIRECTIONS**************************************

//***********************************BEGINING OF THE MACROS ON THE HOMOGENEOUS DIRECTIONS*************************************

/**  \def cimg_forXHomogeneous(img,is_homogeneous,hx,hx0)
  *
  * img is the image to postprocess.\n
  * is_homogeneous is a vector of boolean which informs us about the status of each direction \n 
  * hx is the iterator of the X-direction, hx0 is the start of hx.\n
  * cimg_forXHomogeneous makes a loop on the X-direction if it is homogeneous and makes the treatment only for hx=hx0 if it is'nt homogeneous.
  *
  */
#define cimg_forXifTrue(img,is_homogeneous,hx,hx0) \
	for (int hx=hx0; hx<hx0+((is_homogeneous[0])?(img.width):1); hx++)

/**  \def cimg_forYHomogeneous(img,is_homogeneous,hy,hy0)
  *
  * img is the image to postprocess.\n
  * is_homogeneous is a vector of boolean which informs us about the status of each direction \n 
  * hy is the iterator of the Y-direction, hy0 is the start of hy.\n
  * cimg_forYHomogeneous makes a loop on the Y-direction if it is homogeneous and makes the treatment only for hy=hy0 if it is'nt homogeneous.
  *
  */
#define cimg_forYifTrue(img,is_homogeneous,hy,hy0) \
	for (int hy=hy0; hy<hy0+((is_homogeneous[1])?(img.height):1); hy++)

/**  \def cimg_forZHomogeneous(img,is_homogeneous,hz,hz0)
  *
  * img is the image to postprocess.\n
  * is_homogeneous is a vector of boolean which informs us about the status of each direction \n 
  * hz is the iterator of the Z-direction, hz0 is the start of hz.\n
  * cimg_forZHomogeneous makes a loop on the Z-direction if it is homogeneous and makes the treatment only for hz=hz0 if it is'nt homogeneous.
  *
  */
#define cimg_forZifTrue(img,is_homogeneous,hz,hz0)\
	for (int hz=hz0; hz<hz0+((is_homogeneous[2])?(img.depth):1); hz++)

/**  \def cimg_forVHomogeneous(img,is_homogeneous,hv,hv0)
  *
  * img is the image to postprocess. \n
  * is_homogeneous is a vector of boolean which informs us about the status of each direction \n 
  * hv is the iterator of the V-direction, hv0 is the start of hv.\n
  * cimg_forHomogeneous makes a loop on the V-direction if it is homogeneous and makes the treatment only for hv=hv0 if it is'nt homogeneous.
  *
  */
#define cimg_forVifTrue(img,is_homogeneous,hv,hv0) \
	for (int hv=hv0; hv<hv0+((is_homogeneous[3])?(img.dimv()):1); hv++)

/**  \def cimg_forXYZVHomogeneous(img,is_homogeneous,nx,ny,nz,nv,nx0,ny0,nz0,nv0)
  *
  * img is the image to postprocess.\n
  * is_homogeneous is a vector of boolean which informs us about the status of each direction \n
  * n(i) is the iterator of the (i)-direction, nv(i) is the start of n(i).\n
  * cimg_forXYZVHomogeneous makes a loop on each direction if it is homogeneous \n  
  * 			    makes the treatment only for the start of each direction if it isn't homogeneous.
  *
  */
#define cimg_forXYZVifTrue(img,is_homogeneous,hx,hy,hz,hv,hx0,hy0,hz0,hv0)	\
	cimg_forVifTrue(img,is_homogeneous,hv,hv0) \
	cimg_forZifTrue(img,is_homogeneous,hz,hz0) \
	cimg_forYifTrue(img,is_homogeneous,hy,hy0) \
	cimg_forXifTrue(img,is_homogeneous,hx,hx0) 





/**
  * Same than previously but with a step
  */
#define cimg_forXStepifTrue(img,is_homogeneous,hx,hx0,stp) \
	for (int hx=hx0; hx<hx0+((is_homogeneous[0])?img.width:1); hx+=(((hx+stp>=img.width) && (hx<img.width-1))?img.width-hx-1:stp))

#define cimg_forYStepifTrue(img,is_homogeneous,hy,hy0,stp) \
	for (int hy=hy0; hy<hy0+((is_homogeneous[1])?img.height:1); hy+=(((hy+stp>=img.height) && (hy<img.height-1))?img.height-hy-1:stp))

#define cimg_forZStepifTrue(img,is_homogeneous,hz,hz0,stp)\
	for (int hz=hz0; hz<hz0+((is_homogeneous[2])?img.depth:1); hz+=(((hz+stp>=img.depth) && (hz<img.depth-1))?img.depth-hz-1:stp))
	
#define cimg_forVStepifTrue(img,is_homogeneous,hv,hv0,stp) \
	for (int hv=hv0; hv<hv0+((is_homogeneous[3])?img.dimv():1); hv+=(((hv+stp>=img.dimv()) && (hv<img.dimv()-1))?img.dimv()-hv-1:stp))
	
/*
	for (int hv=hv0,int i=0,int ni=((is_homogeneous[3])?(img.dimv()):1); i<ni; i++,hv+=(i==ni)?img.dimv()-hv-1:stp)
*/
#define cimg_forXYZVStepifTrue(img,is_homogeneous,hx,hy,hz,hv,hx0,hy0,hz0,hv0,stpX,stpY,stpZ,stpV)	\
	cimg_forVStepifTrue(img,is_homogeneous,hv,hv0,stpV) \
	cimg_forZStepifTrue(img,is_homogeneous,hz,hz0,stpZ) \
	cimg_forYStepifTrue(img,is_homogeneous,hy,hy0,stpY) \
	cimg_forXStepifTrue(img,is_homogeneous,hx,hx0,stpX) 




/**
  * Same than previously but with the extented step
  */
#define cimg_forXExtendifTrue(img,is_homogeneous,hx,hx0,ext) \
	for (int hx=hx0; hx<hx0+((is_homogeneous[0])?img.width:1); (hx<ext || hx>(hx0+(is_homogeneous[0]?img.width:1)-ext-2))?(hx++):(hx+=hx0+(is_homogeneous[0]?img.width:1)-2*ext-1))

#define cimg_forYExtendifTrue(img,is_homogeneous,hy,hy0,ext) \
	for (int hy=hy0; hy<hy0+((is_homogeneous[1])?img.height:1);(hy<ext || hy>(hy0+(is_homogeneous[1]?img.height:1)-ext-2))?(hy++):(hy+=hy0+(is_homogeneous[1]?img.height:1)-2*ext-1))

#define cimg_forZExtendifTrue(img,is_homogeneous,hz,hz0,ext)\
	for (int hz=hz0; hz<hz0+((is_homogeneous[2])?img.depth:1); (hz<ext || hz>(hz0+(is_homogeneous[2]?img.depth:1)-ext-2))?(hz++):(hz+=hz0+(is_homogeneous[2]?img.depth:1)-2*ext-1))
	
#define cimg_forVExtendifTrue(img,is_homogeneous,hv,hv0,ext) \
	for (int hv=hv0; hv<hv0+((is_homogeneous[3])?img.dimv():1);(hv<ext || hv>(hv0+(is_homogeneous[3]?img.dimv():1)-ext-2))?(hv++):(hv+=hv0+(is_homogeneous[3]?img.dimv():1)-2*ext-1))

#define cimg_forXYZVExtendifTrue(img,is_homogeneous,hx,hy,hz,hv,hx0,hy0,hz0,hv0,extX,extY,extZ,extV)	\
	cimg_forVExtendifTrue(img,is_homogeneous,hv,hv0,extV) \
	cimg_forZExtendifTrue(img,is_homogeneous,hz,hz0,extZ) \
	cimg_forYExtendifTrue(img,is_homogeneous,hy,hy0,extY) \
	cimg_forXExtendifTrue(img,is_homogeneous,hx,hx0,extX) 


//***********************************END OF THE MACROS ON THE HOMOGENEOUS DIRECTIONS******************************************



#endif/*PLUGIN_CIMGLIBRARY_CIMGFOR*/

