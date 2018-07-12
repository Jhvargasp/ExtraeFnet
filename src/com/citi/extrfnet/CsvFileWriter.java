package com.citi.extrfnet;

import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.Vector;

public class CsvFileWriter {

	// Delimiter used in CSV file
	private static final String COMMA_DELIMITER = ":";
	private static final String NEW_LINE_SEPARATOR = "\n";

	// CSV file header
	private static final String FILE_HEADER = "id,firstName,lastName,gender,age";

	public static void writeCsvFile(String fileName, List<Vector> results) {

		FileWriter fileWriter = null;

		try {
			fileWriter = new FileWriter(fileName);

			// Write the CSV file header
			//fileWriter.append(FILE_HEADER.toString());

			// Add a new line separator after the header
			//fileWriter.append(NEW_LINE_SEPARATOR);

			// Write a new student object list to the CSV file
			for (Vector row : results) {
				for (int i = 0; i < row.size(); i++) {
					Object value=row.get(i);
					fileWriter.append(value.toString());
					if (i < row.size() - 1) {
						fileWriter.append(COMMA_DELIMITER);
					}
				}
				fileWriter.append(NEW_LINE_SEPARATOR);
			}

			System.out.println("CSV file was created successfully with "+results.size()+" records!!!");

		} catch (Exception e) {
			System.out.println("Error in CsvFileWriter !!!");
			e.printStackTrace();
		} finally {

			try {
				fileWriter.flush();
				fileWriter.close();
			} catch (IOException e) {
				System.out.println("Error while flushing/closing fileWriter !!!");
				e.printStackTrace();
			}

		}
	}

}
