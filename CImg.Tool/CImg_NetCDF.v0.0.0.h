#ifndef CIMG_NETCDF
#define CIMG_NETCDF

#include <iostream>
#include <string>
#include <vector>
#include <netcdfcpp.h>

#include "NetCDFinfo.h"

//#include "../PIVlml/useCImg.h"
/*
#ifndef cimg_debug
  #define cimg_display_type  0
  #define cimg_debug         20
  #include "../../CImg/useCImg.ini.h"
  using namespace cimg_library;
  #if ( defined(_MSC_VER) && _MSC_VER<=1200 ) || defined(__DMC__)
    #define std
  #endif
#endif
*/
#define NC_ERROR   2
#define CODE_ERROR 3
#define DIM_ERROR  4

//! Information on NetCDF type of data
/**
 * Information on NetCDF type of data, implemented types are the following:
 *      \c char,         \c int,         \c float
 * \see NcTypeInfo<char> NcTypeInfo<int> NcTypeInfo<float>
 * \note This list may not be updated, but have a look in \e Class \e Hierarchy of documentation
**/
/*
template<typename T> struct NcTypeInfo {
//! Identifier - it selves (i.e. \c <int>value of \c NcType from \c ncvalues.h) on NetCDF type of data
  static const int ncId() {static const int i=ncNoType; return i;}
//! Identifier as a string (i.e. \c <char*>value of \c NcType from \c ncvalues.h) on NetCDF type of data
  static const char* ncStr() {static const char *const s="ncNoType"; return s;}
};
//! Information on NetCDF type of a \c char data
template<> struct NcTypeInfo<char> {
  static const int ncId() {static const int i=ncChar; return i;}
  static const char* ncStr() {static const char *const s="ncChar"; return s;}
};
//! Information on NetCDF type of an \c int data
template<> struct NcTypeInfo<int> {
  static const int ncId() {static const int i=ncInt; return i;}
  static const char* ncStr() {static const char *const s="ncInt"; return s;}
};
//! Information on NetCDF type of a \c float data
template<> struct NcTypeInfo<float> {
  static const int ncId() {static const int i=ncFloat; return i;}
  static const char* ncStr() {static const char *const s="ncFloat"; return s;}
};
//! Information on NetCDF type of a \c double data
template<> struct NcTypeInfo<double> {
  static const int ncId() {static const int i=ncDouble; return i;}
  static const char* ncStr() {static const char *const s="ncDouble"; return s;}
};
//! Information on NetCDF type of a \c bool data
template<> struct NcTypeInfo<bool> {
  static const int ncId() {static const int i=ncByte; return i;}
  static const char* ncStr() {static const char *const s="ncByte (used as boolean)"; return s;}
};
//! Information on NetCDF type of a \c bool data
template<> struct NcTypeInfo<ncbyte> {
  static const int ncId() {static const int i=ncByte; return i;}
  static const char* ncStr() {static const char *const s="ncByte (used as boolean)"; return s;}
};
//! Information on NetCDF type as a string of any \c NcType
char *NcTypeStr(int ncId)
{
  char *s;
  switch(ncId)
  {
    case ncChar:  s=(char *)(NcTypeInfo<char>::ncStr());  break;
    case ncInt:   s=(char *)(NcTypeInfo<int>::ncStr());   break;
    case ncFloat: s=(char *)(NcTypeInfo<float>::ncStr()); break;
    case ncDouble:s=(char *)(NcTypeInfo<double>::ncStr());break;
    case ncByte:  s=(char *)(NcTypeInfo<bool>::ncStr());  break;
    default: return NULL;
  }
  return s;
}

/*
#if cimg_debug>10 \
#endif \
*/

//! add NetCDF read/write support to CImg class
/**
 * an object of this class may be created or CImg may derived from this class to handle NetCDF
 * \li object in \c .cpp
 * \code
 *   CImgNetCDF<int> object;
 * \endcode
 * \li derivation in \c .h
 * \code
 *   template<typename T> CnewClass: \c CImg<T>, \c CImgNetCDF<T>
 * \endcode
 *
 * \note \c CImgNetCDF and \c CImg must be of the same type
 * \note For this implementation, NetCDF variable dimensions need to be 5: four fixed dimensions (x,y,z,v) and one unlimited dimension (t)
 *
 * \code
 *   
 * \endcode
 *
 * \see CImg
 *
**/
template<typename T> struct CImgNetCDF
{
//variable
  public:
  //! pointer to NetCDF error handling
  /**
   * 
   *
   * \code
   *   NcError err(NcError::verbose_nonfatal);
   * \endcode
   *
   * \see 
   *
  **/
  NcError *pNCError;
  //! pointer to NetCDF file
  /**
   * 
   *
   * \code
   *   string f="2D3C.nc";
   *   NcFile dataFile(f.c_str(),NcFile::ReadOnly);//Read
   *   NcFile dataFile(f.c_str(),NcFile::Replace);//Write
   * \endcode
   *
   * \see 
   *
  **/
  NcFile *pNCFile;
  //! pointers to NetCDF dimensions
  /**
   * 
   *
   * \code
   *   if (!(pDimi=dataFile.get_dim("dim_x"))) return NC_ERROR;//Read
   *   if (!(pDimi=dataFile.add_dim("dim_x",dimi))) return NC_ERROR;//Write
   * \endcode
   *
   * \see loadDimx,y,z,v LOAD_DIM
   *
  **/
  NcDim *pNCDimx,*pNCDimy,*pNCDimz,*pNCDimv;
  //! virtual functions which may return CImg dimensions
  /**
   * To use this functions, CImg may derive from this class.
   *
   * \code
   *   dataFile.dimx()
   * \endcode
   *
   * \see loadDimTime loadDimx,y,z,v from LOAD_DIM checkDimx,y,z,v(CImg) from CHECK_DIM
   *
  **/
#define DIM(X) virtual int dim##X(void) \
               { \
                 std::cerr << "CImgNetCDF::" << __func__ << "()" << std::endl; \
                 return 0; \
               }
  DIM(x)
  DIM(y)
  DIM(z)
  DIM(v)
  //! functions read dimensions from NetCDF file
  /**
   * 
   *
   * \code
   *   dataFile.loadDimx()
   * \endcode
   *
   * \see loadDimTime DIM CHECK_DIM
   *
  **/
//                      std::cerr << "CImgNetCDF::" << __func__ << "()" << std::endl; \

#define LOAD_DIM(X) int loadDim##X(void) \
                    { \
                      if(pNCDim##X==NULL) return CODE_ERROR; \
                      return (int)pNCDim##X->size(); \
                    }
  LOAD_DIM(x)
  LOAD_DIM(y)
  LOAD_DIM(z)
  LOAD_DIM(v)
  //! 
  /**
   * 
   *
   * \code
   *   if (!(pDimt=dataFile.get_dim("time"))) return NC_ERROR;//Read
   *   if (!(pDimt=dataFile.add_dim("time"))) return NC_ERROR;//Write: add an unlimited dimension
   * \endcode
   *
   * \see loadDimTime
   *
  **/
  NcDim *pNCDimt;
  //! pointers to NetCDF data
  /**
   * access to NetCDF data
   *
   * \code
   *   
   * \endcode
   *
  **/
  NcVar *pNCvar;
  //! function read unlimited dimension from NetCDF file
  /**
   * the unlimited time dimension may be a loaded one by one
   *
   * \code
   *   dataFile.loadDimTime()
   * \endcode
   *
   * \see loadDimx,y,z,v from LOAD_DIM
   *
  **/
  int loadDimTime(void)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "()" << std::endl;
#endif
    if(pNCDimt==NULL) {std::cerr << "CImgNetCDF::" << __func__ << "() error: pNCDimt==NULL." << std::endl;return CODE_ERROR;}
    return (int)pNCDimt->size();
  }
//methods
  //! 
  /**
   * 
   *
   * \param  : 
   *
   * \code
   *   
   * \endcode
   *
  **/
  CImgNetCDF(void)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "()" << std::endl;
#endif
    pNCError=new NcError(NcError::verbose_nonfatal);
    pNCFile=NULL;
    pNCDimt=pNCDimv=pNCDimz=pNCDimy=pNCDimx=NULL;
    pNCvar=NULL;
  }
  virtual ~CImgNetCDF(void)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "()" << std::endl;
#endif
//    delete(pNCvar);
//    delete(pNCDimv);
//    delete(pNCDimz);
//    delete(pNCDimy);
//    delete(pNCDimx);
    delete(pNCFile);
    delete(pNCError);
  }
  //! 
  /**
   * 
   *
   * \param  : 
   *
   * \code
   *   
   * \endcode
   *
  **/
  int loadNetCDFFile(char *fileName)//,std::vector<std::string> pDim_names=NULL,std::string *pTime_name=NULL)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "(" << fileName << "," << std::endl;//;
//  std::cerr << "," << ((pTime_name==NULL)?"default":(*pTime_name).c_str()) << ")" << std::endl;
#endif
    pNCFile=new NcFile(fileName,NcFile::ReadOnly);
//    if(pDim_names==NULL)
      return !(pNCFile->is_valid());
//    else
//      return loadNetCDFDims(*pDim_names,pTime_name);
  }
  //! 
  /**
   * 
   *
   * \param  : 
   *
   * \code
   *   
   * \endcode
   *
  **/
  int saveNetCDFFile(char *fileName,std::vector<std::string> *pDim_names=NULL,std::string *pTime_name=NULL)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "(" << fileName << ",";
//  std::cerr << ((dimi_name==NULL)?"default":dimi_name) << "," << ((dimj_name==NULL)?"default":dimj_name) << ",";
  std::cerr << ((pTime_name==NULL)?"":*pTime_name) << ")" << std::endl;
#endif
    pNCFile=new NcFile(fileName,NcFile::Replace);
    if( (pDim_names!=NULL) && (pTime_name!=NULL)) return addNetCDFDims(*pDim_names,*pTime_name);
    else return 0;
  }
  //! 
  /**
   * 
   *
   * \param  : 
   *
   * \code
   *   
   * \endcode
   *
  **/
  int setNetCDFFile(NcFile *pNetCDFFile)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "(" << pNetCDFFile << ")" << std::endl;
#endif
    pNCFile=pNetCDFFile;
    return 0;
  }
  //! add variables in NetCDF file
  /**
   * save variables in NetCDF file, may be called after creation of dimensions \c CImgNetCDF::addNetCDFDims
   *
   * \code
   *   dimv=2;char *fileName="2D2C.nc";
   *   C2DnCNetCDFvelocity vel;//this class MUST be derivated as dimi(), dimj() and dimv() are VIRTUAL
   *   vel.pNCFile=new NcFile(fileName,NcFile::Replace);
   *   vel.addNetCDFDims("dimX","dimY","time");
   *   std::vector<std::string> var_names(dimv+1);
   *   var_names[0]="vel_X";var_names[1]="vel_Y";
   *   var_names[2]="flag";
   *   std::vector<std::string> unit_names(2);
   *   unit_names[0]="m/s";unit_names[1]="none";
   *   addNetCDFVar(&var_names,&unit_names);
   * \endcode
   *
   * \see CImgNetCDF::addNetCDFDims
   *
  **/
  int addNetCDFVar(std::string var_name,std::string unit_name)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "(" << var_name << "," << unit_name << ")" << std::endl;
#endif
    //var component
    ///create NC pointers
    if (!(pNCvar=pNCFile->add_var(var_name.c_str(),(NcType)NcTypeInfo<T>::ncId(),pNCDimt,pNCDimv,pNCDimz,pNCDimy,pNCDimx))) return NC_ERROR;
    //Define units attributes for data variables.
    if (!(pNCvar->add_att("units",unit_name.c_str()))) return NC_ERROR;
    return 0;
  }
  //! add variables in NetCDF file
  /**
   * save variables in NetCDF file, may be called after creation of dimensions \c CImgNetCDF::addNetCDFDims
   *
   * \code
   *   dimv=2;char *fileName="2D2C.nc";
   *   C2DnCNetCDFvelocity vel;//this class MUST be derivated as dimi(), dimj() and dimv() are VIRTUAL
   *   vel.pNCFile=new NcFile(fileName,NcFile::Replace);
   *   vel.addNetCDFDims("dimX","dimY","time");
   *   std::vector<std::string> var_names(dimv+1);
   *   var_names[0]="vel_X";var_names[1]="vel_Y";
   *   var_names[2]="flag";
   *   std::vector<std::string> unit_names(2);
   *   unit_names[0]="m/s";unit_names[1]="none";
   *   addNetCDFVar(&var_names,&unit_names);
   * \endcode
   *
   * \see CImgNetCDF::addNetCDFDims
   *
  **/
//  int addNetCDFVar(CImg &img,std::string var_name,std::string unit_name,std::vector<std::string> dim_names,std::string time_name)
  int addNetCDFVar(CImg<T> &img,std::string var_name,std::string unit_name)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "(CImg<" << img.pixel_type() << ">," << var_name << "," << unit_name << ")" << std::endl;
#endif
    //check dims
    if(loadDimx()!=img.dimx()) {return DIM_ERROR;}
    if(loadDimy()!=img.dimy()) {return DIM_ERROR;}
    if(loadDimz()!=img.dimz()) {return DIM_ERROR;}
    if(loadDimv()!=img.dimv()) {return DIM_ERROR;}
    //var component
    return addNetCDFVar(var_name,unit_name);
  }
  //! add variable data in NetCDF file
  /**
   * add variable data in NetCDF file, may be called after creation of both dimensions \c CImgNetCDF::addNetCDFDims and variables \c CImgNetCDF::addNetCDFVar 
   *
   * \code
   *   
   * \endcode
   *
   * \see CImgNetCDF::addNetCDFDims CImgNetCDF::addNetCDFVar
   *
  **/
/*  template<typename T> T* ptr(void)
  {
    return NULL;
  }*///###
  virtual int addNetCDFData(void)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "()" << std::endl;
#endif
    if(pNCvar==NULL) return CODE_ERROR;
    int time=loadDimTime();
    T *ptr=NULL;//=(*this).ptr();
    if (!pNCvar->put_rec(ptr,time)) return NC_ERROR;
    return 0;
  }
  int addNetCDFData(CImg<T> &img,int time=-1)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "(CImg<" << img.pixel_type() << ">,";
  std::cerr << time << " (if -1 append time))" << std::endl;
#endif
    if(pNCvar==NULL) return CODE_ERROR;
    //check dims
    if(loadDimx()!=img.dimx()) {return CODE_ERROR;}
    if(loadDimy()!=img.dimy()) {return CODE_ERROR;}
    if(loadDimz()!=img.dimz()) {return CODE_ERROR;}
    if(loadDimv()!=img.dimv()) {return CODE_ERROR;}
    //set time if needed (append mode)
    if(time==-1) time=loadDimTime();
    //write data
    T *ptr=img.ptr();
    if (!pNCvar->put_rec(ptr,time)) return NC_ERROR;
    return 0;
  }
  //! load variable dimension from NetCDF file
  /**
   * load from NetCDF file a certain number of variables from names, that will set the variable dimension
   *
   * \code
   *   
   * \endcode
   *
  **/
  int loadNetCDFVar(CImg<T> &img,std::string &var_name,std::string &pUnit_name=NULL)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "(CImg,string,string)" << std::endl;
#endif
    ///set default var names in needed
    //###
    ///get variable
    if (!(pNCvar=pNCFile->get_var(var_name.c_str()))) return NC_ERROR;
    ///check dims//###
    if( pNCvar->num_dims()!=4 && pNCvar->num_dims()!=5 )
    {
      std::cerr << "CImgNetCDF::" << __func__ << "(CImg," << var_name << ",###) num_dims()=" << pNCvar->num_dims() << "." << std::endl;
      std::cerr << "It should have 4 dimensions or 5 with an unlimited one." << std::endl;
      return NC_ERROR;
    }
    ///set CImg dimensions if needed
    if(img.is_empty()) img.assign(loadDimx(),loadDimy(),loadDimz(),loadDimv());
    ///get its unit
    //###
    return 0;
  }
  //! load dimensions from NetCDF file
  /**
   * load variable dimensions from NetCDF file
   *
   * \param  : 
   *
   * \code
   *   
   * \endcode
   *
  **/
  int loadNetCDFDims(std::vector<std::string> dim_names,std::string *pTime_name=NULL)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "(";
  std::cerr << "," << ((pTime_name==NULL)?"default":(*pTime_name).c_str()) << ")" << std::endl;
#endif
    //check requested number of dimensions //###
    if(dim_names.size()!=4) std::cerr << "CImgNetCDF::" << __func__ << "(" << std::endl;
    //set default dim names in needed
    //###
    //get dims
    if (!(pNCDimx=pNCFile->get_dim(dim_names[0].c_str()))) return NC_ERROR;//Read
    if (!(pNCDimy=pNCFile->get_dim(dim_names[1].c_str()))) return NC_ERROR;//Read
    if (!(pNCDimz=pNCFile->get_dim(dim_names[2].c_str()))) return NC_ERROR;//Read
    if (!(pNCDimv=pNCFile->get_dim(dim_names[3].c_str()))) return NC_ERROR;//Read
    if(pTime_name!=NULL)
      if (!(pNCDimt=pNCFile->get_dim((*pTime_name).c_str())))        return NC_ERROR;//Read
    //dims can be now loaded using loadDim[x,y,z,v,Time]() methods
    return 0;
  }
  //! check if NetCDF \c variable and \c CImg dims are the same
  /**
   * check if NetCDF \c variable and \c CImg dims are the same
   *
   * \arg img: \c CImg image dimensions to check
   *
   * \code
   *   CImg<int> img(1,2,3,4);CImgNetCDF<int> fp("data.nc");
   *   if( fp.checkDimx(img) || fp.checkDimy(img) || fp.checkDimz(img) || fp.checkDimv(img) ) exit(1);
   * \endcode
   *
   * \see DIM LOAD_DIM
   *
  **/
#define CHECK_DIM(X) int checkDim##X(CImg<T> img) \
                          { \
                            std::cerr << "CImgNetCDF::" << __func__ << "(CImg)" << std::endl; \
                            if(pNCDim##X==NULL) return CODE_ERROR; \
                            return img.dim##X()!=(int)pNCDim##X->size(); \
                          }
  CHECK_DIM(x)
  CHECK_DIM(y)
  CHECK_DIM(z)
  CHECK_DIM(v)
  //! load variable data from NetCDF file
  /**
   * load from NetCDF file variable data at a time
   *
   * \arg img: load data values in this \c CImg
   * \arg time: get data at time \c time (must be in the range 0 to < \c loadDimTime())
   *
   * \code
   *   
   * \endcode
   *
  **/
  int loadNetCDFData(CImg<T> &img,int time=0)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "(CImg,time=" << time << ")" << std::endl;
#endif
#if cimg_debug>0
    ///check dimensions
    if( checkDimx(img) || checkDimy(img) || checkDimz(img) || checkDimv(img) )
    {
      std::cerr << "CImgNetCDF::" << __func__ << "(CImg," << time << ") error: dimensions differs";
      std::cerr << " (NetCDF:" << loadDimx() << ","<< loadDimy() << "," << loadDimz() << "," << loadDimv() << "," << loadDimy();
      std::cerr << " ;CImg:" << img.dimx() << "," << img.dimy() << "," << img.dimz() << "," << img.dimv();
      std::cerr << ")" << std::endl;
      return DIM_ERROR;
    }
#endif
    ///get component data
    if (!pNCvar->set_cur(time,0,0,0,0)) return NC_ERROR;
    if (!pNCvar->get(img.ptr(0,0,0,0), 1,loadDimv(),loadDimz(),loadDimy(),loadDimx())) return NC_ERROR;
    return 0;
  }
  int addNetCDFDims(std::vector<std::string> dim_names,std::string time_name)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "(";
//  std::cerr << ((dim_names==NULL)?"default":dimi_name) << "," << ((dimj_name==NULL)?"default":dimj_name) << ",";
  std::cerr << time_name << ")" << std::endl;
#endif
/*    if(dim_names[0]==NULL) dimi_name="dim_x";
    if(dim_names[1]==NULL) dimj_name="dim_y";
    if(dim_names[2]==NULL) dimj_name="dim_z";
    if(dim_names[3]==NULL) dimj_name="dim_v";
#####*/
//    if(time_name==NULL) time_name="time";
    if(!(pNCDimx=pNCFile->add_dim(dim_names[0].c_str(),dimx()))) return NC_ERROR;//Write
    if(!(pNCDimy=pNCFile->add_dim(dim_names[1].c_str(),dimy()))) return NC_ERROR;//Write
    if(!(pNCDimz=pNCFile->add_dim(dim_names[2].c_str(),dimz()))) return NC_ERROR;//Write
    if(!(pNCDimv=pNCFile->add_dim(dim_names[3].c_str(),dimv()))) return NC_ERROR;//Write
    if(!(pNCDimt=pNCFile->add_dim(time_name.c_str()/*unlimited*/))) return NC_ERROR;//Write: add an unlimited dimension
    return 0;
  }
  int addNetCDFDims(CImg<T> &img,std::vector<std::string> dim_names,std::string time_name)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "(CImg<" << img.pixel_type() << ">,";
//  std::cerr << ((dim_names==NULL)?"default":dimi_name) << "," << ((dimj_name==NULL)?"default":dimj_name) << ",";
  std::cerr << time_name << ")" << std::endl;
#endif
/*    if(dim_names[0]==NULL) dimi_name="dim_x";
    if(dim_names[1]==NULL) dimj_name="dim_y";
    if(dim_names[2]==NULL) dimj_name="dim_z";
    if(dim_names[3]==NULL) dimj_name="dim_v";
#####*/
//    if(time_name==NULL) time_name="time";
    if(!(pNCDimx=pNCFile->add_dim(dim_names[0].c_str(),img.dimx()))) return NC_ERROR;//Write
    if(!(pNCDimy=pNCFile->add_dim(dim_names[1].c_str(),img.dimy()))) return NC_ERROR;//Write
    if(!(pNCDimz=pNCFile->add_dim(dim_names[2].c_str(),img.dimz()))) return NC_ERROR;//Write
    if(!(pNCDimv=pNCFile->add_dim(dim_names[3].c_str(),img.dimv()))) return NC_ERROR;//Write
    if(!(pNCDimt=pNCFile->add_dim(time_name.c_str()/*unlimited*/))) return NC_ERROR;//Write: add an unlimited dimension
    return 0;
  }
  //! 
  /**
   * 
   *
   * \param  : 
   *
   * \code
   *   
   * \endcode
   *
  **/
  int setNetCDFDims(NcDim *pNetCDFDimx,NcDim *pNetCDFDimy=NULL,NcDim *pNetCDFDimz=NULL,NcDim *pNetCDFDimv=NULL,NcDim *pNetCDFDimt=NULL)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "(" << pNetCDFDimx << "," << pNetCDFDimy  << "," << pNetCDFDimz << "," << pNetCDFDimv << "," << pNetCDFDimt << ")" << std::endl;
#endif
    pNCDimx=pNetCDFDimx;
    pNCDimy=pNetCDFDimy;
    pNCDimy=pNetCDFDimz;
    pNCDimy=pNetCDFDimv;
    pNCDimt=pNetCDFDimt;
    return 0;
  }
};

//CImgNetCDF_test defines
#define DIM(X) \
  int dim##X(void) \
  { \
    int i=(std::string(#X))[0]-116; \
    std::cerr << "CImgNetCDF_test::" << __func__ << "() static return " << i << std::endl; \
    return i; \
  }

//! 
/**
 * 
 *
 * \code
 *   
 * \endcode
 *
 * \see 
 *
**/
template<typename T> struct CImgNetCDF_test: CImgNetCDF<T>
{
  public:
  //new dimx(), dimy(), dimz() and dimv()
  DIM(x)
  DIM(y)
  DIM(z)
  DIM(v)
  //call C2DNetCDF::saveNetCDFFile
  int saveNetCDFFile(char *fileName,std::vector<std::string>dim_names,std::string time_name)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF_test::" << __func__ << "(" << fileName << "," ;
  std::cerr << time_name << ")" << std::endl;
#endif
    return CImgNetCDF<T>::saveNetCDFFile(fileName,&dim_names,&time_name);
  }
  //! add variable data in NetCDF file
  /**
   * add variable data in NetCDF file, may be called after creation of both dimensions \c CImgNetCDF::addNetCDFDims and variables \c CImgNetCDF::addNetCDFVar 
   *
   * \code
   *   
   * \endcode
   *
   * \see CImgNetCDF::addNetCDFDims CImgNetCDF::addNetCDFVar
   *
  **/
  int addNetCDFData(void)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF::" << __func__ << "()" << std::endl;
#endif
    if((*this).pNCvar==NULL) return CODE_ERROR;
    int time=CImgNetCDF<T>::loadDimTime();
    //make dummy data
    int dim=dimx()*dimy()*dimz()*dimv();
    T *ptr=(T*)malloc(sizeof(T)*dim);
    for(int i=0;i<dim;i++) ptr[i]=(T)i;
    //write
    if (!((*this).pNCvar)->put_rec(ptr,time)) return NC_ERROR;
    return 0;
  }
  //call C2DNetCDF::loadFile
  int loadFile(char *fileName,char *dimi_name=NULL,char *dimj_name=NULL,char *time_name=NULL)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF_test::" << __func__ << "(" << fileName << "," << ((dimi_name==NULL)?"default":dimi_name) << "," << ((dimj_name==NULL)?"default":dimj_name) << "," << ((time_name==NULL)?"default":time_name) << ")" << std::endl;
#endif
    CImgNetCDF<T>::loadFile(fileName,dimi_name,dimj_name,time_name);
    //then may use loadDimi(),loadDimj(),loadDimTime() to allocate
  }
  //add function
  void print(void)
  {
#if cimg_debug>10
  std::cerr << "CImgNetCDF_test::" << __func__ << "()" << std::endl;
#endif
    std::fprintf(stdout,"dimx=%d,dimy=%d,dimz=%d,dimv=%d,dimt=%d\n",dimx(),dimy(),dimz(),dimv(),CImgNetCDF<T>::loadDimTime());
  }
};

//! add NetCDF read/write support to CImg class
/**
 * an object of this class may be created or CImg may derived from this class to handle NetCDF
 * \li object in \c .cpp
 * \code
 *   CImgNetCDF<int> object;
 * \endcode
 * \li derivation in \c .h
 * \code
 *   template<typename T> CnewClass: \c CImg<T>, \c CImgNetCDF<T>
 * \endcode
 *
 * \note \c CImgNetCDF and \c CImg must be of the same type
 * \note For this implementation, NetCDF variable dimensions need to be 5: four fixed dimensions (x,y,z,v) and one unlimited dimension (t)
 *
 * \code
 *   
 * \endcode
 *
 * \see CImg
 *
**/
template<typename T> struct CImgListNetCDF: CImgNetCDF<T>
{
//variable
  public:
  //! pointers to NetCDF data
  /**
   * access to NetCDF data
   *
   * \code
   *   
   * \endcode
   *
  **/
  std::vector<NcVar*> pNCvars;
  //! 
  /**
   * 
   *
   * \param  : 
   *
   * \code
   *   
   * \endcode
   *
  **/
  int addNetCDFDims(CImgList<T> &imgs,std::vector<std::string> dim_names,std::string time_name)
  {
#if cimg_debug>10
  std::cerr << "CImgListNetCDF::" << __func__ << "(CImgList<" << imgs.pixel_type() << ">(" << imgs.size << "),";
//  std::cerr << ((dim_names==NULL)?"default":dimi_name) << "," << ((dimj_name==NULL)?"default":dimj_name) << ",";
  std::cerr << time_name << ")" << std::endl;
#endif
    //check sizes
    if(imgs.size<1) {return DIM_ERROR;}
    return CImgNetCDF<T>::addNetCDFDims(imgs(0),dim_names,time_name);
  }
  //! add variables in NetCDF file
  /**
   * save variables in NetCDF file, may be called after creation of dimensions \c CImgNetCDF::addNetCDFDims
   *
   * \code
   *   dimv=2;char *fileName="2D2C.nc";
   *   C2DnCNetCDFvelocity vel;//this class MUST be derivated as dimi(), dimj() and dimv() are VIRTUAL
   *   vel.pNCFile=new NcFile(fileName,NcFile::Replace);
   *   vel.addNetCDFDims("dimX","dimY","time");
   *   std::vector<std::string> var_names(dimv+1);
   *   var_names[0]="vel_X";var_names[1]="vel_Y";
   *   var_names[2]="flag";
   *   std::vector<std::string> unit_names(2);
   *   unit_names[0]="m/s";unit_names[1]="none";
   *   addNetCDFVar(&var_names,&unit_names);
   * \endcode
   *
   * \see CImgNetCDF::addNetCDFDims
   *
  **/
  int addNetCDFVar(CImgList<T> &imgs,std::vector<std::string> var_names,std::vector<std::string> unit_names)
  {
#if cimg_debug>10
  std::cerr << "CImgListNetCDF::" << __func__ << "(CImgList<" << imgs.pixel_type() << ">(" << imgs.size << "),";
// << var_names << "," << unit_names
   std::cerr << ")" << std::endl;
#endif
    //check sizes
    if(imgs.size<1) {return DIM_ERROR;}
    if(var_names.size()!=unit_names.size()) {return DIM_ERROR;}
    //check dims
    if((*this).loadDimx()!=imgs(0).dimx()) {return DIM_ERROR;}
    if((*this).loadDimy()!=imgs(0).dimy()) {return DIM_ERROR;}
    if((*this).loadDimz()!=imgs(0).dimz()) {return DIM_ERROR;}
    if((*this).loadDimv()!=imgs(0).dimv()) {return DIM_ERROR;}
    //allocates
    pNCvars.assign(imgs.size,NULL);
    //var component
    for(int i=0;i<pNCvars.size();i++)
    {
      if((*this).CImgNetCDF<T>::addNetCDFVar(var_names[i],unit_names[i])) {return NC_ERROR;}
      pNCvars[i]=(*this).pNCvar;(*this).pNCvar=NULL;
    }
    return 0;
  }
  //! add variable data in NetCDF file
  /**
   * add variable data in NetCDF file, may be called after creation of both dimensions \c CImgNetCDF::addNetCDFDims and variables \c CImgNetCDF::addNetCDFVar 
   *
   * \code
   *   
   * \endcode
   *
   * \see CImgListNetCDF::addNetCDFDims CImgListNetCDF::addNetCDFVar
   *
  **/
  int addNetCDFData(CImgList<T> &imgs,int time=-1)
  {
#if cimg_debug>10
  std::cerr << "CImgListNetCDF::" << __func__ << "(CImgList<" << imgs.pixel_type() << ">(" << imgs.size << ")";
  std::cerr << ",time=" << time << " (if -1 append time))" << std::endl;
#endif
    if(pNCvars.empty()) return CODE_ERROR;
    //check sizes
    if(imgs.size!=pNCvars.size()) {return DIM_ERROR;}
    //set time if needed (append mode)
    if(time==-1) time=(*this).CImgNetCDF<T>::loadDimTime();
    //var component
    for(int i=0;i<pNCvars.size();i++)
    {
      //write data
      (*this).pNCvar=pNCvars[i];//set current variable data pointer
      if((*this).CImgNetCDF<T>::addNetCDFData(imgs[i],time)) {return NC_ERROR;}
      (*this).pNCvar=NULL;
    }
    return 0;
  }
  //! load variable dimension from NetCDF file
  /**
   * load from NetCDF file a certain number of variables from names, that will set the variable dimension
   *
   * \code
   *   
   * \endcode
   *
  **/
  int loadNetCDFVar(CImgList<T> &imgs,std::vector<std::string> &var_names,std::vector<std::string> &unit_names)
  {
#if cimg_debug>10
  std::cerr << "CImgListNetCDF::" << __func__ << "(CImgList<" << imgs.pixel_type() << ">(" << imgs.size << ")";
  std::cerr << ",vector<string>,vector<string>)" << std::endl;
#endif
    //allocates
    pNCvars.assign(var_names.size(),NULL);
    //allocates if needed
    if(imgs.is_empty()) {imgs.assign(pNCvars.size(),(*this).loadDimx(),(*this).loadDimy(),(*this).loadDimz(),(*this).loadDimv());}
    //var component
    for(int i=0;i<pNCvars.size();i++)
    {
      //load variable and allocates if needed
      if((*this).CImgNetCDF<T>::loadNetCDFVar(imgs[i],var_names[i],unit_names[i])) {return NC_ERROR;}//### pUnit_names
      pNCvars[i]=(*this).pNCvar;//set current variable data pointer
      (*this).pNCvar=NULL;
    }
    return 0;
  }
  //! load variable data from NetCDF file
  /**
   * load from NetCDF file variable data at a time
   *
   * \arg img: load data values in this \c CImg
   * \arg time: get data at time \c time (must be in the range 0 to < \c loadDimTime())
   *
   * \code
   *   
   * \endcode
   *
  **/
  int loadNetCDFData(CImgList<T> &imgs,int time=0)
  {
#if cimg_debug>10
  std::cerr << "CImgListNetCDF::" << __func__ << "(CImgList<" << imgs.pixel_type() << ">(" << imgs.size << ")";
  std::cerr << ",time=" << time << ")" << std::endl;
#endif
    if(pNCvars.empty()) return CODE_ERROR;
    //check sizes
    if(imgs.size!=pNCvars.size()) {return DIM_ERROR;}
    //var component
    for(int i=0;i<pNCvars.size();i++)
    {
      //write data
      (*this).pNCvar=pNCvars[i];//set current variable data pointer
      if((*this).CImgNetCDF<T>::loadNetCDFData(imgs[i],time)) {return NC_ERROR;}
      (*this).pNCvar=NULL;
    }
    return 0;
  }
};

#endif/*CIMG_NETCDF*/

