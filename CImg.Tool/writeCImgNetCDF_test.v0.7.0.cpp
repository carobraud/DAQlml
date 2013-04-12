#include <iostream>
#include <vector>
#include <string>
#include <netcdfcpp.h>

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
// We are writing 5D data, a 6 x 12 y-x grid, with 2
// timesteps of data.
  static const int nx = 12;
  static const int ny = 6;
  static const int nz = 4;
  static const int nv = 3;
  static const int nt = 2;

 

/*
  string fo="CImgNetCDF_test.nc";
  CImgNetCDF_test<float> test;
  cout << "CImgNetCDF_test::saveNetCDFFile(" << fo << ",...) return " << test.saveNetCDFFile((char*)fo.c_str(),dim_names,dim_time) << endl;
  test.print();
  cout << "CImgNetCDF_test::addNetCDFVar(" << fo << ",...) return " << test.addNetCDFVar(var_name,unit_name) << endl;
  cout << "CImgNetCDF_test::addNetCDFData" << fo << ",...) return " << test.addNetCDFData() << endl;
  cout << "CImgNetCDF_test::addNetCDFData" << fo << ",...) return " << test.addNetCDFData() << endl;
  cout << endl;
*/
/*
  string fo="CImgNetCDF_cimgTest.nc";
  CImgNetCDF<float> cimgTest;
  CImg<float> img(nx,ny,nz,nv);
  cout << "CImgNetCDF_cimgTest::saveNetCDFFile(" << fo << ",...) return " << cimgTest.saveNetCDFFile((char*)fo.c_str()) << endl;
  cout << "CImgNetCDF_cimgTest::addNetCDFDims(" << fo << ",...) return " << cimgTest.addNetCDFDims(img,dim_names,dim_time) << endl;
  cout << "CImgNetCDF_cimgTest::addNetCDFVar(" << fo << ",...) return " << cimgTest.addNetCDFVar(img,var_name,unit_name) << endl;
  cimg_forXYZV(img,x,y,z,v) img(x,y,z,v)=x*y*z*v;
  cout << "CImgNetCDF_cimgTest::addNetCDFData" << fo << ",...) return " << cimgTest.addNetCDFData(img) << endl;
  cimg_forXYZV(img,x,y,z,v) img(x,y,z,v)=x*y*z*v+1;
  cout << "CImgNetCDF_cimgTest::addNetCDFData" << fo << ",...) return " << cimgTest.addNetCDFData(img) << endl;
  cout << endl;
*/

//nom des dim
 vector<string> dim_names;
  string dim_time="dimt";
  dim_names.push_back("dimx");
  dim_names.push_back("dimy");
  dim_names.push_back("dimz");
  dim_names.push_back("dimv");
  string var_name="CImg";
  string var_name2="CImg";
  string unit_name="pixel";
//nom de veriables
  vector<string> var_names;
  var_names.push_back("u");
  var_names.push_back("v");
  var_names.push_back("w");
  vector<string> var_names2;
  var_names2.push_back("u2");
  var_names2.push_back("v2");
  var_names2.push_back("w2");
//nom des unites
  vector<string> unit_names;
  unit_names.push_back("pixel");
  unit_names.push_back("pixel");
  unit_names.push_back("pixel");



//dimension 4

  string fo="CImgNetCDF_cimgListTest.4d-1.nc";
  CImgListNetCDF<int> cimgListTest;
  CImgListNetCDF<int> cimgListTest2;
  CImgList<int> imgList(var_names.size(),nx,ny,nz,nv);
  cout << "CImgListNetCDF::saveNetCDFFile(" << fo << ",...) return " 	<< cimgListTest.saveNetCDFFile((char*)fo.c_str()) << endl;
  cout << "CImgListNetCDF::saveNetCDFFile(" << fo << ",...) return " 	<< cimgListTest2.setNetCDFFile(cimgListTest.getNetCDFFile()) << endl;
  cout << "CImgListNetCDF::addNetCDFDims(" << fo << ",...) return " 	<< cimgListTest.addNetCDFDims(imgList,dim_names,dim_time) << endl;
  cout << "CImgListNetCDF::addNetCDFDims(" << fo << ",...) return " 	<< cimgListTest2.setNetCDFDims(cimgListTest.vpNCDim, cimgListTest.pNCDimt) << endl;
  cout << "CImgListNetCDF::addNetCDFVar(" << fo << ",...) return " 	<< cimgListTest.addNetCDFVar(imgList,var_names,unit_names) << endl;
  cout << "CImgListNetCDF::addNetCDFVar(" << fo << ",...) return " 	<< cimgListTest2.addNetCDFVar(imgList,var_names2,unit_names) << endl;

//t=0
  cimglist_for(imgList,l) cimg_forXYZV(imgList(l),x,y,z,v) imgList(l)(x,y,z,v)=x*y*z*v*l;
  cout << "CImgListNetCDF::addNetCDFData" << fo << ",...) return " 	<< cimgListTest.addNetCDFData(imgList) << endl;
  //imgList.pow(2);
  cout << "CImgListNetCDF::addNetCDFData" << fo << ",...) return " 	<< cimgListTest2.addNetCDFData(imgList,cimgListTest.loadDimTime()-1) << endl;

//t=1
  cimglist_for(imgList,l) cimg_forXYZV(imgList(l),x,y,z,v) imgList(l)(x,y,z,v)=(x*y*z*v+1)*(l+1);
  cout << "CImgListNetCDF::addNetCDFData" << fo << ",...) return " 	<< cimgListTest.addNetCDFData(imgList) << endl;
  //imgList.pow(2);
  cout << "CImgListNetCDF::addNetCDFData" << fo << ",...) return " 	<< cimgListTest2.addNetCDFData(imgList,cimgListTest.loadDimTime()-1) << endl;

  cout << endl;



//dimension 3
/*
  string fo="CImgNetCDF_cimgListTest.nc";
  CImgListNetCDF<int> cimgListTest;
  CImgList<int> imgList(var_names.size(),nx,ny,nz);
  cout << "CImgListNetCDF::saveNetCDFFile(" << fo << ",...) return " 	<< cimgListTest.saveNetCDFFile((char*)fo.c_str()) << endl;
  cout << "CImgListNetCDF::addNetCDFDims(" << fo << ",...) return " 	<< cimgListTest.addNetCDFDims(imgList,dim_names,dim_time) << endl;
  cout << "CImgListNetCDF::addNetCDFVar(" << fo << ",...) return " 	<< cimgListTest.addNetCDFVar(imgList,var_names,unit_names) << endl;

  cimglist_for(imgList,l) cimg_forXYZ(imgList(l),x,y,z) imgList(l)(x,y,z)=x*y*z*l;

  cout << "CImgListNetCDF::addNetCDFData" << fo << ",...) return " 	<< cimgListTest.addNetCDFData(imgList) << endl;

  cimglist_for(imgList,l) cimg_forXYZ(imgList(l),x,y,z) imgList(l)(x,y,z)=(x*y*z+1)*(l+1);

  cout << "CImgListNetCDF::addNetCDFData" << fo << ",...) return " 	<< cimgListTest.addNetCDFData(imgList) << endl;
  cout << endl;
*/


/*
  fo="CImgNetCDF_test_CImg.nc";CImgNetCDF_cimgListTest.nc
  CImgNetCDF_test_CImg<double> image(nx,ny,nz,nv);
  cout << "CImgNetCDF_test_CImg::saveNetCDFFile(" << fo << ",...) return " << image.saveNetCDFFile((char*)fo.c_str(),dim_names,dim_time) << endl;
  image.print();
  cout << "CImgNetCDF_test_CImg::addNetCDFVar(" << fo << ",...) return " << image.addNetCDFVar(var_name,unit_name) << endl;
  for(int t=0;t<nt;t++)
  {
    int i=t;cimg_forXYZV(image,x,y,z,v) image(x,y,z,v)=i++;
    cout << "CImgNetCDF_test_CImg::addNetCDFData" << fo << ",...) return " << image.addNetCDFData() << endl;
  }
*/
/*
  fo="CImgNetCDF_CImg.nc";
  CImg<float> image(nx,ny,nz,nv);
  cout << "CImg::saveNetCDFFile(" << fo << ",...) return " << image.saveNetCDFFile((char*)fo.c_str(),dim_names,dim_time) << endl;
  image.print();
  cout << "CImg::addNetCDFVar(" << fo << ",...) return " << image.addNetCDFVar(var_name,unit_name) << endl;
  for(int t=0;t<nt;t++)
  {
    int i=t;cimg_forXYZV(image,x,y,z,v) image(x,y,z,v)=i++;
    cout << "CImg::addNetCDFData" << fo << ",...) return " << image.addNetCDFData() << endl;
  }
*/
   // The file is automatically closed by the destructor. This frees
   // up any internal netCDF resources associated with the file, and
   // flushes any buffers.
   cout << "*** SUCCESS writing example file " << fo << "!" << endl;
   return 0;
}
