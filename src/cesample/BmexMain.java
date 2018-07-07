package cesample;

import java.util.Iterator;
import java.util.ResourceBundle;
import java.util.Vector;

import javax.swing.table.DefaultTableModel;

import org.apache.log4j.Logger;

import com.filenet.api.collection.RepositoryRowSet;
import com.filenet.api.core.ObjectStore;
import com.filenet.api.exception.EngineRuntimeException;
import com.filenet.api.query.RepositoryRow;

public class BmexMain {

	public static void main(String[] args) {
		BmexMain m= new BmexMain();
		m.execute();
		
		

	}
	private CEConnection ce = new CEConnection();
	private static Logger log = Logger.getLogger(BmexMain.class);
	private static ResourceBundle bundle = ResourceBundle.getBundle("filenet");


	private void executeQuery() {
        try
        {
        	{
        		String osname = bundle.getString("objectStore");
        		ObjectStore os = ce.fetchOS(osname);
        		Vector rows = new Vector();
        		Vector columns = new Vector();
        		int mRows = 999999	;
        		String select="DocumentTitle";
        		String from="ExpedientesDC";
				String where="VersionStatus=1 ";//
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
        			System.out.println(rows);
        		}
        	}
        }
        catch (EngineRuntimeException e)
        {
        	e.printStackTrace();
        }
	}
	
	private void execute() {
		ce.establishConnection(bundle.getString("usr"),bundle.getString("pwd"),bundle.getString("context"),bundle.getString("url"));
		executeQuery();
	}

}
