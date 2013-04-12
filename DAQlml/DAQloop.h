#ifndef DAQ_LOOP
#define DAQ_LOOP

#ifndef LOOP_USLEEP_TIME
//! waiting time for the buffer
#define LOOP_USLEEP_TIME 100
#endif

//! read the data from the mapped buffer
/** 
 * 
 * @param data  
 * @param map 
 * @param DAQdev
 * @param control 
 * 
 * @return 
 */
//! \todo remove double pointer (i.e. **) to at least single pointer (i.e. * or may be &)
template<typename T>
inline int sample_data_buffer(cimg_library::CImgList<T>& data, void *map, DAQdevice& DAQdev, bool control=false)
{
  std::cerr<<__func__<<"\n"<<std::flush;
  int ret; // return of function
  int front = 0; // address of mapped buffer
  int back = 0;  //
  int sample_count = 0; // number of samples stored in one channel
  int sampling_complete_flag=0;

  int size=DAQdev.bufsize;
  int sample_number=DAQdev.sample_number;
  int channel_number=DAQdev.channel_index.size();

  std::cout<<"LOOP_USLEEP_TIME = "<<LOOP_USLEEP_TIME<<std::endl;

  // sampling loop begins
  while(1)
    {
      // get the number of bytes available in the streaming buffer.
      front += comedi_get_buffer_contents(DAQdev.dev, DAQdev.subdevice);
      
      // when the buffer is not updated, wait time specified by LOOP_USLEEP_TIME
      if(front == back)
	{
	  usleep(LOOP_USLEEP_TIME);
	  continue;
	}
      // read available buffer (from back to front)
      int col = 0;
      for(int i = back; i < front; i += sizeof(sampl_t))
	{
	  if (DAQdev.verbose)std::cout<<"front="<<front<<",back="<<back<<",index="<<i<<",value"<<*(sampl_t *)(map + (i % size))<<std::endl;
	  data[col](sample_count)=*(sampl_t *)(map + (i % size)); // read the contents of the memory directly and convert it into sampl_t type, which is actually integer.
	  col++;
	  if(col == channel_number)
	    { 
	      col = 0; //re-initialize col when one series of scan is finished  
	      sample_count++; 	
	      if(sample_count==sample_number) sampling_complete_flag=1; //sampling should be stopped when the sample_count reaches to the total number of sample
	    }
	  if(sampling_complete_flag==1) break;
	}

      // mark the position where the program has already read. refer to the comedi lib manual for further info
      ret = comedi_mark_buffer_read(DAQdev.dev, DAQdev.subdevice, front - back);
      if(ret < 0){comedi_perror("comedi_mark_buffer_read"); break;}
	
      // "front" of this time becomes "back" for next time.
      back = front;
    
      // exit while loop
      if(sampling_complete_flag) break;
    
    }//sampling loop
 


 
  return 0;
}

template<typename T>
inline int sample_data_point(cimg_library::CImgList<T>& data, DAQdevice& DAQdev, bool control=false)
{
 std::cerr<<__func__<<"\n"<<std::flush;
 int sample_number=DAQdev.sample_number;
 int channel_number=DAQdev.channel_index.size();
 
 RT_preempt RT;
// Initialization administration RT  
 RT.initialization(DAQdev.sampling_rate);
lsampl_t value = 99;
for(int i=0;i< sample_number;++i)
{
  // sleep of a nanosecond
  RT.nanowait();
//acquire 1 point for all channels
for(int channel=0;channel<channel_number; ++channel)
{
//! \todo [medium] set \c range and \c aref from \c DAQdev
comedi_data_read_delayed(DAQdev.dev, DAQdev.subdevice, DAQdev.channel_index[channel]/*channel*/,0/*range*/,0/*aref*/,&value,1);
data[channel](i)=value;

}//channel loop

// computation of the next time interval in a deterministic way: 50 microsec)
  RT.next_time_interval();
  

 }//i loop

 return 0;
}

#endif// DAQ_LOOP

