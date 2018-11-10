package com.citi.extrfnet;

import java.io.File;
import java.net.URL;
import java.net.URLClassLoader;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.ResourceBundle;
import java.util.Vector;

import javax.swing.table.DefaultTableModel;

import org.apache.log4j.Logger;

import com.filenet.api.collection.RepositoryRowSet;
import com.filenet.api.constants.AutoClassify;
import com.filenet.api.constants.CheckinType;
import com.filenet.api.constants.RefreshMode;
import com.filenet.api.core.Document;
import com.filenet.api.core.Factory;
import com.filenet.api.core.ObjectStore;
import com.filenet.api.core.ReferentialContainmentRelationship;
import com.filenet.api.exception.EngineRuntimeException;
import com.filenet.api.query.RepositoryRow;
import com.filenet.apiimpl.util.ExportXML;

public class BmexImportMain {

	private static final String FOLDER_LOCATION = "/test/cetest";
	//private static final String EXPEDIENTES_DC = "Document";
	private static final String EXPEDIENTES_DC = "ExpedientesDC";


	public static void main(String[] args) {
		BmexImportMain m= new BmexImportMain();
		String locationBundle=args[0];
		setBundle(locationBundle);
		m.execute(args[1]);
		
		

	}
	private static void setBundle(String locationBundle) {
		try {
		File file = new File(locationBundle);
		URL[] urls = {file.toURI().toURL()};
		ClassLoader loader = new URLClassLoader(urls);
		bundle = ResourceBundle.getBundle("filenet", Locale.getDefault(), loader);
		}catch (Exception e) {
			e.printStackTrace();
		}
		
	}
	private CEConnection ce = new CEConnection();
	private static Logger log = Logger.getLogger(BmexImportMain.class);
	private static ResourceBundle bundle = ResourceBundle.getBundle("filenet");

	private void execute(String fName) {
		UtilCypher cypher=new UtilCypher();
		String pwd="";
		try {
			pwd=cypher.decrypt(bundle.getString("pwd"));
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		ce.establishConnection(bundle.getString("usr"),pwd,bundle.getString("context"),bundle.getString("url"));
		List<String[]>records=CsvFileReader.readCsvFile(fName);
		createRecords(records);
	}

	private void createRecords(List<String[]>results) {
		
		String osname = bundle.getString("objectStore");
		ObjectStore os = ce.fetchOS(osname);
		for (String[] row : results) {
			//19
			//Id,NumCliente,TipoDoc,XfolioS,Contrato,Linea,Producto,Instrumento,FolioS403,UOC,Folio,CalificaOnDemand,Status,XfolioP,FechaOperacion
			//" SecLote": Statusimagen":"fileName
			String fileName=row[19];
			//String fileName=row[0];
			Document doc;
			try {
				doc = CEUtil.createDocWithContent(new File(fileName), MimeUtil.getMimeType(fileName), os,row[1] ,EXPEDIENTES_DC);
				int i=1;
	//			doc.getProperties().putObjectValue("DocumentTitle", row[i++]);
				doc.getProperties().putObjectValue("NumCliente", row[i++]);
				doc.getProperties().putObjectValue("TipoDoc", Integer.parseInt(row[i++]));
				doc.getProperties().putObjectValue("XfolioS", Integer.parseInt(row[i++]));
				doc.getProperties().putObjectValue("Contrato", (row[i++]));
				doc.getProperties().putObjectValue("Linea", Integer.parseInt(row[i++]));
				
				doc.getProperties().putObjectValue("Producto", Integer.parseInt(row[i++]));
				doc.getProperties().putObjectValue("Instrumento", Integer.parseInt(row[i++]));
				doc.getProperties().putObjectValue("FolioS403", Integer.parseInt(row[i++]));
				doc.getProperties().putObjectValue("UOC", Integer.parseInt(row[i++]));
				doc.getProperties().putObjectValue("Folio", Integer.parseInt(row[i++]));
				
				doc.getProperties().putObjectValue("CalificaOnDemand", Integer.parseInt(row[i++]));
				doc.getProperties().putObjectValue("Status", Integer.parseInt(row[i++]));
				doc.getProperties().putObjectValue("XfolioP", (row[i++]));
				doc.getProperties().putObjectValue("FechaOperacion", new SimpleDateFormat("dd/MM/yyyy").format(row[i++]));
				
				
				doc.getProperties().putObjectValue("SecLote", Integer.parseInt(row[i++]));
				doc.getProperties().putObjectValue("StatusImagen", Integer.parseInt(row[i++]));

				doc.checkin(AutoClassify.DO_NOT_AUTO_CLASSIFY, CheckinType.MAJOR_VERSION);
				doc.save(RefreshMode.REFRESH);
		        ReferentialContainmentRelationship rcr = CEUtil.fileObject(os, doc, FOLDER_LOCATION);
		        rcr.save(RefreshMode.REFRESH);
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
		}
		System.out.println("Created "+results.size()+" records...");
	}
	
}
