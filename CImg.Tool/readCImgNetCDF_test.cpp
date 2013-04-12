#include <iostream>
#include <vector>
#include <string>
#define cimg_debug         2

#include "CImg_NetCDF.h"

//useCimg
#include "CImg.h"
using namespace cimg_library;




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

//nom des dim
 vector<string> dim_names;
  string dim_time="dimt";
/*
  dim_names.push_back("dimx");
  dim_names.push_back("dimy");
  dim_names.push_back("dimz");
  dim_names.push_back("dimv");
*/

//nom de veriables
  vector<string> var_names;
  var_names.push_back("u");
  var_names.push_back("v");
  //var_names.push_back("w");
//nom des unites
  vector<string> unit_names;
  unit_names.push_back("pixel");
  unit_names.push_back("pixel");
  //unit_names.push_back("pixel");

  string var_name="u";
  string unit_name="pixel";

/*
  string fo="CImgNetCDF_cimgListTest.nc";
  CImgNetCDF<float> cimgTest;
  CImg<float> img;

  cout << "CImgNetCDF::loadNetCDFFile(" << fo << ",...) return " << cimgTest.loadNetCDFFile((char*)fo.c_str()) << endl;
  cout << "CImgNetCDF::loadNetCDFDims(" << fo << ",...) return " << cimgTest.loadNetCDFDims(dim_names,&dim_time) << endl;
  cout << "CImgNetCDF::loadNetCDFVar(" << fo << ",...) return "  << cimgTest.loadNetCDFVar(img,var_name,unit_name)  << endl;
  cout << "CImgNetCDF::loadNetCDFData(" << fo << ",...) return " << cimgTest.loadNetCDFData(img)  << endl;
  cout << "img.max()=" << img.max() << endl;
  cout << "CImgNetCDF::loadNetCDFData(" << fo << ",...) return " << cimgTest.loadNetCDFData(img,1) << endl;
  cout << "img.max()=" << img.max() << endl;

  cout << endl;
*/


  //string fo="CImgNetCDF_cimgListTest.nc";
  string fo="CImgNetCDF_cimgListTest.4d-1.nc";



  CImgListNetCDF<float> cimgListTest;
//  CImgList<int> imgList(var_names.size(),nx,ny,nz,nv);
  CImgList<float> imgList;
  cout << "CImgListNetCDF::loadNetCDFFile(" << fo << ",...) return " << cimgListTest.loadNetCDFFile((char*)fo.c_str()) << endl;
  //cout << "CImgListNetCDF::loadNetCDFDims(" << fo << ",...) return " << cimgListTest.loadNetCDFDims(dim_names,&dim_time) << endl;	//names known
  cout << "CImgListNetCDF::loadNetCDFDimNames(" << fo << ",...) return " << cimgListTest.loadNetCDFDimNames(var_names[0],dim_names,&dim_time) << endl;
  

  cout << "The variable names are :"<<endl;
  for(int a = 0 ; a < dim_names.size();a++){
  	cout<< dim_names[a]<<endl;
  }


  cout << "CImgListNetCDF::loadNetCDFVar(" << fo << ",...) return "  << cimgListTest.loadNetCDFVar(imgList,var_names,unit_names)  << endl;
/*
  for(int t=0;t<1;t++)//cimgListTest.loadDimTime()
  {
    cout << "CImgListNetCDF::loadNetCDFData(" << fo << ",...) return " << cimgListTest.loadNetCDFData(imgList)  << endl;
    cout << "imgList.max()=" << imgList.max() << endl;
  }
*/
  // The file is automatically closed by the destructor. This frees
  // up any internal netCDF resources associated with the file, and
  // flushes any buffers.

  cout << "*** SUCCESS reading example file " << fo << "!" << endl;
  return 0;

}
