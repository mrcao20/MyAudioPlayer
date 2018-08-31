import binascii
import os

def createSecretKey(size):
	return ''.join( [ hex(x)[2:] for x in os.urandom(size) ] )[0:16]
	
def rsaEncrypt(text, pubKey, modulus):
	text = text[::-1].encode()
	rs = int(binascii.hexlify(text), 16)**int(pubKey, 16) % int(modulus, 16)
	return format(rs, 'x').zfill(256)
