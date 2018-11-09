package com.citi.extrfnet;

import java.io.File;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.ResourceBundle;
import java.util.Vector;

import javax.swing.table.DefaultTableModel;

import org.apache.log4j.Logger;

import com.filenet.api.collection.RepositoryRowSet;
import com.filenet.api.constants.RefreshMode;
import com.filenet.api.core.Document;
import com.filenet.api.core.Factory;
import com.filenet.api.core.ObjectStore;
import com.filenet.api.exception.EngineRuntimeException;
import com.filenet.api.query.RepositoryRow;

public class BmexMain {

	private static final String EXPEDIENTES_DC = "ExpedientesDC";


	public static void main(String[] args) {
		int opc=0;
		String fOp=null;
		if(args!=null && args.length>0){
			opc=Integer.parseInt(args[1]);
			fOp=(args[2]);
		}
		BmexMain m= new BmexMain();
		String locationBundle=args[0];
		setBundle(locationBundle);
		m.execute(opc,fOp);
		
		

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
	private static Logger log = Logger.getLogger(BmexMain.class);
	private static ResourceBundle bundle = ResourceBundle.getBundle("filenet");


	private List executeQuery(int opc,String fechaOperacion,String signo, String valor) {
		List results=new ArrayList<>();
        try
        {
        	{
        		String osname = bundle.getString("objectStore");
        		ObjectStore os = ce.fetchOS(osname);
        		Vector rows = new Vector();
        		Vector columns = new Vector();
        		int mRows = 999999	;
        		String select="Id,NumCliente,TipoDoc,XfolioS,Contrato,Linea,Producto,Instrumento,FolioS403,UOC,Folio,CalificaOnDemand,Status,XfolioP,FechaOperacion";
        		String from=EXPEDIENTES_DC;
				String where="VersionStatus=1 ";//
				where+=" and CalificaOnDemand=1 ";//
				switch (opc) {
				case 1:
					where+=" and Status<3 ";//
					break;
				case 2:
					String []args=fechaOperacion.split("/");
					String formatDate=args[2]+args[0]+args[1]+"T000000Z ";//comes MM/dd/yyyy need yyyy/MM/dd
					where+=" and Status=3 ";//
					where+=" and FechaOperacion="+formatDate;//
					break;

				default:
					break;
				}
				//TODO. setup is missing
				//where+=" and UOC "+signo+" "+valor;//
				where+=" and Xfolios IS NOT NULL ";//
				System.out.println("Where..."+where);
				RepositoryRowSet rrs = CEUtil.fetchResultsRowSet(os, select, from, where, mRows);
        		Iterator it = rrs.iterator();
        		boolean firstPass = true;
        		
        		while(it.hasNext())
        		{
        			RepositoryRow rr = (RepositoryRow) it.next();
        			if(firstPass)
        			{
        				columns = CEUtil.getResultProperties(rr);
        				firstPass = false;
        			}
        			rows = CEUtil.getResultRow(rr);
        			
        			String docTitle=CEUtil.writeDocContentToFile(CEUtil.fetchDocById(os, rr.getProperties().getIdValue("Id").toString()), bundle.getString("folderDestination"));
        			rows.add(docTitle);
        			
        			results.add(rows);
        		}
        		
        	}
        }
        catch (EngineRuntimeException e)
        {
        	e.printStackTrace();
        }
        return results;
	}
	
	private void execute(int opc,String fOp) {
		UtilCypher cypher=new UtilCypher();
		String pwd="";
		try {
			pwd=cypher.decrypt(bundle.getString("pwd"));
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		ce.establishConnection(bundle.getString("usr"),pwd,bundle.getString("context"),bundle.getString("url"));
		List results=executeQuery(opc,fOp,null,null);
		CsvFileWriter.writeCsvFile(bundle.getString("fileName"),results);
		procesaIndices(results,opc,fOp);
	}

	
	
	private void procesaIndices(List<Vector> results, int opc, String fOp) {
		String osname = bundle.getString("objectStore");
		ObjectStore os = ce.fetchOS(osname);
		for (Vector row : results) {
			Document doc = Factory.Document.fetchInstance(os, row.get(0).toString(),null);
	        doc.getProperties().putValue("Status", 3);
	        //doc.getProperties().putValue(arg0, arg1);
	        doc.save(RefreshMode.REFRESH);
		}
		System.out.println("Updated "+results.size()+" records...");
		
	}

}
