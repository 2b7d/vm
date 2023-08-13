void meminit(void *mem, int data_size, int cap);
void memfree(void *mem);

void memgrow(void *mem);

void *memnext(void *mem);
