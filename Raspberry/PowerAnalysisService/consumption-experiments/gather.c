/*
* The current sensing file for SlugCam's power analysis experiments
* Version: 1.0
* Date: 9/23/14
*/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <wiringPi.h>
#include <mcp3004.h>

#define SENSOROFFSET 128

int main ( int argc, char *argv [ ] ) {
      int curr_adc_value;
      int miliamps;
      FILE *sensorReadingsFile;
     
      if ( argc < 3 ) {
          fprintf(stderr, "%s: Invalid number of arguments.\nFormat: ./gather <sampling_rate> <Length_of_experiment> <Data_File_Name> \n","InputErr");
          exit (0);
      }      
	
      sensorReadingsFile = fopen ( argv[3], "w+" );      
      if ( sensorReadingsFile == 0 ){
          fprintf(stderr, "%s: Unable to open File.\n","FileI/O");
          exit (0);          
      }

     /*
      *Initializing mcp3008 is the same as mcp3004, except you have
      * pins 100-107 available, and then you send the spi slave select "0"
     */  	
      wiringPiSetup();
      mcp3004Setup ( 100, 0 );

      curr_adc_value = analogRead (100);

      printf ( "adc: %d\n", curr_adc_value );
      
      miliamps = (curr_adc_value - 512 - SENSOROFFSET) * 0.0264;
      
      printf ( "Measured: %d mA\n",miliamps);

      fprintf(sensorReadingsFile, "%d\t%d\n",0,miliamps);
	
      if ( fclose(sensorReadingsFile) < 0 ){
          fprintf(stderr, "%s: Unable to close File.\n","FileI/O");
          exit (0);
      } 

      return 0;
}
