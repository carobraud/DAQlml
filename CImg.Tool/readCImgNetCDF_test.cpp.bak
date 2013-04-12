#include <iostream>
#include <vector>
#include <string>
  #define cimg_debug         2

#include "../../CImg/useCImg.ini.h"

#include "CImg_NetCDF.h"
/*
#ifndef cimg_debug
  #define cimg_display_type  0
  #define cimg_debug         20
  #include "../CImg/CImg.ini.h"
  using namespace cimg_library;
  #if ( defined(_MSC_VER) && _MSC_VER<=1200 ) || defined(__DMC__)
    #define std
  #endif
#endif
*/
using namespace std;

int main()
{
  //dims
  int nx;
  int ny;
  int nz;
  int nv;
  int nt;

  vector<string> dim_names;
  string time_name="dim_time";
  dim_names.push_back("dim_x1");
  dim_names.push_back("dim_x2");
  dim_names.push_back("dummy1");
  dim_names.push_back("dummy2");
  string var_name="CImg";
  string unit_name="pixel";
/*
  string fo="CImgNetCDF_cimgTest.nc";
  CImgNetCDF<float> cimgTest;
  CImg<float> img;

  cout << "CImgNetCDF::loadNetCDFFile(" << fo << ",...) return " << cimgTest.loadNetCDFFile((char*)fo.c_str()) << endl;
  cout << "CImgNetCDF::loadNetCDFDims(" << fo << ",...) return " << cimgTest.loadNetCDFDims(dim_names,&time_name) << endl;
  cout << "CImgNetCDF::loadNetCDFVar(" << fo << ",...) return "  << cimgTest.loadNetCDFVar(img,var_name,unit_name)  << endl;
  cout << "CImgNetCDF::loadNetCDFData(" << fo << ",...) return " << cimgTest.loadNetCDFData(img)  << endl;
  cout << "img.max()=" << img.max() << endl;
  cout << "CImgNetCDF::loadNetCDFData(" << fo << ",...) return " << cimgTest.loadNetCDFData(img,1) << endl;
  cout << "img.max()=" << img.max() << endl;
*/
  cout << endl;

  vector<string> var_names;
  var_names.push_back("vel_x1");
  var_names.push_back("vel_x2");
  var_names.push_back("vel_x3");
//  var_names.push_back("f");
  vector<string> unit_names;
  unit_names.push_back("gray level");
  unit_names.push_back("gray level");
  unit_names.push_back("gray level");
//  unit_names.push_back("n/a");

  string fo="FP05TST10YZBLK0000-0149.5D1u.nc";
  CImgListNetCDF<float> cimgListTest;
//  CImgList<int> imgList(var_names.size(),nx,ny,nz,nv);
  CImgList<float> imgList;
  cout << "CImgListNetCDF::loadNetCDFFile(" << fo << ",...) return " << cimgListTest.loadNetCDFFile((char*)fo.c_str()) << endl;
  cout << "CImgListNetCDF::loadNetCDFDims(" << fo << ",...) return " << cimgListTest.loadNetCDFDims(dim_names,&time_name) << endl;
  cout << "CImgListNetCDF::loadNetCDFVar(" << fo << ",...) return "  << cimgListTest.loadNetCDFVar(imgList,var_names,unit_names)  << endl;

  for(int t=0;t<1/*cimgListTest.loadDimTime()*/;t++)
  {
    cout << "CImgListNetCDF::loadNetCDFData(" << fo << ",...) return " << cimgListTest.loadNetCDFData(imgList)  << endl;
    cout << "imgList.max()=" << imgList.max() << endl;
  }

  // The file is automatically closed by the destructor. This frees
  // up any internal netCDF resources associated with the file, and
  // flushes any buffers.

  cout << "*** SUCCESS reading example file " << fo << "!" << endl;
  return 0;
}
