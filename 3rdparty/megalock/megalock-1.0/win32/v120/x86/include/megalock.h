#ifdef __cplusplus						
	extern "C" {
#endif
		extern unsigned int  lock_init_usb(unsigned int op1);		// 메가락 탐색
		extern unsigned char lock_write(unsigned char op1, char op2);
		extern unsigned char lock_read(unsigned char op1);

		extern unsigned int lock_boot_cnt(void);		// 부팅 횟수 얻기
		extern unsigned int lock_timeout_cnt(void);		// 통신 감도 측정을 위한 것으로 통상적으로 사용할 필요가 없습니다.
		extern unsigned char lock_version(void);
		extern unsigned char lock_sn(char);				// 메가락 고유번호
		extern unsigned char lock_check(void);			// 메가락 존재 여부를 빠르고 간단히 체크
		extern void lock_write_enable(char lock_op1);
		extern int lock_write_ex(int op1, int op2);
		extern int lock_read_ex(int op1);
		extern BOOL lock_receive(void);

		// 이전 버전과 호환성 유지를 위한 함수, 신규업체인 경우 아래의 함수는 사용하지 마세요.
		extern unsigned char lock_auto(char op1);
		extern unsigned char lock_func0(unsigned char op1, unsigned char op2);
		extern unsigned char lock_func1(unsigned char op1, unsigned char op2);
		extern unsigned char lock_func2(unsigned char op1, unsigned char op2);
		extern unsigned char lock_func3(unsigned char op1, unsigned char op2);

#ifdef __cplusplus						
	}
#endif