/*
 * Author: Aaron Lim
 * Project: WAFR
 * Filename: max30102_processing.h
 * Description: Data smoothing algorithm for the max30102 pulse oximetry sensor
 * 
 * Revision History:
 * 2-14-17 Rev 1.00 Initial Release
 * 
 */

#ifndef max30102_processing_H
#define max30102_processing_H

#define FS 25 // Sampling frequency in Hz
#define DATA_WIN_SECS 4 // How many seconds of data is processed at once
// NOTE: The actual sampling frequency is set to 100Hz with an averaging factor of 4.
//       This gives an implemented sampling frequency of 25Hz
#define BUFFER_SIZE ( FS * DATA_WIN_SECS ) // 100 samples (4s of data)
#define MVG_AVG_SIZE 4 // Four point moving average
#define MAX_NUM_PEAKS 15 // 15 peaks in 100 samples (4sec)
#define MIN_PEAK_WIDTH 7     // 8 = max discernable HR is 180 bpm
                             // 7 = max discernable HR is 210 bpm
                             // 6 = max discernable HR is 225 bpm
#define INITIAL_SAMPLE_SIZE 5 // Number of loops to fill hr and spo2 buffers initially
#define PEAKS_TO_HR(peaks) (peaks*60)/(DATA_WIN_SECS*INITIAL_SAMPLE_SIZE)

/*
// spo2_table is approximated as -45.060 * ratioAverage * ratioAverage + 30.354 * ratioAverage + 94.845 ;
const uint8_t spo2_table[184]={ 95, 95, 95, 96, 96, 96, 97, 97, 97, 97, 97, 98, 98, 
              98, 98, 98, 99, 99, 99, 99, 99, 99, 99, 99, 100, 100, 100, 100, 100, 100, 
              100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 99, 
              99, 99, 99, 99, 99, 99, 99, 98, 98, 98, 98, 98, 98, 97, 97, 97, 97, 96, 96, 
              96, 96, 95, 95, 95, 94, 94, 94, 93, 93, 93, 92, 92, 92, 91, 91, 90, 90, 89, 
              89, 89, 88, 88, 87, 87, 86, 86, 85, 85, 84, 84, 83, 82, 82, 81, 81, 80, 80, 
              79, 78, 78, 77, 76, 76, 75, 74, 74, 73, 72, 72, 71, 70, 69, 69, 68, 67, 66, 
              66, 65, 64, 63, 62, 62, 61, 60, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 
              49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 31, 30, 
              29, 28, 27, 26, 25, 23, 22, 21, 20, 19, 17, 16, 15, 14, 12, 11, 10, 9, 7, 6, 
              5, 3, 2, 1 } ;
*/

static int32_t tmp_ir[ BUFFER_SIZE ];
static int32_t tmp_red[ BUFFER_SIZE ];


float get_spo2_ratio(uint32_t *red_buffer, uint32_t *ir_buffer);
                           
uint32_t get_num_peaks(uint32_t *ir_buffer);

void peaks_above_min_height(int32_t *ir_locs, uint32_t *num_peaks, int32_t *tmp_ir, uint32_t threshold, uint32_t max_num_peaks);

void remove_close_peaks(int32_t *tmp_ir, int32_t *ir_locs, uint32_t *num_peaks, uint32_t min_width);

void sort_ascending(int32_t *arr, uint32_t len);

void sort_descending(int32_t *arr, uint32_t len);







#endif // max30102_processing_H
