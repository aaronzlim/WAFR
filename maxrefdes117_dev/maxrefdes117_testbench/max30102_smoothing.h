/*
 * Author: Aaron Lim
 * Project: WAFR
 * Filename: max30102_smoothing.h
 * Description: Data smoothing algorithm for the max30102 pulse oximetry sensor
 * 
 * Revision History:
 * 2-14-17 Rev 1.00 Initial Release
 * 
 */

#ifndef max30102_smoothing_H
#define max30102_smoothing_H

#define FS 25 // Sampling frequency in Hz
// NOTE: The actual sampling frequency is set to 400Hz with an averaging factor of 4.
//       This gives an implemented sampling frequency of 100Hz
#define BUFFER_SIZE ( FS * 4 ) // 100 samples (4s of data)
#define MVG_AVG_SIZE 4
#define MAX_NUM_PEAKS 15
#define RATIO_BUFFER_SIZE 5

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

static int32_t tmp_ir[ BUFFER_SIZE ];
static int32_t tmp_red[ BUFFER_SIZE ];


void max30102_calc_hr_spo2(uint32_t *red_buffer, uint32_t *ir_buffer, int32_t *spo2, 
                           int8_t *spo2_valid, int32_t *heart_rate, int8_t *hr_valid);

void find_peaks(int32_t *ir_locs, int32_t *num_peaks, int32_t *tmp_ir, uint32_t threshold, uint32_t min_width, uint32_t max_num_peaks);

void peaks_above_min_height(int32_t *ir_locs, int32_t *num_peaks, int32_t *tmp_ir, int32_t *threshold, uint32_t max_num_peaks);

void remove_close_peaks(uint32_t *tmp_ir, uint32_t *ir_locs, uint32_t *num_peaks, uint32_t min_width);

void sort_ascending(uint32_t *arr, uint32_t len);







#endif // max30102_smoothing_H
