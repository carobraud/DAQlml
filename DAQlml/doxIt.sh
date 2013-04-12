#! /bin/bash

##list='HPIVlml CImg.PlugIn PCO LaVision raw Hiris CImg.Tool NetCDF.Tool CImg'
list='DAQlml'
#move code versions to temporary directory
for d in $list
do
  mkdir -p ../DoxIt.tmp/$d
  mv ../$d/*.v?.?.?.* ../DoxIt.tmp/$d
done
#get HPIVlml help output (xterm-color unformated)
dir=../DAQlml
bin=$dir/DAQlml
out=DAQlml.help
$bin -h 2> $dir/$out; cat -v $dir/$out | sed 's/\^\[\[//g' | sed 's/1m//g' | sed 's/0;0;0m//g' | sed 's/0;35;59m//g' | sed 's/0;32;59m//g' | sed 's/4;31;59m//g' > $dir/$out.output;rm $dir/$out
#create documentation
doxygen DAQlml.Doxygen
#put back code versions
for d in $list
do
  mv ../DoxIt.tmp/$d/* ../$d/
  rmdir ../DoxIt.tmp/$d
done
rmdir ../DoxIt.tmp

