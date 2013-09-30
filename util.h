#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

void die(const char *e);
void require64BitLongs(void);
