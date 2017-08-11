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
		RunNumber int NOT NULL,
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
		TelescopeId int,
		index(SetNum)); """)
	print "Table %s just created \n" %myTable
	cnx.commit() # Make sure data is committed to the database

if myTable_Set in table_list:
        print "Table %s already exists \n" %myTable_Set
else:   
	cursor.execute("""CREATE TABLE Test_Camera1U_FFCoeff_Set (
		SetNum int NOT NULL auto_increment PRIMARY KEY,
		TelescopeId int NOT NULL,
		WhenEntered TIMESTAMP,
		index(TelescopeId)); """)
	cnx.commit() 
	print "Table %s just created \n" %myTable_Set
#cursor.close()



#cursor.execute("""insert into Test_Camera1U_FFCoeff_Set set SetNum=2,Comments="This is %s table", TelescopeId=1;"""%myTable)

#cursor.execute("insert into SimpleTable set Name=myTable;")


#Fill tables
data=np.genfromtxt("FFcoeff.dat",skip_header=1,comments="#",delimiter=",")
#print(data[1,])

SetNum = 1
RunNumber = 128000
TelescopeId= 1

for row in data:
	PixId=row[0]
	HiEntries=row[1]
	LoEntries=row[2]
	HiIllumination=row[3]
	HiIlluminationRMS=row[4]
	LoIllumination=row[5]
	LoIlluminationRMS=row[6]
	HiCoeff=row[7]
	HiCoeffRMS=row[8]
	LoCoeff=row[9]
	LoCoeffRMS=row[10]
	Chi2ndfHi=row[11]
	Chi2ndfLo=row[12]
	cursor.execute("""insert into Test_Camera1U_FFCoeff set 
			pixel=%i,
			RunNumber=%i,
			SetNum=%i,
			HiIllumination=%f,
			HiIlluminationRMS=%f,
			LoIllumination=%f,
			LoIlluminationRMS=%f,
			HiCoeff=%f,
			HiCoeffRMS=%f,
			LoCoeff=%f,
			LoCoeffRMS=%f,
			Chi2ndfHi=%f,
			Chi2ndfLo=%f,
			TelescopeId=%i;"""%(PixId,RunNumber,SetNum,HiIllumination,HiIlluminationRMS,LoIllumination,LoIlluminationRMS,
			HiCoeff,HiCoeffRMS,LoCoeff,LoCoeffRMS,Chi2ndfHi,Chi2ndfLo,TelescopeId))
	cnx.commit() 

#Loop over the rows of data and store each value into the corresponding column of the table
#Check first if a certain SetNum is already in the table 
cursor.execute("select * from %s"%myTable_Set)
#print cursor
#print cursor.fetchall()[0][1]
SetNum_list = map(lambda x:x[0],cursor)
#print SetNum_list

if not SetNum in SetNum_list:
	cursor.execute("""insert into Test_Camera1U_FFCoeff_Set set 
			SetNum=%i,
			TelescopeId=%i;"""%(SetNum,TelescopeId))
	cnx.commit() 

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


print "##################################################"

