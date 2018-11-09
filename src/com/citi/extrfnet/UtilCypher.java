package com.citi.extrfnet;

import java.security.Key;
import java.security.NoSuchAlgorithmException;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.spec.SecretKeySpec;

public class UtilCypher {

	public String encrypt(String texto) throws Exception {
		// Generamos una clave de 128 bits adecuada para AES
		KeyGenerator keyGenerator = KeyGenerator.getInstance("AES");
		keyGenerator.init(128);
		Key key = keyGenerator.generateKey();

		// Alternativamente, una clave que queramos que tenga al menos 16 bytes
		// y nos quedamos con los bytes 0 a 15
		key = new SecretKeySpec("una clave de 16 bytes".getBytes(), 0, 16, "AES");

		// Ver como se puede guardar esta clave en un fichero y recuperarla
		// posteriormente en la clase RSAAsymetricCrypto.java

		// Se obtiene un cifrador AES
		Cipher aes = Cipher.getInstance("AES/ECB/PKCS5Padding");

		// Se inicializa para encriptacion y se encripta el texto,
		// que debemos pasar como bytes.
		aes.init(Cipher.ENCRYPT_MODE, key);
		byte[] encriptado = aes.doFinal(texto.getBytes());

		return new String(encriptado);
	}

	public String decrypt(String sEncript) throws Exception {
		KeyGenerator keyGenerator = KeyGenerator.getInstance("AES");
		keyGenerator.init(128);
		Key key = keyGenerator.generateKey();

		// Alternativamente, una clave que queramos que tenga al menos 16 bytes
		// y nos quedamos con los bytes 0 a 15
		key = new SecretKeySpec("una clave de 16 bytes".getBytes(), 0, 16, "AES");

		// Se iniciliza el cifrador para desencriptar, con la
		// misma clave y se desencripta
		Cipher aes = Cipher.getInstance("AES/ECB/PKCS5Padding");
		aes.init(Cipher.DECRYPT_MODE, key);
		byte[] desencriptado = aes.doFinal(sEncript.getBytes());

		// Texto obtenido, igual al original.
		return (new String(desencriptado));
	}
	
	public static void main(String[] args) throws Exception {
		UtilCypher c=new UtilCypher();
		String t=c.encrypt("sistemas2012");
		System.out.println(t);
		System.out.println(c.decrypt(t));
	}
}
