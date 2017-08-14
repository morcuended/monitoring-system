import mysql.connector
import MySQLdb as mdb
from mysql.connector import errorcode
import numpy as np
#from __future__ import print_function

# Define database parameters
myTable="Test_Camera1U_FFCoeff"
myTable_Set="Test_Camera1U_FFCoeff_Set"



config = {
  'user': 'morcuende',
  'host': '141.34.130.13',
  'database': 'HESS_HESS1U',
  'raise_on_warnings': True,
} 

try:
  cnx = mysql.connector.connect(**config)
  print "##################################################"
  print("You are now connected")
  print "--------------------------------------------------"
except mysql.connector.Error as err:
  if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
    print("Something is wrong with your user name or password \n")
  elif err.errno == errorcode.ER_BAD_DB_ERROR:
    print("Database does not exist \n")
  else:
    print(err)
else:
  cursor=cnx.cursor()
##########################################
cursor.execute("SHOW TABLES;")
table_list = map(lambda x:x[0],cursor)
#print table_list
if myTable in table_list:
	print "Table %s already exists \n" %myTable
#print cursor.fetchall()
else:
	cursor.execute("""CREATE TABLE Test_Camera1U_FFCoeff (
		entry int NOT NULL auto_increment PRIMARY KEY,
		SetNum int NOT NULL,
		RunNumber int,
		RunStarts DATETIME,
		pixel int NOT NULL,
		HiIllumination double,
		HiIlluminationRMS double,
		LoIllumination double,
		LoIlluminationRMS double,
		HiCoeff double,
		HiCoeffRMS double,
		LoCoeff double,
		LoCoeffRMS double,
		Chi2ndfHi double,
		Chi2ndfLo double, 
		index(SetNum)); """)
	print "Table %s just created \n" %myTable
	cnx.commit() # Make sure data is committed to the database

if myTable_Set in table_list:
        print "Table %s already exists \n" %myTable_Set
else:   
	cursor.execute("""CREATE TABLE Test_Camera1U_FFCoeff_Set (
		SetNum int NOT NULL auto_increment PRIMARY KEY,
		TelescopeId int,
		WhenEntered TIMESTAMP,
		index(TelescopeId)); """)
	cnx.commit() 
	print "Table %s just created \n" %myTable_Set

#cursor.execute("""insert into Test_Camera1U_FFCoeff_Set set SetNum=2,Comments="This is %s table", TelescopeId=1;"""%myTable)
#cursor.execute("insert into SimpleTable set Name=myTable;")

##Fill tables
#data=np.genfromtxt("FFcoeff.dat",skip_header=1,comments="#",delimiter=",")
#
## defining dummy numbers
#RunNumber = 128000
#TelescopeId= 1
#
##if not SetNum in SetNum_list:
#cursor.execute("""insert into Test_Camera1U_FFCoeff_Set set 
#		TelescopeId=%i;"""%(TelescopeId))
#cnx.commit() 
#
#cursor.execute("select SetNum from %s"%myTable_Set)
#SetNum = map(lambda x:x[0],cursor)
##print SetNum,"\n"
#SetNum = SetNum[len(SetNum)-1] #get the last element of the list (ie the last SetNum entry in *_Set table)
##print SetNum,"\n"
#
#for row in data:
#	PixId=row[0]
#	HiEntries=row[1]
#	LoEntries=row[2]
#	HiIllumination=row[3]
#	HiIlluminationRMS=row[4]
#	LoIllumination=row[5]
#	LoIlluminationRMS=row[6]
#	HiCoeff=row[7]
#	HiCoeffRMS=row[8]
#	LoCoeff=row[9]
#	LoCoeffRMS=row[10]
#	Chi2ndfHi=row[11]
#	Chi2ndfLo=row[12]
#	cursor.execute("""insert into Test_Camera1U_FFCoeff set 
#			pixel=%i,
#			RunNumber=%i,
#			SetNum=%i,
#			HiIllumination=%f,
#			HiIlluminationRMS=%f,
#			LoIllumination=%f,
#			LoIlluminationRMS=%f,
#			HiCoeff=%f,
#			HiCoeffRMS=%f,
#			LoCoeff=%f,
#			LoCoeffRMS=%f,
#			Chi2ndfHi=%f,
#			Chi2ndfLo=%f
#			;"""%(PixId,RunNumber,SetNum,HiIllumination,HiIlluminationRMS,LoIllumination,LoIlluminationRMS,
#			HiCoeff,HiCoeffRMS,LoCoeff,LoCoeffRMS,Chi2ndfHi,Chi2ndfLo))
#	cnx.commit() 
#
#	#Loop over the rows of data and store each value into the corresponding column of the table
#	#Check first if a certain SetNum is already in the table 
#cursor.execute("select * from %s"%myTable_Set)
#SetNum_list = map(lambda x:x[0],cursor)

# Write entry into SimpleTable ()

#Get the number of columns in myTable 
cursor.execute("""SELECT COUNT(*) FROM information_schema.columns WHERE table_name="%s";"""%myTable)
column_num = map(lambda x:x[0],cursor)
max_col=column_num[0]-2


#set condition if name=myTable is in SimpleTable

cursor.execute("""SELECT COUNT(*) FROM SimpleTable WHERE Name="%s";"""%myTable)
is_it_there = map(lambda x:x[0],cursor)

if is_it_there[0] == 0:
	cursor.execute("""insert into SimpleTable set 
		Name="%s",
		MaxColumns="%i",
		SqlTableSet="%s",
		SqlTableData="%s",
		SqlSetColumn="SetNum",
		SqlSetColumnData="SetNum";"""%(myTable,max_col,myTable_Set,myTable))
	print "Table %s has just been written into SimpleTable\n"%myTable
	cnx.commit() 
else: print "Table %s is already in SimpleTable \n"%myTable

# Write entries into SimpleTableColmun, with the SimpleTable number corresponding to myTable:
# Firstly, obtain the SimpleTable value (must be 1541 for myTable)

cursor.execute("""select SimpleTable from SimpleTable where Name="%s";"""%myTable)
SimpleTable_num = map(lambda x:x[0],cursor)
SimpleTable_num = SimpleTable_num[0]

#Insert values/rows in SimpleTableColumn         HEREEEEEEEE
# loop over columns in myTable:

cursor.execute("""select COLUMN_NAME from INFORMATION_SCHEMA.COLUMNS where TABLE_NAME='Test_Camera1U_FFCoeff';""")
columns=map(lambda x:x[0],cursor)
#print columns[0]

#Creating SimpleTableColumn
cursor.execute("""insert into SimpleTableColumn set 
		SqlColumn="TelescopeId",
		SimpleTable="%i",
		ColumnType="ID",
		nColumn=1,
		Description="Telescope Id for this entry/set";"""%(SimpleTable_num))

cursor.execute("""insert into SimpleTableColumn set 
                SqlColumn="WhenEntered",
                SimpleTable="%i",
                ColumnType="Aux",
                nColumn=2,
		ValueType="time",
                Description="Date of creation of this entry/set";"""%(SimpleTable_num))

cursor.execute("""insert into SimpleTableColumn set 
                SqlColumn="RunNumber",
                SimpleTable="%i",
                ColumnType="Data",
                nColumn=1,	
                Description="RunNumber used for the calculation of the coefficients";"""%(SimpleTable_num))

cursor.execute("""insert into SimpleTableColumn set 
                SqlColumn="RunStarts",
                SimpleTable="%i",
                ColumnType="Data",
                nColumn=2,
		ValueType="time",
                Description="When did the run start";"""%(SimpleTable_num))

cursor.execute("""insert into SimpleTableColumn set 
                SqlColumn="pixel",
                SimpleTable="%i",
                ColumnType="Data",
                nColumn=3,
                Description="Pixel position of the given coefficient";"""%(SimpleTable_num))

cursor.execute("""insert into SimpleTableColumn set 
                SqlColumn="HiIllumination",
                SimpleTable="%i",
                ColumnType="Data",
                nColumn=4,
		ValueType="real",
                Description="Calculated illumination for the pixel (high-gain)";"""%(SimpleTable_num))

cursor.execute("""insert into SimpleTableColumn set 
                SqlColumn="HiIlluminationRMS",
                SimpleTable="%i",
                ColumnType="Data",
                nColumn=5,
		ValueType="real",	
                Description="Error on the illumination for high-gain";"""%(SimpleTable_num))

cursor.execute("""insert into SimpleTableColumn set 
                SqlColumn="LoIllumination",
                SimpleTable="%i",
                ColumnType="Data",
                nColumn=6,
		ValueType="real",
                Description="Calculated illumination for the pixel (low-gain)";"""%(SimpleTable_num))

cursor.execute("""insert into SimpleTableColumn set 
                SqlColumn="LoIlluminationRMS",
                SimpleTable="%i",
                ColumnType="Data",
                nColumn=7,
		ValueType="real",	
                Description="Error on the illumination for low-gain";"""%(SimpleTable_num))

cursor.execute("""insert into SimpleTableColumn set 
                SqlColumn="HiCoeff",
                SimpleTable="%i",
                ColumnType="Data",
                nColumn=8,
		ValueType="real",
                Description="Calculated coeff. for high-gain";"""%(SimpleTable_num))

cursor.execute("""insert into SimpleTableColumn set 
                SqlColumn="HiCoeffRMS",
                SimpleTable="%i",
                ColumnType="Data",
                nColumn=9,
		ValueType="real",
                Description="Calculated error on coeff. for high-gain";"""%(SimpleTable_num))

cursor.execute("""insert into SimpleTableColumn set 
                SqlColumn="LoCoeff",
                SimpleTable="%i",
                ColumnType="Data",
                nColumn=10,
		ValueType="real",
                Description="Calculated coeff. for low-gain";"""%(SimpleTable_num))

cursor.execute("""insert into SimpleTableColumn set 
                SqlColumn="LoCoeffRMS",
                SimpleTable="%i",
                ColumnType="Data",
                nColumn=11,
		ValueType="real",
                Description="Calculated error on coeff. for low-gain";"""%(SimpleTable_num))

cursor.execute("""insert into SimpleTableColumn set 
                SqlColumn="Chi2ndfHi",
                SimpleTable="%i",
                ColumnType="Data",
                nColumn=12,
		ValueType="real",
                Description="Calculated chi2/ndf for the gaussian fit (high-gain)";"""%(SimpleTable_num))

cursor.execute("""insert into SimpleTableColumn set 
                SqlColumn="Chi2ndfLo",
                SimpleTable="%i",
                ColumnType="Data",
                nColumn=13,
		ValueType="real",
                Description="Calculated chi2/ndf for the gaussian fit (low-gain) ";"""%(SimpleTable_num))









cnx.commit() 

cnx.close()
cursor.close()
print "##################################################"

