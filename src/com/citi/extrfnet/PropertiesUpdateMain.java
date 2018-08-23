package com.citi.extrfnet;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.net.URL;
import java.net.URLClassLoader;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Properties;
import java.util.ResourceBundle;
import java.util.Vector;

import javax.swing.table.DefaultTableModel;

import org.apache.log4j.Logger;

import com.filenet.api.collection.RepositoryRowSet;
import com.filenet.api.constants.RefreshMode;
import com.filenet.api.core.Document;
import com.filenet.api.core.Factory;
import com.filenet.api.core.ObjectStore;
import com.filenet.api.core.ReferentialContainmentRelationship;
import com.filenet.api.exception.EngineRuntimeException;
import com.filenet.api.query.RepositoryRow;
import com.filenet.apiimpl.util.ExportXML;

public class PropertiesUpdateMain {

	public static void main(String[] args) throws Exception {
		PropertiesUpdateMain m = new PropertiesUpdateMain();
		String locationBundle = args[0];
		setBundle(locationBundle);
		m.updatePasswordInBundle(locationBundle, args[1]);
	}

	private static void setBundle(String locationBundle) {
		try {
			File file = new File(locationBundle);
			URL[] urls = { file.toURI().toURL() };
			ClassLoader loader = new URLClassLoader(urls);
			bundle = ResourceBundle.getBundle("filenet", Locale.getDefault(), loader);
		} catch (Exception e) {
			e.printStackTrace();
		}

	}

	private static Logger log = Logger.getLogger(PropertiesUpdateMain.class);
	private static ResourceBundle bundle = ResourceBundle.getBundle("filenet");

	private void updatePasswordInBundle(String folder, String flatPwd) throws Exception {
		UtilCypher cypher = new UtilCypher();
		String pwd = "";
		try {
			pwd = cypher.encrypt(flatPwd);

		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		File file = new File(folder + "/filenet.properties");
		Properties prop = new Properties();
		FileInputStream instream = new FileInputStream(file);
		prop.load(instream);
		instream.close();
		prop.setProperty("pwd", pwd); // change whatever you want here
		FileOutputStream outstream = new FileOutputStream(file);
		prop.store(outstream, "comments go here");
		outstream.close();
		System.out.println("Password updated!!!");
		log.debug("Password updated!!!");
	}

}
