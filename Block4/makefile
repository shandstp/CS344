all: otp_enc_d otp_enc otp_dec_d otp_dec keygen 

otp_enc_d: otp_enc_d.c
	gcc -g -std=gnu99 -Wall otp_enc_d.c -o otp_enc_d 

otp_enc: otp_enc.c
	gcc -g -std=gnu99 -Wall otp_enc.c -o otp_enc

otp_dec_d: otp_dec_d.c
	gcc -g -std=gnu99 -Wall otp_dec_d.c -o otp_dec_d

otp_dec: otp_dec.c
	gcc -g -std=gnu99 -Wall otp_dec.c -o otp_dec

keygen: keygen.c
	gcc -g -std=gnu99 -Wall keygen.c -o keygen

clean:
	rm -f otp_enc_d otp_enc otp_dec_d otp_dec keygen 
