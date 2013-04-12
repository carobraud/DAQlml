/*Additional documentation for the generation of the reference page (using doxygen)*/
/** \mainpage
 *
 *  index:
 *  \li \ref sectionCommandLine
 *  \li \ref sectionParameterFile
 *  \li \ref sectionDAQlmlDocumentation
 *  \li \ref sectionDoxygenSyntax
 *
 *  \section sectionCommandLine command line options
 *  \verbinclude "DAQlml.help.output"
 *
 *  \section sectionParameterFile CDL parameter file
 *  Use NetCDF library tool \c ncgen to compile \c parameters.cdl text file to \c parameters.nc binary file (e.g. \c ncgen \c parameters.cdl \c -o \c parameters.nc).
 *  \verbinclude "parameters.cdl"
 *
 *  \section sectionDAQlmlDocumentation documentation outline
 *  This is the reference documentation of <a href="http://www.meol.cnrs.fr/">DAQlml</a>, from the <a href="http://www.univ-lille1.fr/lml/">LML</a>.\n\n
 *  DAQlml is a data acquisition software. The program begins in the main function in the <a href="main_8cpp.html">main.cpp</a> source file.\n\n
 *  This documentation has been automatically generated from the DAQlml sources, 
 *  using the tool <a href="http://www.doxygen.org">doxygen</a>. It should be readed as HTML, LaTex and man page.\n
 *  It contains both
 *  \li a detailed description of all classes and functions
 *  \li a user guide (cf. \ref pages.html "related pages")
 *
 *  that as been documented in the sources.
 *
 *  \par Additional needed libraries:
 *
 *  \li <a href="http://cimg.sourceforge.net">the CImg Library</a> using <a href="http://www.imagemagick.org/">ImageMagick</a> for a few image format
 *  \li <a href="http://www.unidata.ucar.edu/software/netcdf/">NetCDF</a> (network Common Data Form)
 *
 *  \par Optional libraries:
 *
 *  \li <a href="http://www.fftw.org/">FFTw</a> (Fastest Fourier Transform in the West) though CImg library
 *  \li added to CImg raw, <a href="http://www.rd-vision.com/">Hiris</a>, <a href="http://www.pco.de/">PCO</a> and <a href="http://www.lavision.de">LaVision</a> images support
 *  \li <a href="http://www.libpng.org/">libPNG</a> (Portable Network Graphics) using <a href="http://www.zlib.net/">zLib</a> (non destructive compression)
 *  \li <a href="http://www.libtiff.org/">libTIFF</a> (Tag Image File Format)
 *  \li parallelism <a href="http://www.open-mpi.org/">MPI</a> (Message Passing Interface)
 *  \li data format <a href="http://www.paraview.org/">ParaView</a>
 *  \li vector graphics <a href="http://libboard.sourceforge.net/">Board</a> (A vector graphics C++ library: Postscript, SVG and Fig files)
 *  \li parallelism <a href="http://www.gnu.org/software/pth/">pThread</a> (POSIX thread)
 *  \li <a href="http://www.boost.org/">Boost.org</a> (aims to C++ Standard Library) <a href="http://www.boost.org/libs/libraries.htm">libs</a>
 *  \li <a href="http://www.oonumerics.org/blitz/">Blitz++</a> (C++ class library for scientific computing)
 *  \li <a href="http://math.nist.gov/tnt/">TNT</a> (Template Numerical Toolkit = PACK of <a href="http://math.nist.gov/lapack++/">Lapack++</a>, <a href="http://math.nist.gov/sparselib++/">Sparselib++</a>, <a href="http://math.nist.gov/iml++/">IML++</a>, and <a href="http://math.nist.gov/mv++/">MV++</a>) , ...
 *
 *  \section sectionDoxygenSyntax make documentation using Doxygen syntax
 *  Each function in the source code should be commented using \b doxygen \b syntax in the same file.
 *  The documentation need to be written before the function.
 *  The basic syntax is presented in this part.
 *  \verbinclude "doxygen.example1.txt"
 *
 *  Two kind of comments are needed for \b declaration and \b explanation \b parts of the function:
 *  Standart documentation should the following (\b sample of code documentation):
 *  \verbinclude "doxygen.example2.txt"
 *
 *  In both declaration and explanation part, \b writting and \b highlithing syntax can be the following:\n\n
 *
 *  \li \c \\n    a new line
 *  \li \c \\li   a list (dot list)
 *
 *  \li \c \\b    bold style
 *  \li \c \\c    code style
 *  \li \c \\e    enhanced style (italic)
 *
 *  For making \b shortcut please use:\n
 *  \li \c \\see to make a shortcut to a related function or variable
 *  \li \c \\link to make a shortcut to a file or a function
 *  \note this keyword needs to be closed using \c \\end*
 *
 *  While coding or debugging, please use intensively:
 *  \li \c \\todo to add a thing to do in the list of <a href="todo.html">ToDo</a> for the whole program
 *  \li \c \\bug to add an \e a \e priori or known bug in the list of <a href="bug.html">Bug</a> for the whole program
 *
 *  In explanation part, \b paragraph style can be the following:\n
 *  \li \c \\code for an example of the function use
 *  \li \c \\note to add a few notes
 *  \li \c \\attention for SOMETHING NOT FULLY DEFINED YET
 *  \li \c \\warning to give a few warning on the function
 *  \note these keywords need to be closed using \c \\end*
 *
 *  \verbinclude "doxygen.example3.txt"
 *
 *  Many other keywords are defined, so please read the documentation of <a href="http://www.doxygen.org/commands.html">doxygen</a>.
 *
 **/

#include <stdio.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>

//debug macro
#define PR(value) std::cerr<<#value<<"="<<value<<std::endl<<std::flush;

//comedi library
#include <comedilib.h>

//NetCDF library and extensions
#include <netcdfcpp.h>
#include "../NetCDF.Tool/struct_parameter_NetCDF.h"
#include "../NetCDF.Tool/NetCDFinfo.h"
#include "../CImg.Tool/CImg_NetCDF.h"

// real time headers
//#ifdef REAL_TIME
#include "../RealTime/RT_PREEMPT.h"
//#endif

//DAQlml headers
#include "DAQcomedi.h"
#include "DAQdata.h"
#include "DAQloop.h"


//process headers
#include "acquisition.h"
#include "control.h"

//test signal
#include "DAQtest.h"

//signal post-processing
//! \todo [very low] signal post-processing adapted from *_flag.h and intensive CImg functions
//#include "statistic.h"
//#include "histogram.h"

//! for time count 
/**
 * One can use this function to measure the actual time (not CPU time).
 * \code
 * double st, en;
 * st = getETime();
 * ----
 * routine to be measured
 * ----
 * en = getETime();
 * std::cout<<"elapsed time: "<<(en-st)<<" sec"<<std::endl;
 * \endcode
 **/
double getETime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

//! main function of the DAQlml program
/**
 * \param [in] argc command line argument number (i.e. string count)
 * \param [in] argv command line argument values (i.e. vector of strings)
 **/
int main(int argc, char *argv[])
{
  const std::string version = "DAQlml v0.4.4: DAta AcQuisition program of LML";
                               
  std::cout<<"----------------------------------------------"<<std::endl;
  std::cout<< version                                        <<std::endl;
  std::cout<<"----------------------------------------------"<<std::endl;

  cimg_usage(version.c_str()); /**< parameters from command line (i.e. options) display program usage*/

  //both declare and get parameters for command line options
  const int  verbose    = (cimg_option("-v",(const char*)NULL,"verbose option")!=NULL);
  const bool show_info  = (cimg_option("--info",(const char*)NULL,"information")!=NULL);
  const bool show_h     = (cimg_option("-h",(const char*)NULL,NULL)!=NULL);
        bool show_help  = (cimg_option("--help",(const char*)NULL,"help (or -h option)")!=NULL);show_help=show_h|show_help;

  ////file names
  const std::string fd  =  cimg_option("--fd","/dev/comedi0","board device file");
  const std::string fp  =  cimg_option("--fp","parameters.nc","input parameter file");
//  const std::string fi  =  cimg_option("--fi","","input data file");
  const std::string fo  =  cimg_option("--fo","data.nc","output data file");
//  const std::string fs  =  cimg_option("--fs","stats.nc","statistics output file");


  ////program behaviours
  //! \todo currently acquire is not used.
//  bool       acquire   =  fi.empty();
  const bool time_axis =  cimg_option("-t",true,"create time axis");
  bool       conv_phys =  cimg_option("-c",true,"convert 16bit int value into voltage");
  const int  test      =  cimg_option("--test",0,"test, 0: no test, 1: square wave test");//, 2: sin wave test");
//  const bool control   =  cimg_option("--control",false,"use of output for control");
  const bool bdinfo    = (cimg_option("--boardinfo",(const char*)NULL,"print board info")!=NULL);
  const int  show      = (cimg_option("--show",0,"display result as a graph, 0: no display 1: data (and histogram on test) 2: + errors 3: + raw data/clean data"));
  const bool buffer  =  cimg_option("--buffer", false,"acquisition type");

  //show help and/or information
  if(show_help) {print_help(std::cerr);      return 0;}
  if(show_info) {cimg::info();               return 0;}
  if(bdinfo)    {get_board_info(fd, bdinfo); return 0;}
  
  //variables for test
  DAQtest DAQt;
  
  if(test==TEST_SQWAVE)  DAQt.test_print_instruction(TEST_SQWAVE);

  // create DAQdevice;
  DAQdevice DAQdev(fd);
  std::cout<<"loading sampling parameters from '"<< fp <<"'."<<std::endl;
  DAQdev.load_parameter(fp);

  void *map;// pointer to mapped memory
  DAQdev.verbose=verbose;
  //initialize acquisition device
  if(buffer)
  {
    DAQdev.config_device_buffer(map);
  }
  else
  {
    //DAQdev.sampling_rate = 1/(DEMIPERIOD*pow(10,-9));
 //std::cout << "\n resultat " <<1/(DEMIPERIOD*pow(10,-9))<< "\n";
	
    DAQdev.config_device_point();
  }
  DAQdev.print();

  //! \todo [low] \c data should be \c sampl_t type (best with template)
  std::cout<<"allocating memory for data."<<std::endl;
  double st, en;
  st=getETime();
  cimg_library::CImgList<int>   data(DAQdev.channel_index.size(),DAQdev.sample_number);
  cimg_library::CImgList<float> data_phys;
  cimg_library::CImgList<float> time;
  en=getETime();
  std::cout<<"elapsed time: "<<(en-st)<<" sec"<<std::endl;

//RT
 // data.fill(99);
  //RT.init

  //control or acquisition loop
  std::cout<<"starting sampling with ";
  st=getETime();
//! \todo currently control is removed.
  if(buffer) sample_data_buffer(data, map, DAQdev);//,control);
  else sample_data_point(data, DAQdev);//,control);
  en=getETime();
  std::cout<<"elapsed time: "<<(en-st)<<" sec"<<std::endl;

  //convert data to data_phys
  if(conv_phys) {
    st=getETime();
    std::cout<<"converting binary data to physical voltage."<<std::endl;
    convert_to_phys(data, data_phys, DAQdev);
data.print("data in main");
data_phys.print("data_phys in main");
    en=getETime();
    std::cout<<"elapsed time: "<<(en-st)<<" sec"<<std::endl;
  }

  //create time axis
  if(time_axis)
    {
      st=getETime();
      std::cout<<"creating time axis."<<std::endl;
      create_time(data, time, DAQdev);
      en=getETime();
      std::cout<<"elapsed time: "<<(en-st)<<" sec"<<std::endl;
    }
  if(verbose) data_phys.print("data physical");

  std::cout<<"printing first 10 samples."<<std::endl;
  for (int j=0; j<10; j++){for (unsigned int i=0; i<DAQdev.channel_index.size(); i++){ if(data_phys[i](j)>=0.0){std::cout<<" ";} std::cout<<std::scientific<<std::setprecision(3)<<data_phys[i](j)<<" "; } std::cout<<std::endl; }

  //check simple statistics, mean, var, min, max
  std::cout<<"computing basic statistics."<<std::endl;
  cimglist_for(data_phys,n){std::cout<<"chan-"<<DAQdev.channel_index[n] << ": " << DAQdev.channel_name[n] << ", mean: "<<data_phys[n].mean() << ", min: "<<data_phys[n].min()<<", max: "<<data_phys[n].max()<<", var: "<<data_phys[n].variance()<<std::endl;}


  ///- test computations (e.g. square or sinus wave tests)
  if(test==TEST_SQWAVE)
    {
      std::cout<<"testing signals by square wave."<<std::endl;  

      DAQt.load_parameter(fp);
//! \todo [high] loop on channels
      /*
      for(i=0;i<data_phys.size();i++)
	{
	  DAQt.test_signal(data_phys[i],DAQdev.sampling_rate,true,false,true,DAQt.reference_frequency,DAQt.reference_DC,DAQt.reference_tolerance);
	}
      */
      DAQt.test_signal(data_phys[0],DAQdev.sampling_rate,DAQt.gaussian_filter,DAQt.DCfrequency,show,DAQt.reference_frequency,DAQt.reference_DC,DAQt.reference_tolerance);     
    }
  
  std::cout<<"saving data into a NetCDF file."<<std::endl;
  st=getETime();
  int acqu_range_min=0,acqu_range_max=DAQdev.maxdata;std::string acqu_range_unit("level");
  if(conv_phys)
    {
      float phys_range_min=DAQdev.comedirange->min,phys_range_max=DAQdev.comedirange->max; std::string phys_range_unit("volt");
      save_data(fo, data_phys, time, DAQdev, "volt", acqu_range_min, acqu_range_max, acqu_range_unit, phys_range_min, phys_range_max, phys_range_unit);
    }
  else
    {
      int   phys_range_min=DAQdev.comedirange->min,phys_range_max=DAQdev.comedirange->max; std::string phys_range_unit("volt");
      save_data(fo, data, time, DAQdev, "16 bit binary", acqu_range_min, acqu_range_max, acqu_range_unit, phys_range_min, phys_range_max, phys_range_unit);
    }
  en=getETime();
  std::cout<<"elapsed time: "<<(en-st)<<" sec"<<std::endl;

  
  std::cout<<"finalizing the device."<<std::endl;
  int ret=comedi_close(DAQdev.dev);
  if(ret<0){
    comedi_perror(DAQdev.filename.c_str());
    exit(1);
  }

  ///- display data on GUI
  //display data
  if(show==1)
    {
      //tranform data as list to display as image
      cimg_library::CImg<float> display;
      if(conv_phys)
	{//floating data (i.e. recorded levels in Volts)
	  cimglist_for(data_phys,n) display.append(data_phys[n]);//concat lines
	  display.assign(data_phys[0].width(),1,1,data_phys.size());//set right dimensions
	}
      else
	{//unsigned interger data (i.e. recorded discrete levels)
	  cimglist_for(data,n) display.append(data[n]);//concat lines
	  display.assign(data[0].width(),1,1,data.size());//set right dimensions
	}
      //display as color graphs by set of 3
      for(int c=0;c<display.spectrum();c+=3)
	{
	  int c0=c,c1=((c+3)<display.spectrum())?(c+2):(display.spectrum()-1);
	  PR(c0);PR(c1);
	  (display.get_shared_channels(c0,c1)).display_graph("channels recorded");
	}
    }//if show
  ///- computation of data statistics, save and display on GUI
  /*
  //later
  if(statistic)
  {
  max_min_mean_rms
  histogram
  if(store_statistic)
  if(GUI_statistic)
  }
  */

  return 0;
}

