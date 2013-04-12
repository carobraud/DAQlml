#ifndef DAQ_COMEDI
#define DAQ_COMEDI
//!acquisition device
/**
 * acquisition device to record multiple 1D channels through comedi library
 * \note member names were initially based on \c option structure from comedi demo examples
 **/
class DAQdevice
{
 public://! \todo [YN] use setter and getter !
  int verbose; ///< verbose option
  std::string filename;///< device file name

  comedi_t *dev; ///< pointer to device
  comedi_range *comedirange; ///< pointer to comedirange;
  comedi_cmd c,*cmd; ///< comedi command

  int subdevice; ///< subdevice id
  int aref; ///< reference, GROUND or DIFFERENCE
  int bufsize; ///< board buffer size
  int maxdata; ///< max value of voltage
  unsigned int chanlist[256]; ///< channellist corresponding to the channel index

  int sample_number; ///< sample number
  int sampling_rate; ///< sampling rate
  int range_id; ///< range id;
  std::vector<std::string> channel_name; ///< channel name
  std::vector<int> channel_index; ///< channel index
 
  //! constructor
  //! \todo subdevice and aref should be configurable somewhere?
  DAQdevice()
    {
      initDAQdevice();
    }
  DAQdevice(std::string dev_filename,bool buffer=true)
    {
      filename=dev_filename;
      initDAQdevice();
    }
// !/todo add variable subdevice: 0 for read and 1 for write
  void initDAQdevice()
  {
    subdevice=0;
    aref=AREF_GROUND;
    cmd = &c;    
  }

  //! set unsigned int chanlist from channel_index, range and aref
  void setchannellist()
  {
    for(unsigned int i = 0; i < channel_index.size(); i++){
      chanlist[i] = CR_PACK(channel_index[i], range_id, aref);
    }
  }

  //! print on \c given stream all member values
  void print(std::ostream &stream)
  {
    stream<<"comedi version code: "<<comedi_get_version_code(dev)<<std::endl;
    stream<<"device file: "<<filename<<std::endl; 
    stream<<"driver name: "<<comedi_get_driver_name(dev)<<std::endl;
    stream<<"board name: "<<comedi_get_board_name(dev)<<std::endl;
    stream<<"subdevice id: " << subdevice << "\n";
    int type = comedi_get_subdevice_type(dev, subdevice);
    stream<<"subdevice type: ";
    switch(type)
      {
      case 0: stream<<"unused"<<std::endl; break;
      case 1: stream<<"analog input"<<std::endl; break;
      case 2: stream<<"analog output"<<std::endl; break;
      }
    stream<<"number of channels: "<<channel_index.size()<<std::endl;
    stream<<"channels: ";
    for(unsigned int i=0; i<channel_index.size(); i++){
      stream<<chanlist[i]<<" ";
    }
    stream<<std::endl;
    stream<<"analog reference id: "<<aref<<std::endl;
    std::cerr<<"analog reference: ";
    switch(aref)
      {
      case AREF_GROUND: std::cerr<<"AREF_GROUND"<<std::endl;break;
      case AREF_COMMON: std::cerr<<"AREF_COMMON"<<std::endl;break;
      case AREF_DIFF: std::cerr<<"AREF_DIFF"<<std::endl;break;
      case AREF_OTHER: std::cerr<<"AREF_OTHER"<<std::endl;break;
      }
    show_range(stream);
    //stream<<"convert binary to voltage: "<<physical<<std::endl;
    stream<<"number of samples: "<<sample_number<<std::endl;
    stream<<"sampling rate: "<<sampling_rate<<" Hz"<<std::endl;

    // dump command
    if(verbose)
      {
	std::cout<<"start_arg: "<< cmd->start_arg <<std::endl;
	std::cout<<"scan_begin_arg: "<< cmd->scan_begin_arg <<std::endl;
	std::cout<<"convert_arg: "<< cmd->convert_arg <<std::endl;
	std::cout<<"scan_end_arg: "<< cmd->scan_end_arg <<std::endl;
	std::cout<<"stop_arg: "<< cmd->stop_arg <<std::endl;
      }
  }
  
  //! print on standard output
  void print()
  {
    print(std::cout);
  }

  //! print voltage range information
  int show_range(std::ostream &stream)
  {
    ///- show minimum and maximum of the range
    stream<<"range=["<<comedirange->min<<".."<<comedirange->max<<"] (";
    ///- show unit of the range
    std::string unit_name;
    switch(comedirange->unit)
      {
      case UNIT_volt:
	unit_name="volts";
	break;
      case UNIT_mA:
	unit_name="milliamps";
	break;
      case UNIT_none:
      default:
	unit_name="none";
	break;
      }//switch unit
    stream<<unit_name<<")\n";
    return 0;
  }

  //! load acquisition parameter from file
  /**
   * load acquisition parameters from NetCDF file (i.e. .CDL; ncgen -o parameters.nc parameters.cdl): such as sampling conditions.
   * The program user should specify all program parameters within this .CDL file.
   * \param [in] file_name NetCDF/CDL parameter file name (e.g. file named "~/acquisition/parameters.nc")
   * \see show_range
   **/
  int load_parameter(const std::string file_name)
  {

    //NetCDF/CDL parameter file object (i.e. parameter class)
    CParameterNetCDF fp;
    ///open file from its name
    int error=fp.loadFile((char *)file_name.c_str());
    if(error){std::cerr<<"loadFile return "<< error <<std::endl;return error;}
    ///open acquisition variable (i.e. process variable)
    //prepare to load parameters as the attribute of "acquisition"
    //! \bug \c process should be \c int or \c byte instead of \c float
    float process;
    std::string process_name="acquisition";
    if((error=fp.loadVar(process,&process_name))){std::cerr<<"Error: process variable \""<<process_name<<"\" can not be loaded (return value is "<<error<<")\n";return error;}
    ///load attributes (i.e. process parameters)
    // load parameters as process attributes
    //! \todo [low] add error messages for all attributes
    fp.loadAttribute("channels",channel_index);
    fp.loadAttribute("channel_name",channel_name);
    fp.loadAttribute("sampling_rate",sampling_rate);
    fp.loadAttribute("number_of_samples",sample_number);
    fp.loadAttribute("range_id",range_id);

    setchannellist();

    return 0;
  }


  //! prepare commands to the board
  /**
   * prepare command for the board
   **/
  int prepare_cmd()
  { 
    unsigned period_nanosec=(unsigned)(1e9/sampling_rate);

    memset(cmd,0,sizeof(*cmd));

    cmd->subdev = subdevice;
    cmd->flags = 0;

    //! start the measurement immediately
    //! one can specify TRIG_EXT for external trigerring
    //! \todo cmd->start_src should be configurable by parameter.nc
    cmd->start_src = TRIG_NOW;
    cmd->start_arg = 0;

    // set the board sampling rate (see comedi website)
    cmd->scan_begin_src = TRIG_TIMER;
    cmd->scan_begin_arg = period_nanosec;

    // set waiting time for A/D convertion to be minimum
    // this should be corrected in command test
    cmd->convert_src = TRIG_TIMER;
    cmd->convert_arg = 1;

    // set number of channels
    cmd->scan_end_src = TRIG_COUNT;
    cmd->scan_end_arg   = channel_index.size();

    // set number of scans
    cmd->stop_src = TRIG_COUNT;
    cmd->stop_arg = sample_number;

    // set channel list
    cmd->chanlist	    = chanlist;
    cmd->chanlist_len = channel_index.size();

    return 0;
  }

  //! configure device (A/D board) and prepare to start the operation
  /** 
   * 
   * @param map pointer to mapped memory region
   * 
   * @return 
  **/ 
  int config_device_common()
  {
    int sdflag;			/// subdevice flag
    
    // load thee device file
    dev = comedi_open(filename.c_str());
    if(!(dev)){
      comedi_perror(filename.c_str());
      exit(1);
    }
    // check subdevice flag before using it
    sdflag = comedi_get_subdevice_flags(dev, subdevice);
    std::cout<<std::showbase;
    if (verbose) std::cout<<"subdevice flag: "<<std::hex<<sdflag<<std::dec<<std::endl;

// get max data and range. these will be used to convert 16bit integer binary to physical voltage
    maxdata = comedi_get_maxdata(dev, subdevice,channel_index.front());
    comedirange = comedi_get_range(dev, subdevice,channel_index.front(),range_id);
    if(verbose) std::cout<<"maxdata:"<<maxdata<<", min:"<<(comedirange)->min<<", max:"<<(comedirange)->max<<", unit:"<<(comedirange)->unit<<std::endl;

    return 0;
  }//config_device_common


  int config_device_point()
  {
    return config_device_common();
  }//config_device_point

  int config_device_specific_buffer(void *&map)
  {

    int ret;

    // get the buffer size of the subdevice 
    bufsize = comedi_get_buffer_size(dev, subdevice);
    if(verbose) std::cout<<"buffer size is "<<bufsize<<std::endl; 

    // map device buffer to main memory through device file /dev/comedi0
    // option MAP_SHARED means updating memory contents when the file (/dev/comedi0) is updated.
    // (this means buffer and memory are linked??)
    // mmap(*addr, size, prot, flags, fd, offset) is the system function.
    map = mmap(NULL, bufsize, PROT_READ, MAP_SHARED, comedi_fileno(dev), 0);
    if( verbose ) std::cout<<"pointer to mapped region: "<<std::hex<<map<<std::dec<<std::endl;
    if( map == MAP_FAILED ){
      perror( "mmap" );
      exit(1);
    }
    // this function should be called twice.
    ret = comedi_command_test(dev, cmd);
    ret = comedi_command_test(dev, cmd);
    if(ret != 0){
      fprintf(stderr,"command_test failed\n");
      exit(1);
    }

    // check actual sampling frequency is the same as the user specified
    int actual_freq=1e9/cmd->scan_begin_arg;
    if(sampling_rate != actual_freq)
      {    
	std::cerr<<"please check the sampling rate."<<std::endl;
	std::cerr<<"user specified sampling freq: "<<sampling_rate<<" Hz"<<std::endl;
	std::cerr<<"        actual sampling freq: "<<actual_freq<<" Hz"<<std::endl;
	exit(1);
      }

    // send the command to the board
    ret = comedi_command(dev, cmd);
    if(ret < 0)
    {
      comedi_perror("comedi_command");
      exit(1);
    }

    return 0;
  }//config_device_specific_buffer

  int config_device_buffer(void *&map)
  {
    int ret;
    prepare_cmd(); // prepare command to the board;
    if((ret=config_device_common())!=0) return ret;
    return config_device_specific_buffer(map);
   }//config_device_buffer

};//DAQdevice class




//! display help options
/** 
 * 
 * 
 * \code
 *   print_help(std::cout);
 * \endcode
 * @return 
 */
//! \todo [low] should be in DAQdevice class
int print_help(std::ostream &stream)
{
  stream<<"";
  return 0;
}

//! convert integer 16bit binary data into float physical voltage
/** 
 * 
 * 
 * @param [in]    data integer binary
 * @param [inout] data_phys physical voltage
 * @param [in]    DAQdev DAQdevice object
 * 
 * @return 
 */
template <typename Tacquisition, typename Tphysical>
  int convert_to_phys(cimg_library::CImgList<Tacquisition>& data, cimg_library::CImgList<Tphysical>& data_phys, DAQdevice DAQdev)
{
  data_phys.assign(data);
  cimglist_for(data,c)
    {
      cimg_forX(data[c],s)
	{
	  data_phys[c](s)=comedi_to_phys(data[c](s), DAQdev.comedirange, DAQdev.maxdata);
	}
    }
  return 0;
}
//! convert float physical voltage into integer 16bit binary data
/** 
 * 
 * 
 * @param data integer binary
 * @param data_phys physical voltage
 * @param [inout] DAQdev DAQdevice object
 * 
 * @return 
 */
template <typename Tacquisition, typename Tphysical>
  int convert_from_phys(cimg_library::CImgList<Tphysical>& data_phys, cimg_library::CImgList<Tacquisition>& data, DAQdevice DAQdev, const bool assign=true)
{
  if(assign) data.assign(data_phys);
  cimglist_for(data_phys,c)
    {
      cimg_forX(data_phys[c],s)
	{
	  data[c](s)=comedi_from_phys(data_phys[c](s), DAQdev.comedirange, DAQdev.maxdata);
	}
    }
  return 0;
}

//! create time corresponding to each sample
/** 
 * create time corresponding to each sample
 * the delay between channels is also taken into account
 * 
 * @param[in] data 
 * @param[out] time 
 * @param[in,out] DAQdev 
 * @param sampling_rate 
 * 
 * @return 
 */
template <typename Tdata, typename Ttime>
  int create_time(cimg_library::CImgList<Tdata>& data, cimg_library::CImgList<Ttime>& time, DAQdevice DAQdev)
{
  //get minimum convert time of the board
  int min_convert_time=DAQdev.cmd->convert_arg; // in nano second

  //std::cerr<<__FILE__<<"/"<<__func__<<"(...)\n"<<std::flush;
  //data.print("data");
  //PR(board_frequency);
  //PR(sampling_rate);
  //allocate time structure
  time.assign(data);
  //time.print("time");
  double cdelay=1e-9*(double)min_convert_time;//delay between channels
  double present_time=0.0;
  cimg_forX(time[0],s)
    {
      cimglist_for(time,c)
	{
	  time[c](s)=present_time+cdelay*c; // time of a channel is delayed depending on c
	}
      present_time+=1/(double)DAQdev.sampling_rate;
    }
  return 0;
}

//! subfunction of 
char *cmd_src(int src,char *buf)
{
  buf[0]=0;
  if(src&TRIG_NONE)strcat(buf,"none|");
  if(src&TRIG_NOW)strcat(buf,"now|");
  if(src&TRIG_FOLLOW)strcat(buf, "follow|");
  if(src&TRIG_TIME)strcat(buf, "time|");
  if(src&TRIG_TIMER)strcat(buf, "timer|");
  if(src&TRIG_COUNT)strcat(buf, "count|");
  if(src&TRIG_EXT)strcat(buf, "ext|");
  if(src&TRIG_INT)strcat(buf, "int|");
#ifdef TRIG_OTHER
  if(src&TRIG_OTHER)strcat(buf, "other|");
#endif
  if(strlen(buf)==0){
    sprintf(buf,"unknown(0x%08x)",src);
  }else{
    buf[strlen(buf)-1]=0;
  }
  return buf;
}


char *tobinary(char *s,int bits,int n)
{
  int bit=1<<n;
  char *t=s;

  for(;bit;bit>>=1)
    *t++=(bits&bit)?'1':'0';
  *t=0;

  return s;
}

void probe_max_1chan(comedi_t *it,int s)
{
  comedi_cmd cmd;
  char buf[100];

  printf("  command fast 1chan:\n");
  if(comedi_get_cmd_generic_timed(it, s, &cmd, 1, 1)<0){
    printf("    not supported\n");
  }else{
    printf("    start: %s %d\n",
	   cmd_src(cmd.start_src,buf),cmd.start_arg);
    printf("    scan_begin: %s %d\n",
	   cmd_src(cmd.scan_begin_src,buf),cmd.scan_begin_arg);
    printf("    convert: %s %d\n",
	   cmd_src(cmd.convert_src,buf),cmd.convert_arg);
    printf("    scan_end: %s %d\n",
	   cmd_src(cmd.scan_end_src,buf),cmd.scan_end_arg);
    printf("    stop: %s %d\n",
	   cmd_src(cmd.stop_src,buf),cmd.stop_arg);
  }
}

void get_command_stuff(comedi_t *it,int s)
{
  comedi_cmd cmd;
  char buf[100];

  if(comedi_get_cmd_src_mask(it,s,&cmd)<0){
    printf("    not supported\n");
  }else{
    printf("    start: %s\n",cmd_src(cmd.start_src,buf));
    printf("    scan_begin: %s\n",cmd_src(cmd.scan_begin_src,buf));
    printf("    convert: %s\n",cmd_src(cmd.convert_src,buf));
    printf("    scan_end: %s\n",cmd_src(cmd.scan_end_src,buf));
    printf("    stop: %s\n",cmd_src(cmd.stop_src,buf));

    probe_max_1chan(it,s);
  }
}

int get_board_info(std::string fd, bool verbose)
{

  std::vector<std::string> subdevice_types;
  subdevice_types.push_back("unused");
  subdevice_types.push_back("analog input");
  subdevice_types.push_back("analog output");
  subdevice_types.push_back("digital input");
  subdevice_types.push_back("digital output");
  subdevice_types.push_back("digital I/O");
  subdevice_types.push_back("counter");
  subdevice_types.push_back("timer");
  subdevice_types.push_back("memory");
  subdevice_types.push_back("calibration");
  subdevice_types.push_back("processor");
  subdevice_types.push_back("serial digital I/O");

  comedi_t *it;
  int i,j;
  int n_subdevices, type;
  int chan, n_chans;
  int n_ranges;
  int subdev_flags;
  comedi_range *rng;
  comedi_cmd cmd;

  it=comedi_open((char*)fd.c_str());
  std::cout<<"version code: "<<comedi_get_version_code(it)<<std::endl;
  std::cout<<"device file: "<<fd<<std::endl; 
  std::cout<<"driver name: "<<comedi_get_driver_name(it)<<std::endl;
  std::cout<<"board name: "<<comedi_get_board_name(it)<<std::endl;

  comedi_get_cmd_generic_timed(it, 0, &cmd, 1, 1); // assuming analog input is channel 0
  int min_convert_time=cmd.convert_arg;

  if(verbose)
    {
      n_subdevices=comedi_get_n_subdevices(it);
      std::cout<<"number of subdevices: "<<n_subdevices<<std::endl;
      for(i = 0; i < n_subdevices; i++){
	printf("subdevice %d:\n",i);
	type = comedi_get_subdevice_type(it, i);
	std::cout<<" type:"<<type<<subdevice_types[type]<<std::endl;
	if(type==COMEDI_SUBD_UNUSED)
	  return 0;
	subdev_flags = comedi_get_subdevice_flags(it, i);
	printf("  flags: 0x%08x\n",subdev_flags);
	n_chans=comedi_get_n_channels(it,i);
	printf("  number of channels: %d\n",n_chans);
	if(!comedi_maxdata_is_chan_specific(it,i)){
	  printf("  max data value: %lu\n", (unsigned long)comedi_get_maxdata(it,i,0));
	}else{
	  printf("  max data value: (channel specific)\n");
	  for(chan=0;chan<n_chans;chan++){
	    printf("    chan%d: %lu\n",chan,
		   (unsigned long)comedi_get_maxdata(it,i,chan));
	  }
	}
	printf("  ranges:\n");
	if(!comedi_range_is_chan_specific(it,i)){
	  n_ranges=comedi_get_n_ranges(it,i,0);
	  printf("    all chans:");
	  for(j=0;j<n_ranges;j++){
	    rng=comedi_get_range(it,i,0,j);
	    printf(" [%g,%g]",rng->min,rng->max);
	  }
	  printf("\n");
	}else{
	  for(chan=0;chan<n_chans;chan++){
	    n_ranges=comedi_get_n_ranges(it,i,chan);
	    printf("    chan%d:",chan);
	    for(j=0;j<n_ranges;j++){
	      rng=comedi_get_range(it,i,chan,j);
	      printf(" [%g,%g]",rng->min,rng->max);
	    }
	    printf("\n");
	  }
	}
	printf("  command:\n");
	get_command_stuff(it,i);

      }
    }//verbose
  return min_convert_time;
}

#endif// DAQ_COMEDI

