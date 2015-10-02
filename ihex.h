// read hex file into buffer, return pointer to buffer or NULL if error.
// caller must free returned buffer.
//   fileName: name of hex file to load
//   start: pointer to start address (to be filled in)
//   length: pointer to length of image (to be filled in)
extern unsigned char *ReadHexFile(const char *fileName,int *start,int *length);

