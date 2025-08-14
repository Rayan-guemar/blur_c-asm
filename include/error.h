
#ifndef ERROR_H
#define ERROR_H

void exit_wrong_file_type(void);
void exit_error_while_reading_file(void);
void exit_memory_allocation_error(void);
void exit_file_open_error(const char* filename);
void exit_file_write_error(const char* filename);
void exit_file_read_error(const char* filename);
void exit_invalid_chunk_error(const char* chunk_id);
void exit_error_message(const char* msg);

#endif  // ERROR_H
