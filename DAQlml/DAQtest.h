#ifndef DAQ_TEST
#define DAQ_TEST

#define TEST_SQWAVE  1
//! \todo [low] add sinus test
#define TEST_SINWAVE 2

//!test device
/**
 * test of characteristics of a square wave : period, DC...
 **/

class DAQtest
{
 public:

  float reference_frequency;
  float reference_DC;
  float reference_tolerance;
  bool DCfrequency;
  bool gaussian_filter;

//! Print instructions depending of the type of signal we want to test (sinus,square...)
/** 
 * detect edges from sqwave single channel data
 * this function returns raising edge separation, falling edge separation and duration
 * 
 * @param[in] data 
 */
int test_print_instruction(int sw)
{
  if (sw==TEST_SQWAVE){
    std::cout<<"You are testing DAQ with square wave."<<std::endl;
    std::cout<<"This test assumes that the 1 kHz square wave comes into channel 0 with a 100 us duration."<<std::endl;
//    std::cout<<"Press enter when it is ready."<<std::endl;
  }
//  std::cin.get();
  return 0;
}



 int load_parameter(const std::string file_name)
 {

   //NetCDF/CDL parameter file object (i.e. parameter class)
   CParameterNetCDF fp;

   ///open file from its name
   int error=fp.loadFile((char *)file_name.c_str());
   if(error){std::cerr<<"loadFile return "<< error <<std::endl;return error;}

   ///open test variable (i.e. process variable)
   //prepare to load parameters as the attribute of "test"
   //! \bug \c process should be \c int or \c byte instead of \c float
   float process;
   std::string process_name="test";
   if((error=fp.loadVar(process,&process_name))){std::cerr<<"Error: process variable \""<<process_name<<"\" can not be loaded (return value is "<<error<<")\n";return error;}

   ///load attributes (i.e. process parameters)
   // load parameters as process attributes
   //! \todo [low] add error messages for all attributes
   fp.loadAttribute("frequency",reference_frequency);
   fp.loadAttribute("duty_cycle",reference_DC);
   fp.loadAttribute("tolerance",reference_tolerance);
   int param;
   fp.loadAttribute("DCfreq",param);
   DCfrequency =(param==1);
   fp.loadAttribute("filter",param);
   gaussian_filter =(param==1);

   return 0;
  }





//! detect edges from sqwave single channel data
/** 
 * detect edges from sqwave single channel data
 * this function returns raising edge separation, falling edge separation and duration
 * 
 * @param[in] data 
 * @param[out] raisingedges
 * @param[out] fallingedges 
 * @param[in] threshold 
 * @param verbose
  * @param [in] borders 
 * 
 * @return 
 */


  template <typename T> int detect_edges(cimg_library::CImg<float> data, 
					 cimg_library::CImg<int> &raisingedges,
					 cimg_library::CImg<int> &fallingedges,
					 T threshold, int verbose,bool borders)
{
  cimg_library::CImg<int>     clean          = data.get_threshold(threshold);
  cimg_library::CImgList<int> gcleanlist;
  //cimg_library::CImg<int> clean;
  cimg_library::CImg<int> clean_gradient;
    
  int raising_count = 0;
  int falling_count = 0;

  // GRADIENT
  gcleanlist= clean.get_gradient("x",1);

  if(verbose >= 3)
    {
      data.display_graph("raw data"); 
      clean.display_graph("clean data");   
    }
  //gcleanlist[0].display_graph("clean_gradient"); 
  //raisingedges.assign(clean_gradient.width());
  //fallingedges.assign(clean_gradient.width());
  
  //clean gradient
  clean_gradient = gcleanlist(0);
  
  //detect raising and falling edges
  raisingedges.assign(clean_gradient.width());
  fallingedges.assign(clean_gradient.width());

  cimg_forX(clean_gradient,s){

  //PR(clean.size()); 

    if(clean_gradient(s)==1){
      raisingedges(raising_count)=s;
      raising_count++;
    }

    if(clean_gradient(s)==-1){
      fallingedges(falling_count)=s;
      falling_count++;
    }
  }//sample loop
 //PR(clean.size()); 

  raisingedges.crop(0,raising_count-1);
  fallingedges.crop(0,falling_count-1);

  //Clean borders
  if(borders)
  { //suppression if falling before raising
    if(fallingedges(0)<raisingedges[0]) fallingedges.crop(1,(fallingedges.size()-1));
    //crop if raising different falling
    if(raisingedges.size()!=fallingedges.size()) raisingedges.crop(0,(fallingedges.size()-2));
  }//clean border

return 0;
}





template <typename T> int duration_samples ( cimg_library::CImg<T> raisingedges,cimg_library::CImg<T> fallingedges,cimg_library::CImg<T> &duration_up,
CImg<T> &duration_down)
{
  // calculation of duration up 
  duration_up.assign(raisingedges.width());
  duration_up=fallingedges-raisingedges;
  //calculation of duration down
  duration_down.assign(raisingedges.width()-1);
  cimg_forX(duration_down,c) duration_down(c)=raisingedges(c+1)-fallingedges(c);
//PR(duration_down(0));

return 0;
}


/**
* This fonction return the data conversion between samples to unity of time  
* \param [in] data in samples
* \param [out] data in unity of time
**/

template <typename Tin,typename Tout> int duration_sampletotime (cimg_library::CImg<Tin> duration_up,int frequence_acqui,cimg_library::CImg<Tout> &duration_up_time)
{
  duration_up_time.assign(duration_up.width());
  Tout scaling=1/(Tout)frequence_acqui;
  duration_up_time=duration_up*scaling;
  return 0;
}




/**
* This fonction return the period data  
* \param [in] uporDC = duration up in unity of time
* \param [in] duration_down_time
* \param [out] TorF = period
**/

template <typename T> int period (cimg_library::CImg<T> uporDC,cimg_library::CImg<T> duration_down_time, cimg_library::CImg<T> &TorF)
{
TorF.assign(duration_down_time.width());
TorF= uporDC + duration_down_time;

return 0;
}



//!calculation of duty cycle
/**
* This fonction return the duty cycle
*
* \param [in] uporDC=duration up in time
* \param [in] period
* \param [out] uporDC= Duty cycle
**/

template <typename T> int compute_duty_cycle(cimg_library::CImg<T> &uporDC,cimg_library::CImg<T> &period)
{ 
  uporDC.assign(period.width());
  cimg_forX(uporDC,x) uporDC(x)/=period(x);
  return 0;
}



//!calculation of frequence
/**
* This fonction return the frequence
*
* \param [in] TorF= period
* \param [out] TorF=frequence cycle
**/
template <typename T> int compute_frequence_cycle(cimg_library::CImg<T> DC,cimg_library::CImg<T> &TorF)
{
  TorF.assign(DC.width());
  cimg_forX(TorF,x) TorF(x)= 1/TorF(x);
  return 0;
}



//! data filter
/**
* this fonction delete the outlier data
*
* \param [in] data all values...
* \param [out] data filter    
**/
template <typename T> int compute_filter_data(cimg_library::CImg<T> data,cimg_library::CImg<T> &data_filter)

{ 
// calcul du min,max,mean et rms 
  
  float data_moy;
  float data_min;
  float data_max; 
  float data_rms;

  data_min= data.min();
  data_max= data.max();
  data_moy=data.mean();
  data_rms= std::sqrt(data.variance());

// filtre des valeurs aberrantes
  
  float low_data;
  int data_filter_count;
  float high_data;
  
  data_filter=data;
  data_filter.assign(data.width());
  data_filter_count=0;
  low_data=data_moy - (3 * data_rms);
  high_data=data_moy + (3 * data_rms);
  cimg_forX(data,x)
  {

    if ((data(x)> low_data) && (data(x)< high_data))

 
    { 
 
     data_filter(data_filter_count)=data(x);
     data_filter_count++;
  
    }
  }//for x
//PR(data_filter_count);
  data_filter.resize(data_filter_count);

  return 0;
}



//! detect error
/**
* this fonction detect the error data after the filter
*
* \param [in] data all values...
* \param [out] data error    
**/
template <typename T> int compute_filter_error(cimg_library::CImg<T> &duration_up,cimg_library::CImg<T> &raisingedges,cimg_library::CImg<T> &data_error_filter)
{ 
  data_error_filter.assign(duration_up.width());
  float data_mean_up=duration_up.mean();
  float data_rms_up=std::sqrt(duration_up.variance());
  int j=0;
  cimg_forX(duration_up,i)
  {

if( (duration_up(i)< (data_mean_up-3 * data_rms_up)) || (duration_up(i) > (data_mean_up+ 3 * data_rms_up)) )

{
data_error_filter(j)=raisingedges(i);
j++;
}
  }//duration_up loop
data_error_filter.crop(0,j-1);
  
data_error_filter.print("error filter");

return 0;
}



  //! data histogram
/**
 * This fonction class the data in a histogram
 * \param [in] data all values ...
 * \param [in] frequence_acqui
 * \param [out] data_histogram
**/
template <typename T> int compute_data_histogram ( cimg_library::CImg<T> data,T frequence_acqui,cimg_library::CImg<T> &data_histogram)  
{  
   int nb_classes;
   T data_min;
   T data_max;
  

//data.print("data");
  data_min= data.min();
  data_max= data.max();
  nb_classes=(int)std::ceil((data_max-data_min)*frequence_acqui+1);
  
	
  PR(nb_classes);
  //  data_histogram.assign(data.width());
  PR(data_max);
  PR(frequence_acqui);
  T tmp=data_max+1/frequence_acqui;
  PR(tmp);
  data_histogram=data.get_histogram(nb_classes,data_min,tmp);
//data_histogram.print("histogram");
  return 0;
}




//! test for TTL (i.e. square signal)
/**
 * \param [in] reference_duration_up: duration of the TTL (i.e. upper level lenght in second)
 * \param [in] reference_frequency: frequency of the TTL (i.e. inverse of total period in second)
**/

template <typename T> int test_signal (cimg_library::CImg<T> data, int frequence_acqui,bool filter, bool DCfreq,int display,T reference_frequency,T reference_DC, T reference_tolerance)
{
  cimg_library::CImg<T> TorF;
  cimg_library::CImg<int> duration_up;
  cimg_library::CImg<int> duration_down;
  cimg_library::CImg<T> uporDC;
  cimg_library::CImg<int> duration_down_time;
  cimg_library::CImg<int> period_samples;
  cimg_library::CImg<int> fallingedges;
  cimg_library::CImg<int> raisingedges;
  cimg_library::CImg<T> data_histogram;
  float data_moy;
  float data_rms;
  cimg_library::CImg<int> data_error_filter;
  cimg_library::CImg<int> data_error_filter_up;
  cimg_library::CImg<int> data_error_filter_down;
  //cimg_library::CImg<T> data_filter;
  T reference_duration_up;
  T reference_period;
  float maxX_histo;
  float maxY_histo;
  float maxX;
  cimg_library::CImg<float> stats_histo;

  reference_duration_up = (reference_DC)/(reference_frequency);
  reference_period = 1.0/reference_frequency;


  //in sample
  detect_edges(data,raisingedges,fallingedges,2.5f,display,true);
  duration_samples(raisingedges,fallingedges,duration_up,duration_down);
  duration_up.print("duration_up");
  duration_down.print("duration_down");
  period(duration_up,duration_down,period_samples);
  period_samples.print("period_samples");
  //in time
  duration_sampletotime(period_samples,frequence_acqui,TorF);
  TorF.print("TorF");
  duration_sampletotime(duration_down,frequence_acqui,duration_down_time);
  duration_down_time.print("duration_down_time");
  duration_sampletotime(duration_up,frequence_acqui,uporDC);
  uporDC.print("uporDC");

if (DCfreq)
{
  compute_duty_cycle(uporDC,TorF);
  compute_frequence_cycle(uporDC,TorF);
}

//filter and histogram if uporDC=duration up in time or duty cycle
if(filter) compute_filter_data(uporDC,uporDC);
uporDC.print("uporDC_filter");
//! \todo check scaling histogram for up or DC
if (!DCfreq) compute_data_histogram (uporDC,(T)frequence_acqui,data_histogram);//T
else compute_data_histogram (uporDC,reference_frequency/(T)frequence_acqui,data_histogram);//DC
data_histogram.print("uporDC_histogram");

if (display >= 1)
{
  T min=uporDC.min();
  T max=uporDC.max();


  if(DCfreq)
  {
    data_histogram.display_graph("histogram_DC",3,1,"Duty cycle",min,max+reference_frequency/(T)frequence_acqui,"Duty cycle count");
    //data_histogram.display_graph("histogram_DC",3,1,"Duty cycle",0,1,"Duty cycle count");

  }
  else
  {
    data_histogram.display_graph("duration up in time ",3,1,"duration up",min,max+1/(T)frequence_acqui,"up count");
  }

}// end if display
else 
{

  if(DCfreq) data_histogram.print("histogram_DC");
  else data_histogram.print("duration up");

return 0;
}// end if no display(print)


//filter and histogram if TorF=frequency or period
if (filter) compute_filter_data(TorF,TorF);
TorF.print("TorF_filter");
compute_filter_error(duration_up,raisingedges,data_error_filter_up);
compute_filter_error(duration_down,fallingedges,data_error_filter_down);
data_error_filter.assign(data_error_filter_up.width()+data_error_filter_down.width());
data_error_filter.draw_image(0,data_error_filter_up);
data_error_filter.draw_image(data_error_filter_up.width(),data_error_filter_down);
data_error_filter.sort();

 if(display >= 2)
   {
     data_error_filter_up.display_graph("error place up",1,1,"error index",0,0,"sample index");
     data_error_filter_down.display_graph("error place down",1,1,"error index",0,0,"sample index");
     data_error_filter.display_graph("error place up+down",1,1,"error index",0,0,"sample index");
   }

//! \todo . scale histogram for T or F
if(!DCfreq) compute_data_histogram(TorF,(T)frequence_acqui,data_histogram);//T
else        compute_data_histogram(TorF,data.width()/(T)frequence_acqui,data_histogram);//F

data_histogram.print("TorF_histogram");

if (display >= 1)
{
  if(DCfreq)data_histogram.display_graph("histogram_frequence",3,1,"frequence",TorF.min(),TorF.max()+frequence_acqui/(T)data.width(),"frequence count");
  else data_histogram.display_graph("period",3,1,"period",TorF.min(),TorF.max(),"period count");
}//end if display
else

{

  if (DCfreq)data_histogram.print("frequence");
  else data_histogram.print("period");

}//end if no display(print)
 //test with the reference data
 std::cout<<"############################ "<< std::endl;
 std::cout<<"#####   BEGIN OF TEST  ##### "<< std::endl;
 std::cout<<"############################ "<< std::endl;


maxX_histo = 0;
cimg_forX(data_histogram,c)
{
  if(data_histogram(c)>data_histogram(maxX_histo))
    {
      maxX_histo = c;
    }
}


float vmax, vmin;
cimg_library::CImg<unsigned char> message(300,200,1,3);
const unsigned char black[]={0,0,0};
const unsigned char white[]={255,255,255};
const unsigned char red[]={255,0,0};
const unsigned char green[]={0,255,0};
message.fill(0);

vmin = TorF.min() ;
std::cout<<"vmin = "<< vmin << std::endl;
std::cout<<"nb_levels = "<< data_histogram.width() << std::endl;

/*

 if(DCfreq)
   {
     vmax = TorF.max() + (float)frequence_acqui/data.width() ;
     std::cout<<"vmax = "<< vmax << std::endl;
     maxX = (((vmax-vmin)*maxX_histo)/data_histogram.width()) + vmin ;

     std::cout<<"Frequence de reference : "<< reference_frequency <<std::endl;
     std::cout<<"Maximum observé : "<< maxX <<std::endl;     
     if(maxX<(reference_frequency*(1+reference_tolerance/(100.0))) && maxX>(reference_frequency*(1-reference_tolerance/(100.0))))
       {
	 std::cout<<"TEST OK"<< std::endl; 
	 message.draw_rectangle(0,0,300,200,green,1);
	 message.draw_text(60,70,"Test OK !",black,0,1,50);
       }
     else
       {
	 message.draw_text(60,70,"Test KO !",white,0,1,50);
	 message.draw_rectangle(0,0,300,200,red,1);
	 std::cout<<"TEST KO ! "<< std::endl; 
       }   
  }
 else
   {
     vmax = TorF.max() + (float)(1./frequence_acqui) ;
     std::cout<<"vmax = "<< vmax << std::endl;
     maxX = (((vmax-vmin)*maxX_histo)/data_histogram.width()) + vmin ;

     std::cout<<"Periode de reference : "<< reference_period <<std::endl;
     std::cout<<"Maximum observé : "<< maxX <<std::endl; 
     if(maxX<(reference_period*(1+reference_tolerance/(100.0))) && maxX>(reference_period*(1-reference_tolerance/(100.0))))
       {
	 std::cout<<"TEST OK "<< std::endl; 
	 message.draw_rectangle(0,0,300,200,green,1);
	 message.draw_text(60,70,"Test OK !",black,0,1,50);
       }
     else
       {
	 std::cout<<"TEST KO ! "<< std::endl; 
	 message.draw_rectangle(0,0,300,200,red,1);
	 message.draw_text(60,70,"Test KO !",white,0,1,50);
       }   
  }

if(display >=1) message.display("Test status");
*/

 std::cout<<"########################### "<< std::endl;
 std::cout<<"####### END OF TEST ####### "<< std::endl;
 std::cout<<"########################### "<< std::endl;

 }





};//DAQ_TEST class




#endif// DAQ_TEST

