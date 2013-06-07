#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILENAME "data_section.s"

void convert_char_to_binary(char* string, char value) {
  unsigned mask = 0x80;

  while(mask > 0x00) {
    if((value & mask) != 0)
      strcat(string, "1");
    else
      strcat(string, "0");
    mask = mask >> 1;
  }
}

void convert_short_to_binary(char* string, short value) {
  unsigned mask = 0x8000;

  while(mask > 0) {
    if((value & mask) != 0)
      strcat(string, "1");
    else
      strcat(string, "0");
    mask = mask >> 1;
  }
}

void convert_word_to_binary(char* string, int value) {
  unsigned mask = 0x80000000;

  while(mask > 0x00000000) {
    if((value & mask) != 0)
      strcat(string, "1");
    else
      strcat(string, "0");
    mask = mask >> 1;
  }
}

void convert_to_binary(char* string, void* value, size_t type) {
  switch(type) {
    case sizeof(char): {
      char* data = (char*)value;
      printf("value to be converted = %02x\n", *data);
      convert_char_to_binary(string, *data);
      printf("string output = %s\n", string);
      break;
    }
    case sizeof(short): {
      short* data = (short*)value;
      printf("value to be converted = %04x\n", *data);
      convert_short_to_binary(string, *data);
      printf("string output = %s\n", string);
      break;
    }
    case sizeof(int): {
      int* data = (int*)value;
      printf("value to be converted = %08x\n", *data);
      convert_word_to_binary(string, *data);
      printf("string output = %s\n", string);
      break;
    }
  }
}

void print_data(void* data, size_t size, int num_items) {
  FILE* pFile;
  static int device_mem_ptr = 0x34;

  // Calculate the total number of bytes, = size * number
  unsigned total_bytes = size * num_items;
  printf("total_bytes = %d\n", total_bytes);

  //int* data_ptr = (int*)&data;

  pFile = fopen("data_section.s", "a");
  // Write the header, defining how long the data section is.
  //char data_header[50]={0};
  //sprintf(data_header, "##Data Section - %d - Data_align=32\n",
    //      (int)sizeof(dat));
  //fputs(data_header, pFile);

  // Divide the number by 4 to get the number of full 32-bit data lines
  unsigned index = 0;
  for (unsigned i = 0; i < (total_bytes >> 2); ++i) {
    char data_line[50] = {0};
    char binary_string[33] = {'\0'};
    index = i << 2;

    switch(size) {
    case sizeof(char): {
      char* bytes = (char*)&data[index];
      convert_to_binary(binary_string, &bytes[0], sizeof(char));
      convert_to_binary(binary_string, &bytes[1], sizeof(char));
      convert_to_binary(binary_string, &bytes[2], sizeof(char));
      convert_to_binary(binary_string, &bytes[3], sizeof(char));
      sprintf(data_line, "%05x - %02x%02x%02x%02x - %s\n", device_mem_ptr,
              bytes[0],
              bytes[1],
              bytes[2],
              bytes[3],
              binary_string);
      break;
    }
    case sizeof(short): {
      short* halves = (short*)&data[index];
      convert_to_binary(binary_string, &halves[0], sizeof(short));
      convert_to_binary(binary_string, &halves[1], sizeof(short));
      sprintf(data_line, "%05x - %04x%04x - %s\n", device_mem_ptr,
              halves[0],
              halves[1],
              binary_string);
      break;
    }
    case sizeof(int): {
      int* word = (int*)&data[index];
      convert_to_binary(binary_string, &word[0], sizeof(int));
      sprintf(data_line, "%05x - %08x - %s\n", device_mem_ptr, word[0],
              binary_string);
      break;
    }
    }

    fputs(data_line, pFile);
    device_mem_ptr += 4;
  }
  // Mask total number of bytes with 0011. If the result is 0
  // then the number of bytes is divisible by 4. If not the function can be
  // called with a value that is zero padded.
  unsigned remaining_bytes = total_bytes & 0x3;

  if(remaining_bytes != 0) {

    char data_line[50] = {0};
    char binary_string[33] = {0};
    switch(size) {
      case sizeof(char): {
        char* remaining_data = (char*)&data[index+4];
        unsigned padded_data = 0;
        char* padded_array[4] = {0};
        for(unsigned i = 0; i < remaining_bytes; ++i) {
          padded_data |= remaining_data[i] << (12 << i);
          padded_array[i] = remaining_data[i];
        }
        convert_to_binary(binary_string, &padded_array[0], sizeof(char));
        convert_to_binary(binary_string, &padded_array[1], sizeof(char));
        convert_to_binary(binary_string, &padded_array[2], sizeof(char));
        convert_to_binary(binary_string, &padded_array[3], sizeof(char));
        sprintf(data_line, "%05x - %02x%02x%02x%02x - %s\n", device_mem_ptr,
                padded_array[0],
                padded_array[1],
                padded_array[2],
                padded_array[3],
                binary_string);
        break;
      }
      case sizeof(short): {
        short* remaining_data = (short*)&data[index+4];
        unsigned padded_data = 0;
        short* padded_array[2] = {0};
        for(unsigned i = 0; i < (remaining_bytes >> 1); ++i) {
          padded_data |= remaining_data[i] << (i << 16);
          padded_array[i] = remaining_data[i];
        }
        convert_to_binary(binary_string, &padded_array[0], sizeof(short));
        convert_to_binary(binary_string, &padded_array[1], sizeof(short));
        sprintf(data_line, "%05x - %04x%04x - %s\n", device_mem_ptr,
                padded_array[0],
                padded_array[1],
                binary_string);
        break;
      }
    }
    fputs(data_line, pFile);
    device_mem_ptr += 4;
  }

  fclose(pFile);

}

int write_dram(cl_kernel kernel) {

  // Need to iterate through the kernel arguments, populating the data section
  // with the global data.
  for(unsigned i = 0; i < kernel->num_args; ++i) {

    pocl_argument* arg = kernel->arguments[i];
    // Here I should check what type the argument is...

    // The number of elements are defined when the argument is set, so iterate
    // through them all.
    for(size_t j = 0; j < arg->size; j++) {
      size_t val = arg->value[j];

      // The Data Section text output file needs to look like this:
      // ADDR - HEX_DATA - BINARY_DATA
      // ie.
      // 0034 - 00000002 - 00000000000000000000000000000000

      char data_line[LINE_LENGTH] = {0};
      // Zero pad, using a minimum of 5 characters
      sprintf(data_line, "%05x - ", device_mem_ptr);
    }
  }
}

int main (void)
{
  unsigned dataA[] = {0,31,64,511,1023,1024};
  unsigned short dataB[] = {10, 100, 1000, 10000, 50, 500};
  char bytes[10] = {0,1,2,3,4,5,6,7,8,9};
  short halves[4] = {100, 1000, 1023, 10000};
  int words[2] = {65300, 1512};

  for (unsigned i = 0; i < 10; ++i)
    printf("Address of byte[%d] = %p\n", i, &bytes[i]);
  print_data(bytes, sizeof(char), 10);

  for (unsigned i = 0; i < 4; ++i)
    printf("Address of halves[%d] = %p\n", i, &halves[i]);
  print_data(halves, sizeof(short), 4);

  for (unsigned i = 0; i < 2; ++i)
    printf("Address of words[%d] = %p\n", i, &words[i]);
  print_data(words, sizeof(int), 2);

  // Write the data, line by line. First the address, followed by the data in
  // hex and then binary.
  /*
  for(unsigned i = 0; i < 6; i++) {

    char data_line[50] = {0};
    char binary_string[32]={0};

    convert_to_binary(binary_string, dataA[i]);
    sprintf(data_line, "%05x - %08x - %s\n", device_mem_ptr, dataA[i],
            binary_string);
    fputs(data_line, pFile);
    device_mem_ptr += sizeof(dataA[i]);
  }*/

  return 0;
}
