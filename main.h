// @@@ eliminate this, move these functions into a more appropriate module

int ReadPartID(int fd);
int ReadPartUID(int fd, unsigned int *uid);
int ReadBootCode(int fd, unsigned char *major, unsigned char *minor);
