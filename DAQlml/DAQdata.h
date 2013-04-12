#ifndef DAQ_DATA
#define DAQ_DATA

//! load previously recorded data
/**
 * load previously recorded data as many channels in a list (i.e. CImgList container)
 * \note all channels should have the same size (i.e. 1D data with same width)
 * \param [in]  file_name file name (e.g. "data.nc")
 * \param [in]  var_names variable names (i.e. the requested list of channels;e.g. "pressure, hot_wire, square_wave, control_signal")
 * \param [out] unit_names variable unit names (i.e. unit names of channels; e.g. "Pa","m/s","V","volt")
 * \param [out] dim_names dimension name (i.e. only one dimension name in the list as channels are 1D data; e.g. "time")
 * \param [out] channels data as physical values (i.e. all requested channels in the data list)
**/
template<typename T> int load_data(std::string file_name,std::vector<std::string> var_names,std::vector<std::string> &unit_names,std::vector<std::string> &dim_names,CImgList<T> &channels)
{
std::cerr<<__FILE__<<"/"<<__func__<<"[data](\""<<file_name<<"\",...)"<<std::endl;
  //NetCDF file object
  CImgListNetCDF<T> fpi;
  ///- check prequisite
  if(file_name.empty())  return -1;
  if(var_names.size()<1) return -2;
  ///- [optional] show dimension; should be one only
  std::cout << "The variable names are:\n";
  for(unsigned int i=0;i<var_names.size();++i)
  {
    std::cout<<var_names[i]<<std::endl;
  }
  ///- open file in read only mode
  std::cout << "CImgListNetCDF::loadNetCDFFile(" << file_name << ",...) return " << fpi.loadNetCDFFile((char*)file_name.c_str()) << std::endl;
  //cout << "CImgListNetCDF::loadNetCDFDims(" << fo << ",...) return " << cimgListTest.loadNetCDFDims(dim_names,&dim_time) << endl;	//names known
  ///load NetCDF header
  ///- load dimension (e.g. "time")
  std::cout << "CImgListNetCDF::loadNetCDFDimNames(" << file_name << ",...) return " << fpi.loadNetCDFDimNames(var_names[0],dim_names,NULL) << std::endl;
  ///- [optional] show dimension; should be one only
  std::cout << "The dimension names are:"<<std::endl;
  for(unsigned int i=0;i<dim_names.size();++i)
  {
    std::cout<<dim_names[i]<<std::endl;
  }
  ///load NetCDF header
  std::cout << "CImgListNetCDF::loadNetCDFVar(" << file_name << ",...) return "  << fpi.loadNetCDFVar(channels,var_names,unit_names)  << std::endl;
  ///load NetCDF data
  std::cout << "CImgListNetCDF::loadNetCDFData(" << file_name << ",...) return " << fpi.loadNetCDFData(channels)  << std::endl;
  return 0;
}
//! load previously recorded data information
/**
 * load previously recorded data as many channels in a list (i.e. CImgList container)
 * \note all channels should have the same size (i.e. 1D data with same width)
 * \param [in]  file_name file name (e.g. "data.nc")
 * \param [out] ...
**/
template <typename Tdata, /*typename Ttime,*/ typename Tacqu>
int load_data_attribute(std::string file_name,std::string var_name,int &sampling_rate,
  Tacqu &acqu_range_min, Tacqu &acqu_range_max, std::string &acqu_range_unit,
  Tdata &phys_range_min, Tdata &phys_range_max, std::string &phys_range_unit)
{
std::cerr<<__FILE__<<"/"<<__func__<<"[information](\""<<file_name<<"\",...)"<<std::endl;
  //NetCDF data file object (i.e. parameter class)
  CParameterNetCDF fp;
  ///open file from its name
  //open file
  int error=fp.loadFile((char *)file_name.c_str());
  if(error){std::cerr<<"loadFile return "<< error <<std::endl;return error;}
  ///read variable
  Tdata data;
  std::string process_name="acquisition";
  if(error=fp.loadVar(data,&var_name)){std::cerr<<"Error: data variable \""<<var_name<<"\" can not be loaded (return value is "<<error<<")\n";return error;}
  ///load attributes (i.e. data information)
  ///- data attribute ranges
  std::string range_name("physical_range");
  std::vector<Tdata> phys_range(2);
  fp.loadAttribute(range_name,phys_range);
  phys_range_min=phys_range[0];phys_range_max=phys_range[1];
//! \todo . [medium] read others attributes
/*
  range_name="physical_range_unit";
  cimglist_for(data,c) (fod.pNCvars[c])->add_att(range_name.c_str(),(const char*)phys_range_unit.c_str());

  range_name="acquisition_range";
  Tacqu acqu_range[2];
  acqu_range[0]=acqu_range_min;acqu_range[1]=acqu_range_max;
  cimglist_for(data,c) (fod.pNCvars[c])->add_att(range_name.c_str(),2,acqu_range);
  range_name="acquisition_range_unit";
  cimglist_for(data,c) (fod.pNCvars[c])->add_att(range_name.c_str(),(const char*)acqu_range_unit.c_str());
*/
  return 0;
}
//! load previously recorded data
/**
 * load previously recorded data as many channels in a list (i.e. CImgList container)
 * \note all channels should have the same size (i.e. 1D data with same width)
 * \param [in]  file_name file name (e.g. "data.nc")
 * \param [in]  var_names variable names (i.e. the requested list of channels;e.g. "pressure, hot_wire, square_wave, control_signal")
 * \param [out] unit_names variable unit names (i.e. unit names of channels; e.g. "Pa","m/s","V","volt")
 * \param [out] dim_names dimension name (i.e. only one dimension name in the list as channels are 1D data; e.g. "time")
 * \param [out] channels data as physical values (i.e. all requested channels in the data list)
 * \param [out] ...
**/
//! \note code progress: \c data from \c CImg to \c CImgList . DONE
//! \note code progress: function as template. DONE
//! \todo [low] remove \c std::cout<<"CImgListNetCDF::loadNetCDF... progress terminal output, and both return and output on error only !
//! \todo [very low] separate load header and data (looking forward to NetCDF unlimited dimension)
template <typename Tdata, /*typename Ttime,*/ typename Tacqu>
int load_data(std::string file_name,std::vector<std::string> var_names,std::vector<std::string> &unit_names,std::vector<std::string> &dim_names,CImgList<Tdata> &channels,
  int &sampling_rate,
  Tacqu &acqu_range_min, Tacqu &acqu_range_max, std::string &acqu_range_unit,
  Tdata &phys_range_min, Tdata &phys_range_max, std::string &phys_range_unit)
{
std::cerr<<__FILE__<<"/"<<__func__<<"[data+attribute](\""<<file_name<<"\",...)"<<std::endl;
  ///load NetCDF data values
  load_data(file_name,var_names,unit_names,dim_names,channels);
  ///load NetCDF acquisition information
  load_data_attribute(file_name,var_names[0],sampling_rate,acqu_range_min,acqu_range_max,acqu_range_unit,phys_range_min,phys_range_max,phys_range_unit);
  return 0;
}

//! save recorded data and additional informations
/**
 * save all recoreded data regarding to a single time axis (in order to display it with time axis under ncview for example)
 * \note each recorded channel is also written with its corresponding time axis (especially with the channel name prefix)
 *
 * @param fo 
 * @param data 
 * @param time 
 * @param DAQdev
 * @param data_unit_name 
 *
 * @return 
 */
//! \todo [medium] make NetCDF file object version (\c  save_data(NcFile*fp,... ) and its overload with file name
template <typename Tdata, typename Ttime, typename Tacqu>
int save_data(
  std::string file_name,
  cimg_library::CImgList<Tdata>& data,
  cimg_library::CImgList<Ttime>& time,
  DAQdevice DAQdev,
  std::string data_unit_name,
  Tacqu acqu_range_min, Tacqu acqu_range_max, std::string &acqu_range_unit,
  Tdata phys_range_min, Tdata phys_range_max, std::string &phys_range_unit
)
{
//std::cerr<<__FILE__<<"/"<<__func__<<"(\""<<file_name<<"\",...)\n"<<std::flush;
//data.print("data");
//time.print("time");
  ///open file (shared between \c fod and \c fot )
  //NetCDF file objects for both data and time
  CImgListNetCDF<Tdata> fod;//e.g. "pressure", "hot_wire"
  CImgListNetCDF<Ttime> fot;//e.g. "time","pressure__time","hot_wire__time"
  //open NetCDF file
  fod.saveNetCDFFile((char*)file_name.c_str());
  //set NetCDF file object for time
  fot.setNetCDFFile(fod.getNetCDFFile());
  ///create dimension (shared between \c fod and \c fot )
  //std::cerr<<"create dimension\n"<<std::flush;
  //dimension name (only one here, as 1D data)
  std::vector<std::string> dim_names;
  //!\note set both dimension and axis names of time to "time"
  dim_names.push_back("time");
  //create dimension
  fod.addNetCDFDims(data,dim_names);
  //set dimension for time
  fot.setNetCDFDims(fod.vpNCDim,fod.pNCDimt);
  ///create variables (and attributes)
  //std::cerr<<"create variables\n"<<std::flush;
  NcFile *fp=fod.getNetCDFFile();
  ///- data variables
  ////data attribute units
  std::vector<std::string> data_unit_names;
  cimglist_for(data,c) data_unit_names.push_back(data_unit_name);
  ////create data variables
  fod.addNetCDFVar(data,DAQdev.channel_name,data_unit_names);
  ////data attribute channel indexes
  std::string index_name("channel_index");
  cimglist_for(data,c) (fod.pNCvars[c])->add_att(index_name.c_str(),DAQdev.channel_index[c]);
  ////data attribute ranges
  std::string range_name("physical_range");
  Tdata phys_range[2];
  phys_range[0]=phys_range_min;phys_range[1]=phys_range_max;
  cimglist_for(data,c) (fod.pNCvars[c])->add_att(range_name.c_str(),2,phys_range);
  range_name="physical_range_unit";
  cimglist_for(data,c) (fod.pNCvars[c])->add_att(range_name.c_str(),(const char*)phys_range_unit.c_str());
  range_name="acquisition_range";
  Tacqu acqu_range[2];
  acqu_range[0]=acqu_range_min;acqu_range[1]=acqu_range_max;
  cimglist_for(data,c) (fod.pNCvars[c])->add_att(range_name.c_str(),2,acqu_range);
  range_name="acquisition_range_unit";
  cimglist_for(data,c) (fod.pNCvars[c])->add_att(range_name.c_str(),(const char*)acqu_range_unit.c_str());
  ///- time variables (first for all data, then each data have it own time)
  ////time data
  cimg_library::CImgList<Ttime> store_time(time.size()+1);
  store_time[0]=time[0];
  cimglist_for(time,c) store_time[c+1]=time[c];
  ////time names
  std::vector<std::string> time_var_names;
  time_var_names.push_back(dim_names[0]);
  cimglist_for(data,c) time_var_names.push_back(DAQdev.channel_name[c]+"__"+dim_names[0]);
  ////time units
//! \todo add unit__long_name="second" refering to unit="s"
  std::vector<std::string> time_unit_names(store_time.size(),"second");
  ////create data variables
  fot.addNetCDFVar(store_time,time_var_names,time_unit_names);
  ///create global attributes
//std::cerr<<"create globals\n"<<std::flush;
  ///- sampling rate
  fp->add_att("sampling_rate", DAQdev.sampling_rate);
  ///- sampling rate
//! \todo . save range: acquisition [min, max], physical [min, max] (as both global and local attribute)
  fp->add_att("range_id", DAQdev.range_id);
  ///write data
//std::cerr<<"create data\n"<<std::flush;
  ///- data
  fod.addNetCDFData(data,0);
  ///- time
  fot.addNetCDFData(store_time,0);
  return 0;
}


#endif// DAQ_DATA

