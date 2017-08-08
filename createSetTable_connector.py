import mysql.connector
import MySQLdb as mdb
from mysql.connector import errorcode
#from warnings import filterwarnings

#filterwarnings('ignore', category = mysql.connector.Warning)

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
  print("You are connected \n")
except mysql.connector.Error as err:
  if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
    print("Something is wrong with your user name or password \n")
  elif err.errno == errorcode.ER_BAD_DB_ERROR:
    print("Database does not exist \n")
  else:
    print(err)
else:
  cursor=cnx.cursor()
#################################################
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
		LoCoefdRMS double, 
		TelescopeId int,
		index(SetNum)); """)
	cnx.commit()

#####################################################
if myTable_Set in table_list:
        print "Table %s already exists \n" %myTable_Set
else:   
	cursor.execute("""CREATE TABLE Test_Camera1U_FFCoeff_Set (
		SetNum int NOT NULL auto_increment PRIMARY KEY,
		TelescopeId int NOT NULL,
		WhenEntered TIMESTAMP,
		index(TelescopeId)); """)
	cnx.commit()
cnx.close()
cursor.close()
