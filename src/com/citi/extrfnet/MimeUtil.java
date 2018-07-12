package com.citi.extrfnet;

import java.util.HashMap;

import javax.management.RuntimeErrorException;

public class MimeUtil {

private MimeUtil() {
		
	}
	
    protected static final class PropertyKeys {
        private static HashMap<String, String> hs;
        static {
            hs = new HashMap<String, String>();
            hs.put("doc", "application/msword");
            hs.put("tif", "image/tiff");
            hs.put("dot", "application/msword");
            hs.put("pdf", "application/pdf");
            hs.put("jpg", "image/jpg");
            hs.put("jpeg", "image/jpeg");
            hs.put("gif", "image/gif");
            hs.put("png", "image/x-png");
            hs.put("txt", "text/plain");
            hs.put("xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
            hs.put("xls", "application/vnd.ms-excel");
            hs.put("docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document");
            hs.put("eml", "message/rfc822");
            hs.put("msg", "application/octet-stream");
        }
    }

    public static String getMimeType(String type) throws Exception {
    	if(type.indexOf(".")>0) {
    		type=type.substring(type.indexOf(".")+1);
    	}
        String mimetype = (String) PropertyKeys.hs.get(type);
        if (mimetype == null) {
        	throw new Exception("type not found : " + type);
        }else{
        	return mimetype;
        }
    }

    public static String getMimeTypeWidhValue(String value) throws Exception {
        String keyValue = null;
        for (String key : PropertyKeys.hs.keySet()) {
            if (PropertyKeys.hs.get(key).equals(value)) {
                keyValue = key;
            }
        }
        if (keyValue == null) { 
        	throw new Exception("type not found : " + value); 
        }
        return keyValue;
}
}
