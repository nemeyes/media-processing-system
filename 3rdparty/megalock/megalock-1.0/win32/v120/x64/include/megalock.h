#ifdef __cplusplus						
	extern "C" {
#endif
		extern unsigned int  lock_init_usb(unsigned int op1);		// �ް��� Ž��
		extern unsigned char lock_write(unsigned char op1, char op2);
		extern unsigned char lock_read(unsigned char op1);

		extern unsigned int lock_boot_cnt(void);		// ���� Ƚ�� ���
		extern unsigned int lock_timeout_cnt(void);		// ��� ���� ������ ���� ������ ��������� ����� �ʿ䰡 �����ϴ�.
		extern unsigned char lock_version(void);
		extern unsigned char lock_sn(char);				// �ް��� ������ȣ
		extern unsigned char lock_check(void);			// �ް��� ���� ���θ� ������ ������ üũ
		extern void lock_write_enable(char lock_op1);
		extern int lock_write_ex(int op1, int op2);
		extern int lock_read_ex(int op1);
		extern BOOL lock_receive(void);

		// ���� ������ ȣȯ�� ������ ���� �Լ�, �űԾ�ü�� ��� �Ʒ��� �Լ��� ������� ������.
		extern unsigned char lock_auto(char op1);
		extern unsigned char lock_func0(unsigned char op1, unsigned char op2);
		extern unsigned char lock_func1(unsigned char op1, unsigned char op2);
		extern unsigned char lock_func2(unsigned char op1, unsigned char op2);
		extern unsigned char lock_func3(unsigned char op1, unsigned char op2);

#ifdef __cplusplus						
	}
#endif