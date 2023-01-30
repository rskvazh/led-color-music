// система может работать С ЛЮБЫМ ИК ПУЛЬТОМ (практически). 
// Коды для своего пульта можно задать начиная со строки 160 в прошивке. Коды пультов определяются скетчем IRtest_2.0, читай инструкцию
#define REMOTE_TYPE 1  // 0 - без пульта, 1 - пульт от WAVGAT, 2 - пульт от KEYES, 3 - кастомный пульт

// ----- КНОПКИ ПУЛЬТА WAVGAT -----
#if REMOTE_TYPE == 1
#define BUTT_UP 0xF39EEBAD
#define BUTT_DOWN 0xC089F6AD
#define BUTT_LEFT 0xE25410AD
#define BUTT_RIGHT 0x14CE54AD
#define BUTT_OK 0x297C76AD
#define BUTT_1 0x4E5BA3AD
#define BUTT_2 0xE51CA6AD
#define BUTT_3 0xE207E1AD
#define BUTT_4 0x517068AD
#define BUTT_5 0x1B92DDAD
#define BUTT_6 0xAC2A56AD
#define BUTT_7 0x5484B6AD
#define BUTT_8 0xD22353AD
#define BUTT_9 0xDF3F4BAD
#define BUTT_0 0xF08A26AD
#define BUTT_STAR 0x68E456AD
#define BUTT_HASH 0x151CD6AD
#endif

// ----- КНОПКИ ПУЛЬТА KEYES -----
#if REMOTE_TYPE == 2
#define BUTT_UP 0xE51CA6AD
#define BUTT_DOWN 0xD22353AD
#define BUTT_LEFT 0x517068AD
#define BUTT_RIGHT 0xAC2A56AD
#define BUTT_OK 0x1B92DDAD
#define BUTT_1 0x68E456AD
#define BUTT_2 0xF08A26AD
#define BUTT_3 0x151CD6AD
#define BUTT_4 0x18319BAD
#define BUTT_5 0xF39EEBAD
#define BUTT_6 0x4AABDFAD
#define BUTT_7 0xE25410AD
#define BUTT_8 0x297C76AD
#define BUTT_9 0x14CE54AD
#define BUTT_0 0xC089F6AD
#define BUTT_STAR 0xAF3F1BAD
#define BUTT_HASH 0x38379AD
#endif

// ----- КНОПКИ СВОЕГО ПУЛЬТА -----
#if REMOTE_TYPE == 3
#define BUTT_UP 0xE51CA6AD
#define BUTT_DOWN 0xD22353AD
#define BUTT_LEFT 0x517068AD
#define BUTT_RIGHT 0xAC2A56AD
#define BUTT_OK 0x1B92DDAD
#define BUTT_1 0x68E456AD
#define BUTT_2 0xF08A26AD
#define BUTT_3 0x151CD6AD
#define BUTT_4 0x18319BAD
#define BUTT_5 0xF39EEBAD
#define BUTT_6 0x4AABDFAD
#define BUTT_7 0xE25410AD
#define BUTT_8 0x297C76AD
#define BUTT_9 0x14CE54AD
#define BUTT_0 0xC089F6AD
#define BUTT_STAR 0xAF3F1BAD  // *
#define BUTT_HASH 0x38379AD   // #
#endif